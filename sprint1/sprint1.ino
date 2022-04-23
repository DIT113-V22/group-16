#include <Smartcar.h>
#include <HardwareSerial.h>

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

bool updateFlag = false;
bool forward = true;
bool cruiseFlag = false;

ArduinoRuntime arduinoRuntime;

BrushedMotor leftMotor(arduinoRuntime, smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(arduinoRuntime, smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

GY50 gyroscope(arduinoRuntime, 37);

const auto pulsesPerMeter = 600;

DirectionlessOdometer leftOdometer{ arduinoRuntime,
                          smartcarlib::pins::v2::leftOdometerPin,
                            []() { leftOdometer.update(); },
                            pulsesPerMeter };
DirectionlessOdometer rightOdometer{ arduinoRuntime,
                          smartcarlib::pins::v2::rightOdometerPin,
                            []() { rightOdometer.update(); },
                            pulsesPerMeter };

SmartCar car(arduinoRuntime, control, gyroscope, leftOdometer, rightOdometer);


const int TRIGGER_PIN           = 6; // D6
const int ECHO_PIN              = 7; // D7
const unsigned int MAX_DISTANCE = 400;

SR04 front(arduinoRuntime, TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);


void obstacleAvoidance(int dis)
{
    int distanceFromObjects = front.getDistance();
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

void carBrake()
{
    car.setSpeed(0);
    car.setAngle(0);
}

void cruiseControl()
{
    car.enableCruiseControl();
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
                                  >= smartcarlib::utils::getAbsolute(degrees) - 5;
    }
    car.setSpeed(0);
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

void ctrlHeading(char input)
{
    switch (input)
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


void setup()
{
    Serial.begin(9600);
}

void loop()
{
    // car.disableCruiseControl();
    if (Serial.available())
    {
        char input = Serial.read(); // read everything that has been received so far and log down
                                    // the last entry
        ctrlHeading(input);
        if(input == 'c')
        {
            cruiseControl();
        }
    }

    if(updateFlag) 
    {
        car.update();
    }

    if(car.getSpeed() != 0 && forward) 
    {
        obstacleAvoidance(BrakeDistance);
    }

    if(car.getSpeed() == 0 && cruiseFlag) 
    {
        if(front.getDistance() < BrakeDistance && front.getDistance() > 0)
        {
            rotate(degreesToTurn, speedToTurn);
            car.setSpeed(cruiseSpeed);
            car.setAngle(0);
            car.update();
        }
    }

    if((front.getDistance() > 0 && front.getDistance() < crackDistance) && car.getSpeed() < 0.01 && cruiseFlag)
    {
        Serial.println("go back");
        go(backDistance, cruiseSpeed);
        rotate(degreesToTurn, speedToTurn);
        car.setSpeed(cruiseSpeed);
        car.setAngle(0);
        car.update();
    }
    delay(10);
}
