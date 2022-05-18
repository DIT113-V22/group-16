#include <Smartcar.h>
#include <HardwareSerial.h>
#include <vector>
#include <MQTT.h>
#include <WiFi.h>
#include <string.h>

#ifdef __SMCE__
#include <OV767X.h>
#endif

//Headers
void controlBus(String topic, String message);
void rotate(int degrees, float speed);
void carBrake();
void publishDistance();
void ctrlHeading(String message);
void obstacleAvoidance( bool alleywayBacking);
void go(long centimeters, float speed);
void cruiseControl();

MQTTClient mqtt;
WiFiClient net;
const auto mqttBrokerUrl = "127.0.0.1";
const auto oneSecond = 1000UL;

const char ssid[] = "admin";
const char pass[] = "123456";

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

const float cruiseSpeed = 1.3;
const float speedToTurn = 0.4;
const int degreesToTurn = 30;

const int softBreak = 150;
const int hardBreak = 60;

//Mqtt topics
const String controlTopic = "/Group/16/Control";
const char *streamTopic = "/Group/16/Camera";
const String distanceTopic = "/Group/16/Distance";


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
    
}



void loop() {
    if(mqtt.connected()){
        mqtt.loop();
        const auto currentTime = millis();
        
        #ifdef __SMCE__
        cameraStream(currentTime);  // publish camera to frontend
        #endif

        publishDistance();
        mqtt.subscribe(controlTopic, 1); //QoS 1

        mqtt.onMessage([](String receivedTopic, String receivedMessage){
            controlBus(receivedTopic, receivedMessage);                     
        });
        controlBus("s", "s");
    }
}


//Redirect different controls
void controlBus(String topic, String message){
    car.update();

    
    String control = "Stop";

    if(topic.compareTo(controlTopic) == 0){
        
        control = message;  
        if(control.compareTo("Cruise") == 0){
            car.enableCruiseControl(5.0F, 0.02F, 10.0F, 50);
            cruiseFlag = true;
        }
        else if(control.compareTo("Stop") && cruiseFlag){

            ctrlHeading(control);
            cruiseFlag = false;
        } 
        else {
            ctrlHeading(control);
            cruiseFlag = false;
        }        
    }
    if(cruiseFlag){
        cruiseControl();
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
    Serial.println("front" + frontDistance);
    
}



// Car control
void carBrake()
{
    car.update();
    car.setSpeed(0);
    car.setAngle(0);
}

void cruiseControl(){
    car.update();
    Serial.println(ultraFront.getDistance());
    if(ultraFront.getDistance() <= softBreak && ultraFront.getDistance() > hardBreak){
        obstacleAvoidance(false);
    } else if(ultraFront.getDistance() <=  hardBreak && ultraFront.getDistance() > 0){
        obstacleAvoidance(true);
    } else {
        car.setSpeed(cruiseSpeed);
        car.setAngle(0); 
    }
     

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

#ifdef __SMCE__
void cameraStream( unsigned long currentTime){
    static auto previousFrame = 0UL;
    if(currentTime - previousFrame >=65){
        previousFrame = currentTime;
        Camera.readFrame(frameBuffer.data());
        mqtt.publish(streamTopic, frameBuffer.data(), frameBuffer.size(), false, 0);
  }
}
#endif

void ctrlHeading(String message){
    if(message.compareTo("Left") == 0){
        car.setSpeed(forwardSpeed);
        car.setAngle(leftDegrees);
        forward = false;
    } 
    else if( message.compareTo("Right") == 0){
        car.setSpeed(forwardSpeed);
        car.setAngle(rightDegrees);
        forward = false;
    } 
    else if( message.compareTo("Forward") == 0){
        car.setSpeed(forwardSpeed);
        car.setAngle(0);
        obstacleAvoidance(false);
        forward = true;
    }
    else if(message.compareTo("Backward") == 0){
        if(car.getSpeed() != 0) {
            carBrake();
        }
        else {
            car.setSpeed(backwardSpeed);
            car.setAngle(0);
        }   
        forward = false;   
    }
    else if( message.compareTo("Stop") == 0 ){
      car.setSpeed(0);
      obstacleAvoidance(false);
    }
    else {
        carBrake();
    }

    
}

void obstacleAvoidance( bool alleywayBacking)
{
    car.update();
    int distFront = ultraFront.getDistance();
    int stopFront = infraFront.getDistance();
    int distLeft = infraLeft.getDistance();
    int distRight = infraRight.getDistance();
    

    //backing up if the car is in an unturnable corridor
    if (alleywayBacking){
        Serial.println("yeet");
        go(backDistance, backwardSpeed); 
        if(distLeft > 0 && distRight > 0){
            obstacleAvoidance( true);
        }
        else if(distLeft == 0){
            rotate(-20, speedToTurn);
            alleywayBacking = false;
        }
        else{
            rotate(20, speedToTurn);
            alleywayBacking = false;
        }
    }
    else{
        if(distFront < softBreak){
            car.update();         
            if(distLeft > 0 && distRight > 0){
                obstacleAvoidance(true);
            }else if( distLeft == 0 ){
                rotate(-20, speedToTurn);
            }else{
                rotate(20, speedToTurn);
            }
        }
    }
    

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
