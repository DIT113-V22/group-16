#include <Smartcar.h>
#include <HardwareSerial.h>
#include <vector>
#include <MQTT.h>
#include <WiFi.h>


#ifdef __SMCE__
#include <OV767X.h>
#endif

MQTTClient mqtt;
WiFiClient net;


std::vector<char> frameBuffer;

const char ssid[] = "admin";
const char pass[] = "hivemq";


//Mqtt topics
const auto controlTopic = "/Group/16/Control";
const auto streamTopic = "/Group/16/Camera";

#ifdef __SMCE__
const auto mqttBrokerUrl = "127.0.0.1";
#else
const auto mqttBrokerUrl = "192.168.0.40";  // if not in the smce virtual environment
#endif

const auto oneSecond = 1000UL;

//Declaring pins
const unsigned short GYRO_PIN = 37;

const unsigned short FRONT_INFRA_PIN = 0;
const unsigned short LEFT_INFRA_PIN = 1;
const unsigned short RIGHT_INFRA_PIN = 2;
const unsigned short BACK_INFRA_PIN = 3;

const unsigned short ULTRA_SONIC_TRIGGER_PIN = 6;
const unsigned short ULTRA_SONIC_ECHO_PIN = 7;
const unsigned int MAX_DISTANCE = 400;

//Declaring speed
const int forwardSpeed = 50;
const int backwardSpeed = -50; // 80% of the full speed backward
const int stoppingSpeed = 0;

const int leftDegrees = -90;  // degrees to turn left
const int rightDegrees = 90;  // degrees to turn right

const int BrakeDistance = 100;  // obstacle avoidance distance
const int crashDistance = 20;   // judge whether the car crash obstacle
const long backDistance = -50;  // when crash the obstacle, car backword distance

const float cruiseSpeed = 1.0;
const float speedToTurn = 1.0;
const int degreesToTurn = 90;

// Declaring flag
// bool updateFlag = false;    // when the car enable cruise control, update its speed
bool forward = false;        // judge whether the car is moving forward
bool cruiseFlag = false;    // judge whether the car is enabled cruise


ArduinoRuntime arduinoRuntime;

BrushedMotor leftMotor(arduinoRuntime, smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(arduinoRuntime, smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

const auto pulsesPerMeter = 600;
DirectionlessOdometer leftOdometer{ arduinoRuntime,
                          smartcarlib::pins::v2::leftOdometerPin,
    []() { leftOdometer.update(); },
    pulsesPerMeter };
DirectionlessOdometer rightOdometer{ arduinoRuntime,
                          smartcarlib::pins::v2::rightOdometerPin,
    []() { rightOdometer.update(); },
    pulsesPerMeter };


GY50 gyroscope(arduinoRuntime, GYRO_PIN);
SR04 ultraFront(arduinoRuntime, ULTRA_SONIC_TRIGGER_PIN, ULTRA_SONIC_ECHO_PIN, MAX_DISTANCE);

GP2D120 infraFront(arduinoRuntime, FRONT_INFRA_PIN);
GP2D120 infraLeft(arduinoRuntime, LEFT_INFRA_PIN);
GP2D120 infraRight(arduinoRuntime, RIGHT_INFRA_PIN);
GP2D120 infraBack(arduinoRuntime, BACK_INFRA_PIN);


SmartCar car(arduinoRuntime, control, gyroscope, leftOdometer, rightOdometer);


void setup()
{
    Serial.begin(9600);

    //initialize camera if use on SMCE
#ifdef __SMCE__
    Camera.begin(QVGA, RGB888, 15);
    frameBuffer.resize(Camera.width() * Camera.height() * Camera.bytesPerPixel());
#endif

    //connect to wifi
    WiFi.begin(ssid, pass);
    mqtt.begin(mqttBrokerUrl, 1883, net);

    Serial.println("Connecting to WiFi...");
    auto wifiStatus = WiFi.status();
    while (wifiStatus != WL_CONNECTED && wifiStatus != WL_NO_SHIELD)
    {
        Serial.println(wifiStatus);
        Serial.print(".");
        delay(1000);
        wifiStatus = WiFi.status();
    }

    // connect to broker
    Serial.println("Connecting to MQTT broker");
    while (!mqtt.connect("arduino", "public", "public"))
    {
        Serial.print(".");
        delay(1000);
    }

    // subscribe topic
    mqtt.subscribe(controlTopic, 1);
    mqtt.onMessage([](String topic, String message)
    {
        ctrlHeading(message);
        cruiseControl(message);
    });
}


void loop()
{
    if(mqtt.connected())
    {
        mqtt.loop();
        const auto currentTime = millis();

#ifdef __SMCE__
        // publish camera to frontend
        static auto previousFrame = 0UL;
        if(currentTime - previousFrame >= 40)
        {
            previousFrame = currentTime;
            Camera.readFrame(frameBuffer.data());
            mqtt.publish(streamTopic, frameBuffer.data(), frameBuffer.size(), false, 0);
        }
#endif

        // publish infrared sensor distance to frontend
        static auto previousTransmission = 0UL;
        if (currentTime - previousTransmission >= oneSecond)
        {
            previousTransmission = currentTime;
            const auto leftDistance = String(infraLeft.getDistance());
            const auto rightDistance = String(infraRight.getDistance());
            const auto frontDistance = String(infraFront.getDistance());

            mqtt.publish("Group/16/Distance/Left", leftDistance);
            mqtt.publish("Group/16/Distance/Right", rightDistance);
            mqtt.publish("Group/16/Distance/Front", frontDistance);
        }
        

        if(cruiseFlag)
        {
            car.update();

            // when car stoped and enable cruise mode, turn certain degree and then keep moving forward
            if(car.getSpeed() == 0)
            {
                if(ultraFront.getDistance() < BrakeDistance && ultraFront.getDistance() > 0)
                {
                    rotate(degreesToTurn, speedToTurn);
                    car.setSpeed(cruiseSpeed);
                    car.setAngle(0);
                    car.update();
                }
            }

            // when the car crash obstacle, go back certain distance and turn certain degree, then keep moving forward
            if((ultraFront.getDistance() < crashDistance && ultraFront.getDistance() > 0) && car.getSpeed() < 0.01)
            {
                Serial.println("go back");
                go(backDistance, cruiseSpeed);
                rotate(degreesToTurn, speedToTurn);
                car.setSpeed(cruiseSpeed);
                car.setAngle(0);
                car.update();
            }
        }

    }


    // force the car to stop automatically when it is 1 meter away from an obstacle
    if(car.getSpeed() != 0 && forward)
    {
        obstacleAvoidance(BrakeDistance);
    }


#ifdef __SMCE__
    // Avoid over-using the CPU if we are running in the emulator
    delay(1);
#endif
}


/* Car control */

// enable cruise mode
void cruiseControl(String message)
{
    if(message.equals("Cruise"))
    {
        // car.enableCruiseControl(5.0F, 0.02F, 10.0F, 50);
        car.enableCruiseControl();
        car.setSpeed(cruiseSpeed);
        car.setAngle(0);
        car.update();
        cruiseFlag = true;
        forward = true;
    }
}

void obstacleAvoidance(int distance)
{
    int distanceFromObjects = ultraFront.getDistance();
    if(cruiseFlag)
    {
        if(distanceFromObjects < distance * 3 && distanceFromObjects > distance * 2)
        {
            car.setSpeed(cruiseSpeed * 0.5);
        }
        if(distanceFromObjects < distance * 2 && distanceFromObjects > distance)
        {
            car.setSpeed(cruiseSpeed * 0.2);
        }
        if(distanceFromObjects == 0)
        {
            car.setSpeed(cruiseSpeed);
        }
    }
    // else is the normal control
    else
    {
        if(distanceFromObjects < distance * 3 && distanceFromObjects > distance * 2)
        {
            car.setSpeed(forwardSpeed * 0.5);
        }
        if(distanceFromObjects < distance * 2 && distanceFromObjects > distance)
        {
            car.setSpeed(forwardSpeed * 0.2);
        }
        // if(distanceFromObjects == 0) {
        //     car.setSpeed(forwardSpeed);
        // }
    }

    if(distanceFromObjects < distance && distanceFromObjects > 0)
    {
        car.setSpeed(stoppingSpeed);
    }
    Serial.println(distanceFromObjects);
}

// car heading control
void ctrlHeading(String message)
{
    car.disableCruiseControl();
    if(message.equals("Left"))   // rotate counter-clockwise going forward
    {
        car.setSpeed(forwardSpeed);
        car.setAngle(leftDegrees);
        forward = false;
    }
    else if(message.equals("Right"))   // turn clock-wise
    {
        car.setSpeed(forwardSpeed);
        car.setAngle(rightDegrees);
        forward = false;
    }
    else if(message.equals("Ahead"))   // go ahead
    {
        car.setSpeed(forwardSpeed);
        car.setAngle(0);
        forward = true;
    }
    else if(message.equals("Back"))   // go back
    {
        car.setSpeed(backwardSpeed);
        car.setAngle(0);
        forward = false;
    }
    else if(message.equals("Stop"))
    {
        car.setSpeed(stoppingSpeed);
        forward = false;
    }
    cruiseFlag = false;
}

// car rotates specific degree
void rotate(int degrees, float speed)
{
    speed = smartcarlib::utils::getAbsolute(speed);
    degrees %= 360; // Put degrees in a (-360,360) scale
    if (degrees == 0)
    {
        return;
    }

    const auto initialHeading = car.getHeading();
    car.setSpeed(speed);
    if (degrees > 0)
    {
        car.setAngle(90);
    }
    else
    {
        car.setAngle(-90);
    }

    bool hasReachedTargetDegrees = false;
    int degreesTurnedSoFar = 0;
    int currentHeading = 0;

    while (!hasReachedTargetDegrees)
    {
        car.update();
        currentHeading = car.getHeading();

        if (degrees < 0 && currentHeading < initialHeading)
        {
            // If we are turning left and the current heading is larger than the
            // initial one (e.g. started at 10 degrees and now we are at 350), we need to
            // substract 360 so to eventually get a signed displacement from the initial heading
            // (-20)
            currentHeading += 360;
        }
        else if (degrees > 0 && currentHeading > initialHeading)
        {
            // If we are turning right and the heading is smaller than the
            // initial one (e.g. started at 350 degrees and now we are at 20), so to get a
            // signed displacement (+30)
            currentHeading -= 360;
        }
        // Degrees turned so far is initial heading minus current (initial heading
        // is at least 0 and at most 360. To handle the "edge" cases we substracted or added 360
        // to currentHeading)
        degreesTurnedSoFar = (degrees > 0 ? initialHeading - currentHeading : currentHeading - initialHeading);

        hasReachedTargetDegrees = smartcarlib::utils::getAbsolute(degreesTurnedSoFar)
                                  >= smartcarlib::utils::getAbsolute(degrees);
    }
    car.setSpeed(0);
}

// car moving certain distance with a specific speed
void go(long centimeters, float speed)
{
    if (centimeters == 0)
    {
        return;
    }
    // Ensure the speed is towards the correct direction
    speed = smartcarlib::utils::getAbsolute(speed) * ((centimeters < 0) ? -1 : 1);
    car.setSpeed(speed);
    car.setAngle(0);

    long initialDistance          = car.getDistance();
    bool hasReachedTargetDistance = false;
    while (!hasReachedTargetDistance)
    {
        car.update();
        auto currentDistance   = car.getDistance();
        auto travelledDistance = initialDistance > currentDistance
                                 ? initialDistance - currentDistance
                                 : currentDistance - initialDistance;
        hasReachedTargetDistance = travelledDistance >= smartcarlib::utils::getAbsolute(centimeters);
    }
    car.setSpeed(0);
}