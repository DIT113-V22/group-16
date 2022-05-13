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
const auto mqttBrokerUrl = "192.168.0.242";

const char ssid[] = "admin";
const char pass[] = "emqx";

std::vector<char> frameBuffer;


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
const int forwardSpeed = 80;
const int backwardSpeed = -80; // 50% of the full speed backward
const int stoppingSpeed = 0;

const int leftDegrees = -90;  // degrees to turn left
const int rightDegrees = 90;  // degrees to turn right

const int BrakeDistance = 100;  // obstacle avoidance distance
const int crackDistance = 20;
const long backDistance = -50;

const float cruiseSpeed = 1.0;
const float speedToTurn = 0.2;
const int degreesToTurn = 90;

//Mqtt topics

const String controlTopic = "/Group/16/Control";
const String streamTopic = "/Group/16/Camera";
const String distanceTopic = "/Group/16/Distance";
=======
const String controlTopic = "Group/16/Control";
const String streamTopic = "Group/16/Camera";
const String distanceTopic = "Group/16/Distance";




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

bool updateFlag = false;
bool forward = true;
bool cruiseFlag = false;


void setup() {
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
        while (wifiStatus != WL_CONNECTED && wifiStatus != WL_NO_SHIELD) {
            Serial.println(wifiStatus);
            Serial.print(".");
            delay(1000);
            wifiStatus = WiFi.status();
        }
        
    while (!mqtt.connect("SmartCarMQTT", "SmartCarMQTT", " ")) {
        Serial.println("MQTT Connecting...");
        delay(1000);
    }
    if (mqtt.connected()) {
        Serial.println("Connected to MQTT Broker");
    }
}



void loop() {
    if(mqtt.connected()){
        mqtt.loop();
        const auto currentTime = millis();

        /*#ifdef __SMCE__
        cameraStream();
        #endif*/

        publishDistance();

        mqtt.subscribe(controlTopic, 1); //QoS 1
        mqtt.onMessage([](String topic, String message){
            car.update();
            if(topic.compareTo(controlTopic) == 0){
                if(message == "Cruise"){
                    cruiseControl();
                } else if(message == "Stop"){
                    carBrake();
                }
                else ctrlHeading(message);
                

                if(car.getSpeed() == 0 && cruiseFlag)
                {
                    if(ultraFront.getDistance() < BrakeDistance && ultraFront.getDistance() > 0)
                    {
                        rotate(degreesToTurn, speedToTurn);
                        car.setSpeed(cruiseSpeed);
                        car.setAngle(0);
                        car.update();
                    }
                }

                if((ultraFront.getDistance() > 0 && ultraFront.getDistance() < crackDistance) && car.getSpeed() < 0.01 && cruiseFlag)
                {
                    Serial.println("go back");
                    go(backDistance, cruiseSpeed);
                    rotate(degreesToTurn, speedToTurn);
                    car.setSpeed(cruiseSpeed);
                    car.setAngle(0);
                    car.update();
                } 
            }
            
        });
    }
}

//mqtt sensor distance
void publishDistance(){
    car.update();

    const auto leftDistance = String(infraLeft.getDistance());
    const auto rightDistance = String(infraRight.getDistance());
    const auto frontDistance = String(infraFront.getDistance());
    mqtt.publish("/Group/16/Distance/Left", leftDistance);
    mqtt.publish("/Group/16/Distance/Right", rightDistance);
    mqtt.publish("/Group/16/Distance/Front", frontDistance);
}



// Car control
void carBrake()
{
    car.setSpeed(0);
    car.setAngle(0);
}

void cruiseControl()
{
    car.update();
    car.enableCruiseControl(5.0F, 0.02F, 10.0F, 50);
    car.setSpeed(cruiseSpeed);
    car.setAngle(0);    
    updateFlag = true;
    cruiseFlag = true;
    forward = true;
}

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

    while (!hasReachedTargetDegrees){
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
                                  >= smartcarlib::utils::getAbsolute(degrees) - 5;
    }
    car.setSpeed(0);
}
/*
void cameraStream(){
    const auto currentTime = millis();
    static auto previousFrame = 0UL;
    if(currentTime - previousFrame >=65){
        previousFrame = currentTime;
        Camera.readFrame(frameBuffer.data());
        mqtt.publish(streamTopic, frameBuffer.data(), frameBuffer.size(), false, 0);
  }
}*/

void ctrlHeading(String message){
    if(message.compareTo("Left") == 0){
        car.setSpeed(forwardSpeed);
        car.setAngle(leftDegrees);
        forward = false;
    } else if( message.compareTo("Right") == 0){
        car.setSpeed(forwardSpeed);
        car.setAngle(rightDegrees);
        forward = false;
    } else if( message.compareTo("Forward") == 0){
        car.setSpeed(forwardSpeed);
        car.setAngle(0);
        obstacleAvoidance(BrakeDistance);
        forward = true;
    } else if(message.compareTo("Backward") == 0){
        if(car.getSpeed() != 0) {
            carBrake();
        }
        else {
            car.setSpeed(backwardSpeed);
            car.setAngle(0);
        }   
        forward = false;   
    } else{
        carBrake();
    }

    
}

void obstacleAvoidance(int dis)
{
    int distanceFromObjects = ultraFront.getDistance();
    // three levels brake system
    if(cruiseFlag) {
        if(distanceFromObjects < dis * 3 && distanceFromObjects > dis * 2) {
            car.setSpeed(cruiseSpeed * 0.5);
        }
        if(distanceFromObjects < dis * 2 && distanceFromObjects > dis) {
            car.setSpeed(cruiseSpeed * 0.2);
        } 
    }
    else {
        if(distanceFromObjects < dis * 3 && distanceFromObjects > dis * 2) {
            car.setSpeed(forwardSpeed * 0.5);
        }
        if(distanceFromObjects < dis * 2 && distanceFromObjects > dis) {
            car.setSpeed(forwardSpeed * 0.2);
        }
    }

    if(distanceFromObjects < dis && distanceFromObjects > 0)
    {
        car.setSpeed(stoppingSpeed);
    }
    // Serial.println("init dis");
    Serial.println(distanceFromObjects);
}

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
