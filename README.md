# Practice 4: Follow line

This project implements a distributed embedded system composed of an Arduino and an ESP32, where each device has clearly separated responsibilities:

- **Arduino**: real-time robot control (line following, motors, sensors, and obstacle detection).
- **ESP32**: high-level communication (WiFi + MQTT) and event reporting to a remote server.

This separation ensures that time-critical control tasks are not affected by network latency or connectivity issues.

## Arduino - ESP32 Serial Communication

A **custom, lightweight serial protocol** is used to exchange messages efficiently:

- Each event is sent as:
  1. A **numeric action code**
  2. Optionally, a **second line containing a parameter**

This approach avoids complex parsing and minimizes latency.

### Implemented Action Codes

| Code | Action |
|-----:|--------|
| 0 | Start confirmation |
| 1 | END_LAP |
| 2 | OBSTACLE_DETECTED |
| 3 | LINE_LOST |
| 5 | INIT_LINE_SEARCH |
| 6 | STOP_LINE_SEARCH |
| 7 | LINE_FOUND |
| 8 | VISIBLE_LINE |

Example:


## Videos

#### Tests on a homemade circuit

https://github.com/user-attachments/assets/8b9784d5-a78a-4695-902f-df368e28e332

Drive URL: https://drive.google.com/file/d/1K9MRWRwTEOqIRizdOu6MiQWcKssgHrNU/view?usp=sharing

#### Exam video

https://github.com/user-attachments/assets/79cd2445-0f08-4cd3-b87e-94db95cffe04

Drive URL: https://drive.google.com/file/d/1EhnQ9-Ppim4SljsFKkLEmgnoPcObYizF/view?usp=sharing

