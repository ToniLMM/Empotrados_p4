# Practice 4: Follow line

This project implements a distributed embedded system composed of an Arduino and an ESP32, where each device has clearly separated responsibilities:

- **Arduino**: real-time robot control (line following, motors, sensors, and obstacle detection).
- **ESP32**: high-level communication (WiFi + MQTT) and event reporting to a remote server.

This separation ensures that time-critical control tasks are not affected by network latency or connectivity issues.

## Arduino - ESP32 Serial Communication

A custom, lightweight serial protocol is used to exchange messages efficiently:

- Each event is sent as:
  - A **numeric action code**
  - Optionally, a **second line containing a parameter**

This approach avoids complex parsing and minimizes latency.

### Implemented Action Codes

| Code | Action |
|-----:|--------|
| 0 | START_LAP |
| 1 | END_LAP |
| 2 | OBSTACLE_DETECTED |
| 3 | LINE_LOST |
| 5 | INIT_LINE_SEARCH |
| 6 | STOP_LINE_SEARCH |
| 7 | LINE_FOUND |
| 8 | VISIBLE_LINE |

## WiFi and MQTT Communication (ESP32)

The ESP32 connects to a WiFi network and publishes messages to a remote MQTT broker.

Each serial event received from the Arduino is converted into a JSON-formatted MQTT message containing:

- `team_name`
- `id`
- `action`
- Associated parameters (`time`, `distance`, `value`)

## Lap Control and Synchronization

The lap start is synchronized between Arduino and ESP32:

1. ESP32 sends a start request (1) via UART.
2. Arduino confirms readiness by responding with 0.
3. ESP32 publishes START_LAP and starts timing.

This ensures that a lap never starts unless WiFi and MQTT connections are active.

## Robot Control (Arduino)

The robot uses three analog infrared sensors (left, center, right).

- A fixed threshold is applied to detect the line.
- Control logic is rule-based, not PID-based.

No PID controller is used. Instead, motor speeds are adjusted using conditional logic depending on which sensors detect the line. This simplifies implementation and proved sufficient for the track.

Examples:

- Center sensor active -> straight motion
- Left sensor active -> corrective right turn
- All sensors lost -> recovery turn based on last valid detection

## Obstacle Detection and Braking

An ultrasonic distance sensor is used with fast polling and basic filtering:

- Valid distance range: 2–30 cm
- Last valid measurement is stored
- Distance is sampled every 15 ms

| Distance | Behavior                            |
| -------: | ----------------------------------- |
|   5–8 cm | Emergency braking + lap termination |
|  9–12 cm | Strong speed reduction              |
| 13–18 cm | Moderate speed reduction            |
| 19–30 cm | Slight speed adjustment             |

## State Management

The system uses a flag-based state machine to ensure safe operation. This prevents invalid transitions such as:

- Sending events after lap completion
- Restarting after a critical stop
- Overlapping control action

## Operating System and Task Scheduling

The system does not use FreeRTOS. Both the Arduino and the ESP32 run on a classic loop-based execution model. Task scheduling is handled manually using `millis()` and fixed time intervals.
This approach was chosen to keep the system simple and deterministic.

## Conclusion

This implementation prioritizes simplicity, reliability, and real-time safety.
By avoiding unnecessary complexity such as PID controllers or an RTOS,
the system remains easy to debug while still meeting all functional requirements.


## Videos

#### Tests on a homemade circuit

https://github.com/user-attachments/assets/8b9784d5-a78a-4695-902f-df368e28e332

Drive URL: https://drive.google.com/file/d/1K9MRWRwTEOqIRizdOu6MiQWcKssgHrNU/view?usp=sharing

#### Exam video

https://github.com/user-attachments/assets/79cd2445-0f08-4cd3-b87e-94db95cffe04

Drive URL: https://drive.google.com/file/d/1EhnQ9-Ppim4SljsFKkLEmgnoPcObYizF/view?usp=sharing

