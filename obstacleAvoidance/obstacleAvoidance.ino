#include <Smartcar.h>
#include <HardwareSerial.h>

const int forwardSpeed   = 50; 
const int stoppingSpeed = 0; 

ArduinoRuntime arduinoRuntime;

BrushedMotor leftMotor(arduinoRuntime, smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(arduinoRuntime, smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

SimpleCar car(control);

const int triggerPin           = 6; // D6
const int echoPin              = 7; // D7
const unsigned int maxDistance = 100;

SR04 front{arduinoRuntime, triggerPin, echoPin, maxDistance};

void setup()
{
    Serial.begin(9600);
    car.setSpeed(forwardSpeed);
}

void loop()
{ 
     int distanceFromObjects = front.getDistance();
     if (distanceFromObjects < 100){
      car.setSpeed(stoppingSpeed);
     }
    Serial.println(distanceFromObjects);
    delay(100);
}
