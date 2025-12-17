#include <FastLED.h>

#define LED_PIN 4
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

#define PIN_ITR_LEFT A2
#define PIN_ITR_MID A1
#define PIN_ITR_RIGHT A0

#define TRIG_PIN 13
#define ECHO_PIN 12

#define PIN_Motor_STBY 3
#define PIN_Motor_AIN_1 7
#define PIN_Motor_PWMA 5
#define PIN_Motor_BIN_1 8
#define PIN_Motor_PWMB 6

unsigned long lap_start = 0;
bool running = false;
bool lap_started = false;
bool system_finished = false;
int detections = 0;
int total_readings = 0;

const int threshold = 600;

bool lost_line = false;
bool previous_line_detected = true;
int last_detection = 0;
int consecutive_lost_count = 0;
const int MAX_CONSECUTIVE_LOST = 6;

const int BASE_SPEED = 155;
const int TURN_SPEED = 115;
const int CORRECTION_SPEED = 15;
const int MAX_SPEED = 210;

int last_valid_distance = 999;
unsigned long last_distance_time = 0;
const int DISTANCE_READ_INTERVAL = 15;

bool braking = false;
unsigned long brake_start_time = 0;
const int BRAKE_TIME = 150;
bool obstacle_detected_flag = false;

void set_motors(int left, int right) {
  if (system_finished) return;

  digitalWrite(PIN_Motor_STBY, HIGH);

  left = constrain(left, -MAX_SPEED, MAX_SPEED);
  right = constrain(right, -MAX_SPEED, MAX_SPEED);

  if (left >= 0) {
    digitalWrite(PIN_Motor_BIN_1, HIGH);
    analogWrite(PIN_Motor_PWMB, left);
  } else {
    digitalWrite(PIN_Motor_BIN_1, LOW);
    analogWrite(PIN_Motor_PWMB, -left);
  }

  if (right >= 0) {
    digitalWrite(PIN_Motor_AIN_1, HIGH);
    analogWrite(PIN_Motor_PWMA, right);
  } else {
    digitalWrite(PIN_Motor_AIN_1, LOW);
    analogWrite(PIN_Motor_PWMA, -right);
  }
}

int read_quick_distance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duration <= 0) return 999;

  int distance = duration * 0.034 / 2;

  if (distance >= 2 && distance <= 30) {
    last_valid_distance = distance;
    return distance;
  }

  return last_valid_distance;
}

void show_color(CRGB c) {
  leds[0] = c;
  FastLED.show();
}

void emergency_brake() {
  digitalWrite(PIN_Motor_STBY, HIGH);

  digitalWrite(PIN_Motor_AIN_1, LOW);
  digitalWrite(PIN_Motor_BIN_1, LOW);
  analogWrite(PIN_Motor_PWMA, 140);
  analogWrite(PIN_Motor_PWMB, 140);
  delay(40);

  analogWrite(PIN_Motor_PWMA, 80);
  analogWrite(PIN_Motor_PWMB, 80);
  delay(30);

  analogWrite(PIN_Motor_PWMA, 0);
  analogWrite(PIN_Motor_PWMB, 0);
  digitalWrite(PIN_Motor_STBY, LOW);
}

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(PIN_Motor_STBY, OUTPUT);
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);

  pinMode(PIN_ITR_LEFT, INPUT);
  pinMode(PIN_ITR_MID, INPUT);
  pinMode(PIN_ITR_RIGHT, INPUT);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  show_color(CRGB::Red);

  for (int i = 0; i < 3; i++) {
    read_quick_distance();
    delay(5);
  }
}

void handle_line_detection() {
  int l = analogRead(PIN_ITR_LEFT);
  int m = analogRead(PIN_ITR_MID);
  int r = analogRead(PIN_ITR_RIGHT);

  bool left_on_line = (l > threshold);
  bool mid_on_line = (m > threshold);
  bool right_on_line = (r > threshold);
  bool lineDetected = left_on_line || mid_on_line || right_on_line;

  total_readings++;
  if (lineDetected) {
    detections++;
    consecutive_lost_count = 0;
    show_color(CRGB::Green);

    if (lost_line) {
      Serial.println("7");
      Serial.println("6");
      lost_line = false;
    }
    previous_line_detected = true;
  } else {
    consecutive_lost_count++;
    show_color(CRGB::Red);

    if (previous_line_detected && lap_started) {
      if (consecutive_lost_count >= MAX_CONSECUTIVE_LOST) {
        Serial.println("3");
        Serial.println("5");
        lost_line = true;
        previous_line_detected = false;
      }
    } else {
      previous_line_detected = false;
    }
  }

  if (braking) return;

  if (left_on_line && mid_on_line && right_on_line) {
    set_motors(BASE_SPEED + 25, BASE_SPEED + 25);
    last_detection = 0;
  } else if (mid_on_line && !left_on_line && !right_on_line) {
    set_motors(BASE_SPEED, BASE_SPEED);
    last_detection = 0;
  } else if (left_on_line && !mid_on_line && !right_on_line) {
    set_motors(TURN_SPEED - CORRECTION_SPEED, BASE_SPEED + CORRECTION_SPEED);
    last_detection = 1;
  } else if (right_on_line && !mid_on_line && !left_on_line) {
    set_motors(BASE_SPEED + CORRECTION_SPEED, TURN_SPEED - CORRECTION_SPEED);
    last_detection = 2;
  } else if (left_on_line && mid_on_line && !right_on_line) {
    set_motors(BASE_SPEED - 12, BASE_SPEED + 12);
    last_detection = 1;
  } else if (right_on_line && mid_on_line && !left_on_line) {
    set_motors(BASE_SPEED + 12, BASE_SPEED - 12);
    last_detection = 2;
  } else if (!left_on_line && !mid_on_line && !right_on_line) {
    if (last_detection == 1) {
      set_motors(-BASE_SPEED / 2, BASE_SPEED);
    } else if (last_detection == 2) {
      set_motors(BASE_SPEED, -BASE_SPEED / 2);
    } else {
      set_motors(-BASE_SPEED / 3, BASE_SPEED / 3);
    }
  } else if (left_on_line && right_on_line && !mid_on_line) {
    set_motors(BASE_SPEED / 2, BASE_SPEED / 2);
    last_detection = 0;
  }
}

void handle_obstacle_detection() {
  unsigned long currentTime = millis();

  if (currentTime - last_distance_time >= DISTANCE_READ_INTERVAL) {
    int distance = read_quick_distance();

    if (distance >= 5 && distance <= 8 && !braking && !obstacle_detected_flag) {
      braking = true;
      obstacle_detected_flag = true;
      brake_start_time = currentTime;

      emergency_brake();

      Serial.println("2");
      Serial.println(distance);

      delay(100);

      unsigned long elapsedTime = currentTime - lap_start;

      Serial.println("1");
      Serial.println(elapsedTime);

      float lineVisibility = 0.0;
      if (total_readings > 0) {
        lineVisibility = (detections * 100.0) / total_readings;
      }

      Serial.println("8");
      Serial.println(lineVisibility, 2);

      system_finished = true;
      running = false;
      lap_started = false;

      for (int i = 0; i < 2; i++) {
        show_color(CRGB::Red);
        delay(80);
        show_color(CRGB::Black);
        delay(80);
      }
      show_color(CRGB::Red);

      while (true) {
        delay(1000);
      }
    } else if (distance >= 9 && distance <= 12 && !braking) {
      int speedReduction = map(distance, 9, 12, 50, 100);
      int currentSpeed = BASE_SPEED - speedReduction;
      if (currentSpeed < 40) currentSpeed = 40;
      set_motors(currentSpeed, currentSpeed);
    } else if (distance >= 13 && distance <= 18 && !braking) {
      int speedReduction = map(distance, 13, 18, 30, 70);
      int currentSpeed = BASE_SPEED - speedReduction;
      if (currentSpeed < 60) currentSpeed = 60;
      set_motors(currentSpeed, currentSpeed);
    } else if (distance >= 19 && distance <= 25 && !braking) {
      set_motors(BASE_SPEED - 20, BASE_SPEED - 20);
    } else if (distance >= 26 && distance <= 30 && !braking) {
      set_motors(BASE_SPEED - 10, BASE_SPEED - 10);
    }

    last_distance_time = currentTime;
  }

  if (braking && currentTime - brake_start_time > BRAKE_TIME) {
    braking = false;
  }
}

void loop() {
  if (system_finished) {
    delay(1000);
    return;
  }

  if (!running && Serial.available()) {
    String received = Serial.readStringUntil('\n');
    received.trim();

    if (received == "1") {
      Serial.println("0");

      lap_start = millis();
      running = true;
      lap_started = true;
      detections = 0;
      total_readings = 0;
      consecutive_lost_count = 0;
      last_distance_time = millis();
      braking = false;
      obstacle_detected_flag = false;

      show_color(CRGB::Blue);
      delay(100);
    }
  }

  if (running && !system_finished) {
    handle_line_detection();
    handle_obstacle_detection();
  }

  delay(3);
}
