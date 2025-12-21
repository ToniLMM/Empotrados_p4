# Empotrados_p4

## ESP32

This ESP32 code serves as the IoT communication bridge for a line-following robot project. It handles WiFi connectivity, MQTT messaging, and serial communication with an Arduino that controls the physical robot.

```cpp
send_mqtt_message()
```
Formats robot events into standardized JSON messages for the MQTT server.

```cpp
read_serial2_send()
```
Listens to Arduino events and forwards them as MQTT messages with appropriate formatting.

```cpp
handle_start_lap()
```
Coordinates the start of a lap between the ESP32, Arduino, and MQTT server.

```cpp
void loop()
```
The loop prioritizes connection management, then message processing, then periodic tasks.


## Arduino

```cpp
set_motors(int left, int right) 
```
Abstracts motor control with safety constraints and direction handling.

```cpp
read_distance()
```
Provides filtered, reliable distance measurements using median filtering.

```cpp
handle_line_detection()
```
Implements a state machine for precise line following with 8 different sensor combinations.

```cpp
handle_obstacle_detection()
```
Implements progressive braking based on obstacle distance for safe stopping.


## Videos

#### Tests on a homemade circuit

https://github.com/user-attachments/assets/8b9784d5-a78a-4695-902f-df368e28e332

Drive URL: https://drive.google.com/file/d/1K9MRWRwTEOqIRizdOu6MiQWcKssgHrNU/view?usp=sharing

#### Exam video

https://github.com/user-attachments/assets/79cd2445-0f08-4cd3-b87e-94db95cffe04

Drive URL: https://drive.google.com/file/d/1EhnQ9-Ppim4SljsFKkLEmgnoPcObYizF/view?usp=sharing

