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

https://github.com/user-attachments/assets/d043ccba-42f7-4beb-84a3-2ada2f209107

https://github.com/user-attachments/assets/8b9784d5-a78a-4695-902f-df368e28e332

#### Exam video



