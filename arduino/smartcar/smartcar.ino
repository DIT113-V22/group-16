#include <Smartcar.h>
#include <HardwareSerial.h>
#include <vector>
#include <MQTT.h>
#include <Wifi.h>

#ifdef __SMCE__
#include <OV767X.h>
#endif

MQttClient mqtt;
WiFiClient net;


const char ssid[] = "***";
const char pass[] = "****";

std::vector<char> frameBuffer;

//Declaring pins
const unsigned short GYRO_PIN = 37;

const unsigned short BACK_INFRA_PIN = 0;
const unsigned short FRONT_INFRA_PIN = 1;
const unsigned short LEFT_INFRA_PIN = 2;
const unsigned short RIGHT_INFRA_PIN = 3;

const unsigned short ULTRA_SONIC_TRIGGER_PIN = 6
const unsigned short ULTRA_SONIC_ECHO_PIN = 7
const unsigned int MAX_DISTANCE = 400;

//Declaring speed
const int forwardSpeed = 80;
const int backwordSpeed = -80; // 50% of the full speed backward
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
const String controlTopic = "Group/16/Control";



ArduinoRuntime arduinoRuntime;

BrushedMotor leftMotor(arduinoRuntime, smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(arduinoRuntime, smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

GY50 gyroscope(arduinoRuntime, GYRO_PIN);
SR04 ultraFront(arduinoRuntime, ULTRA_SONIC_TRIGGER_PIN, ULTRA_SONIC_ECHO_PIN, MAX_DISTANCE);

GP2D120 infraFront(arduinoRuntime, FRONT_INFRA_PIN);
GP2D120 infraLeft(arduinoRuntime, LEFT_INFRA_PIN);
GP2D120 infraRight(arduinoRuntime, RIGHT_INFRA_PIN);
GP2D120 infraBack(arduinoRuntime, BACK_INFRA_PIN);





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

}

void loop() {
  // put your main code here, to run repeatedly:
}



// Car control
void carBrake()
{
    car.setSpeed(0);
    car.setAngle(0);
}


void rotate(int degrees, float speed){

}


mqtt.subscribe(controlTopic,1);
mqtt.onMessage([](String topic, String message));




void cameraStream(){
    if(mqtt.connected()){
        mqtt.loop();
        const auto currentTime = millis();

        #ifdef __SMCE__
        static auto previousFrame = 0UL;
        if(currentTime - previousFrame >=65){
            previousFrame = currentTime;
            Camera.readFrame(frameBuffer.data());
            mqtt.publish("/group/16/camera", frameBuffer.data(), frameBuffer.size(), false, 0);
        }
        #endif


    }
}

void ctrlHeading(message){



    switch (message)
    {
    case 'a': // rotate counter-clockwise going forward
        car.setSpeed(forwardSpeed);
        car.setAngle(leftDegrees);
        forward = false;
        break;
    case 'd': // turn clock-wise
        car.setSpeed(forwardSpeed);
        car.setAngle(rightDegrees);
        forward = false;
        break;
    case 'w': // go ahead
        car.setSpeed(forwardSpeed);
        car.setAngle(0);
        obstacleAvoidance(BrakeDistance);
        forward = true;
        break;
    case 's': // go back
        if(car.getSpeed() != 0) {
            carBrake();
        }
        else {
            car.setSpeed(backwordSpeed);
            car.setAngle(0);
        }
        forward = false;
        break;
    default: // if you receive something that you don't know, just stop
        car.setSpeed(0);
        car.setAngle(0);
    }
}













