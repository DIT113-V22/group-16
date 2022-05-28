# Mini Project: System Development

## Group-16
## Members:

1. Bora Kocak
2. Daniel Dovhun
3. Cuong Darma Le Manh
4. Yuhan Li
5. Qianyuan Wang

## Demo Video:

## What are we going to make?
We are going to do an Android software that allows an end user to control a smart car remotely. This app will allow users to control and watch a live stream of the car by the camera. Besides, the smart car can detect obstacles around it and avoid it using embedded sensors for it to travel freely. The smart car also have the features such as emergency breaking system.Also the smart car have the Cruise function.



## Why we want make it? 
We want make an app that let users can control the car easily and safely when the car is not so far with the user.It aims to minimize the human mistakes in the driving ezxerience. It aims to lower the amount of crashes.
  
  Scenario:
    In winter, it may take a while for the user to come out of the room to the parking space. If he uses a mobile phone to remotely control his car to the door, it will avoid the cold caused by just leaving the room.
   
   Scenario:
    During the parking car remotely and manually, the driver could not figure out if he is close to other cars. He uses camera and obstacle warning to check how close he is to other cars. This way the parking becomes a safer and smooother experience for the driver.

 ## How are we going to make it?
During the making of the car mostly arduino and the SMCE-gd will be used.Because we are making an Android app so we will use the Android studio.For programming language, Java and C/C++ will be used to developed software for the project.
The team will work with agile development methods. There will be weekly meetings where we discuss progress and the problems with the software. The problems we can not solve by our own will be adress to the TAs in weekly TA meetings

## Features of the App

1. [Manual Control](https://github.com/DIT113-V22/group-16/wiki/Milestone-2#milestone-2-manual-control)
2. [Cruise Control](https://github.com/DIT113-V22/group-16/wiki/Milestone-2#milestone-2-cruise-control)
3. [Obstacle Avoidance](https://github.com/DIT113-V22/group-16/wiki/Milestone-1#milestone-1-obstacle-avoidance)
4. [Obstacle Warning](https://github.com/DIT113-V22/group-16/wiki/Milestone-1#milestone-2-obstacle-warning)
5. [Video Streaming](https://github.com/DIT113-V22/group-16/wiki/Milestone-3#milestone-3-video-streaming)

## Techologies
* SMCE-gd
* MQTT
* Arduino
* Android Studio
* C/C++
* Java
* Github
* Gradle
## Sensor we use
1. [Ultrasonic sensor (SR04)](https://github.com/ItJustWorksTM/smce-gd/wiki/Vehicle-Capabilities#ultrasonic-distance-sensor)
2. [Infrared distance sensor (GP2D120)](https://github.com/ItJustWorksTM/smce-gd/wiki/Vehicle-Capabilities#infrared-distance-sensor)
3. [Gyroscope (GY50)](https://github.com/ItJustWorksTM/smce-gd/wiki/Vehicle-Capabilities#gyroscope)
## Function and technology correspondence
1. Use UI of Android studio to let the App have the UI can control the car.
2. Use the MQTT broker connect the Android app and SMCE-gd.
3. Use Utlrasonic sensor to detect obstacle to achieve the Obstacle Avoidance.
4. Use the Infrared sensor to achieve the Obstacal Warning.
## Resource
1. [SMCE](https://github.com/ItJustWorksTM/smce-gd/releases)
2. [Android studio](https://developer.android.com/studio)
3. [Arduino](https://www.arduino.cc/en/software)

## Get started:
```
1. Android Studio
2. JDK1.8
3. SMCE-gd
4. ArduinoIDE
5. HiveMQ(or any other MQTT broker)
```
### Get started
1. Clone the project by SSH or HTTPS
* SSH:
```
git@github.com:DIT113-V22/group-16.git
```
* HTTPS:
```
https://github.com/DIT113-V22/group-16.git
```
2. After you clone or download the project, use the Android Studio to open the “androidApp” file and sync the “build.gradle”file.
3. Then start the MQTTBroker that you have download.(The default MQTThost is your local-IP and the default MQTTport is 1883. You can set the MQTThost in "MainActivity.java")
4. After this open the "arduino.ino" by ArduinoIDE, set the same host, port, USERNAME, and the PASSWORD.
5. When finish those preparations, open the SMCE-gd and choose the "arduino.ino" file compile it and run, and open the "androidApp" by Android studio and run the project.
 
