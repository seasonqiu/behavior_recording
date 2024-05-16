/*
  Modified code from Nathan Seidle for the purpose of load cell detection
  By: Tian Qiu
  MIMO Lab
  Date: Feburary 28th, 2024

  This is a code for dynamically detecting load cell signal in 80 Hz.
  It's used in the motor task where the animal has to pull on the lever
  to acquire reward.

  It require a load cell connection to the HX711 chip (using code
  developed by Nathan Seidle, description is followed after this)
  
  Setup:
  two led
  
    one led signal the lever availability to be pulled
    one led signal the reward availability

  New features:
    Add the flashing and waveform analysis
        Read the raising and falling signal
*/
/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This is the calibration sketch. Use it to determine the calibration_factor that the main example uses. It also
 outputs the zero_factor useful for projects that have a permanent mass on the scale in between power cycles.

 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Use this calibration_factor on the example sketch

 This example assumes pounds (lbs). If you prefer kilograms, change the Serial.print(" lbs"); line to kg. The
 calibration factor will be significantly different but it will be linearly related to lbs (1 lbs = 0.453592 kg).

 Your calibration factor may be very positive or very negative. It all depends on the setup of your scale system
 and the direction the sensors deflect from zero state
 This example code uses bogde's excellent library: https://github.com/bogde/HX711
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE
 Arduino pin 2 -> HX711 CLK
 3 -> DOUT
 5V -> VCC
 GND -> GND

 Most any pin on the Arduino Uno will be compatible with DOUT/CLK.

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

*/

#include "HX711.h"

// 1. define the output pins:
#define DOUT  3
#define CLK  2
#define AOUT 6
#define WAITOUT  9
#define HOLDOUT 10
#define REWARDOUT 11
#define SYNC 13

// 2. define the state string
String state;
bool weight_check = false;
bool repeat = true;
bool pull_time = 0;
bool sync_state = 0;
int repeat_time = 1;

// 3. define float type timekeeping variables:
unsigned long timekeep;
unsigned long timekeep_weight;
unsigned long time_difference;

// 4. define float type duration variables:
unsigned long duration_wait = 50;
unsigned long duration_hold = 50;
unsigned long duration_release = 50;
unsigned long duration_punish = 50;
unsigned long duration_grace_period = 2000;
unsigned long duration_reward = 1000;

// variable to manipulate reward amount if certain amoung of trials are correct
int consecutive_correct = 0;

// 5. define float type required input for the task:
float weight_threshold = 20.0;

// 6. define/initialize float type behavior input:
float weight = 0.0;

String event_name = "";

HX711 scale;

float calibration_factor = 7050; //-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");
  pinMode(WAITOUT, OUTPUT); // ON: lever available, OFF: lever unavailable
  pinMode(HOLDOUT, OUTPUT); // ON: indicate to keep holding, OFF: indicate to release
  pinMode(REWARDOUT, OUTPUT); // ON: reward available, OFF: reward unavailable
  pinMode(SYNC, INPUT);
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  timekeep = millis();
  state = "iti";
  log_event(state,sync_state,duration_hold);
  digitalWrite(WAITOUT,HIGH);
}

void log_event(String event, bool sync_state, unsigned long duration_hold) {
  Serial.print(timekeep);
  Serial.print(",");
  Serial.print(event);
  Serial.print(",");
  Serial.print(weight, 2);
  Serial.print(",");
  Serial.print(sync_state);
  Serial.print(";");
  Serial.print(duration_hold);
  Serial.println();
  }
bool check_weight() {
  if(weight>weight_threshold)
  {
    timekeep_weight = millis();
    weight_check = true;
    if(event_name != "pressed")
    {
      if(event_name != "hold")
      {
        event_name = "pressed";
        log_event(event_name,sync_state,duration_hold);
        }

    }
  }
  else
  {
      timekeep_weight = millis();
      weight_check = false;
      if (state == "iti"){
        if(event_name == "pressed"){
          event_name = "released";
          log_event(event_name,sync_state,duration_hold);
        }
      }
  }
  return weight_check;
  }
  
bool check_repeat() {
  if(pull_time<repeat_time) {
    return true;
    }
  else{
    return false;
    }
  }  
  
String state_transition() {
  if(state == "iti"){
    if(check_weight())
    {
      timekeep = millis();
      state = "wait";
      log_event(state,sync_state,duration_hold);
      }
    }
  else if(state == "wait") {
    time_difference = millis() - timekeep;
    if(time_difference >= duration_wait){
      digitalWrite(WAITOUT,LOW);
      digitalWrite(HOLDOUT, HIGH);
      state = "hold";
      pull_time += 1;
      timekeep = millis();
      }
    else if(weight_check == false) {
      digitalWrite(WAITOUT,LOW);
      timekeep = millis();
      state = "punish";
      log_event(state,sync_state,duration_hold);
      }
    }
  else if(state == "hold") {
    time_difference = millis() - timekeep;
    if(time_difference >= duration_hold){
      digitalWrite(HOLDOUT,LOW);
      if(weight_check == false){
        if(pull_time == repeat_time) {
          digitalWrite(REWARDOUT,HIGH);
          timekeep = millis();
          state = "reward";
          consecutive_correct = consecutive_correct + 1;
          pull_time = 0;
          log_event(state,sync_state,duration_hold);
          }
        else{
          timekeep = millis();
          state = "release";
          pull_time += 1;
          log_event(state,sync_state,duration_hold);
          }
        }
      else{
        timekeep = millis();
        state = "overtime";
        log_event(state,sync_state,duration_hold);
        }
      }
    else if(weight_check == false) {
      digitalWrite(HOLDOUT,LOW);
      timekeep = millis();
      state = "punish";
      log_event(state,sync_state,duration_hold);
      }
    }
  else if (state == "release") {
    time_difference = millis() - timekeep;
    if (time_difference >= duration_release){
      digitalWrite(HOLDOUT, HIGH);
      state = "hold";
      timekeep = millis();
      }
    else{
      state = "overtime";
      timekeep = millis();
      }
    }
  else if(state == "reward"){
    time_difference = millis() - timekeep;
    if(time_difference >= duration_reward){
      digitalWrite(REWARDOUT,LOW);
      timekeep = millis();
      state = "iti";
      if(consecutive_correct>=10){
        consecutive_correct = 0;
        duration_hold = duration_hold * 1.45;
        }
      digitalWrite(WAITOUT, HIGH);
      log_event(state,sync_state,duration_hold);
      }
    }
  else if(state == "overtime"){
    time_difference = millis() - timekeep;
    if(time_difference > duration_grace_period){
      digitalWrite(REWARDOUT,LOW);
      timekeep = millis();
      state = "punish";
      log_event(state,sync_state,duration_hold);
      }
    else if(weight_check == false){
      if(check_repeat()){
        digitalWrite(HOLDOUT, HIGH);
        timekeep = millis();
        state = "hold";
        log_event(state,sync_state,duration_hold);
        }
      else{
        digitalWrite(REWARDOUT,HIGH);
        timekeep = millis();
        state = "reward";
        consecutive_correct = consecutive_correct + 1;
        log_event(state,sync_state,duration_hold);
        }

      }
    }
    
  else if(state == "punish") {
    time_difference = millis() - timekeep;
    if(time_difference > duration_punish){
      Serial.println();
      digitalWrite(WAITOUT, HIGH);
      timekeep = millis();
      state = "iti";
      log_event(state,sync_state,duration_hold);
      }
    }
  return state;
  }
  
void loop() {
  // first thing to check always is the weight and the calibration
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  sync_state = digitalRead(SYNC);
  weight = scale.get_units(); // lbs
  log_event("weight_check",sync_state,duration_hold);
//  Serial.print(weight/5);
  analogWrite(AOUT, weight/5);
  weight_check = check_weight();
  state = state_transition();
}
