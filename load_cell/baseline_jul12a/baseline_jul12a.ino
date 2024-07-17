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
#define DOUT1  3
#define DOUT2  5
#define CLK1  2
#define CLK2  4
#define AOUT 6
#define WAITOUT  9
#define HOLDOUT 10
#define REWARDOUT 11
#define SYNC 13

// 2. define the state string
String state;
bool weight_check = false;
bool repeat = true;
int pull_time = 0;
bool sync_state = 0;
bool random_check = false;
bool released_check = false;
int repeat_time = 2;

// 3. define float type timekeeping variables:
unsigned long timekeep;
unsigned long timekeep_weight;
unsigned long time_difference;

// 4. define float type duration variables:
unsigned long duration_iti = 5000;
unsigned long duration_wait = 10;
unsigned long duration_hold = 10;
unsigned long duration_hold_released = 500;
unsigned long duration_release = 5000;
unsigned long duration_release_grace_period = 5000;
unsigned long duration_reward = 2000;
long duration_hold_random;

// variable to manipulate reward amount if certain amoung of trials are correct
int consecutive_correct = 0;

// 5. define float type required input for the task:
float weight_lever_threshold = 30.0;
float weight_loadcell_threshold = 200.0;

// 6. define/initialize float type behavior input:
float weight_lever = 0.0;
float weight_loadcell = 0.0;

String event_name = "";

HX711 lever;
HX711 loadcell;

float calibration_factor = 7050; //-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(9600);
//  Serial.println("HX711 calibration sketch");
//  Serial.println("Remove all weight from scale");
//  Serial.println("After readings begin, place known weight on scale");
//  Serial.println("Press + or a to increase calibration factor");
//  Serial.println("Press - or z to decrease calibration factor");
  pinMode(WAITOUT, OUTPUT); // ON: lever available, OFF: lever unavailable
  pinMode(HOLDOUT, OUTPUT); // ON: indicate to keep holding, OFF: indicate to release
  pinMode(REWARDOUT, OUTPUT); // ON: reward available, OFF: reward unavailable
  pinMode(SYNC, INPUT);
  lever.begin(DOUT1, CLK1);
  lever.set_scale();
  lever.tare(); //Reset the scale to 0
  loadcell.begin(DOUT2, CLK2);
  loadcell.set_scale();
  loadcell.tare(); //Reset the scale to 0

  long zero_factor_lever = lever.read_average(); //Get a baseline reading
  long zero_factor_loadcell = loadcell.read_average(); //Get a baseline reading
//  Serial.print("Zero factor for lever: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
//  Serial.println(zero_factor_lever);
//  Serial.print("Zero factor for loadcell: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
//  Serial.println(zero_factor_lever);
  timekeep = millis();
  state = "wait";
  digitalWrite(WAITOUT,HIGH);
  log_event(state,sync_state,duration_hold,duration_release);
}

void log_event(String event, bool sync_state, unsigned long duration_hold, unsigned long duration_release) {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(event);
  Serial.print(",");
  Serial.print(weight_lever, 2);
  Serial.print(",");
  Serial.print(sync_state);
  Serial.print(";");
  Serial.print(duration_hold);
  Serial.print(";");
  Serial.print(duration_release);
  Serial.println();
  }
bool check_weight(float weight, float weight_threshold) {
  if(weight>weight_threshold)
  {
    timekeep_weight = millis();
    weight_check = true;
    if(event_name != "pressed")
    {
      if(event_name != "hold")
      {
        event_name = "pressed";
        log_event(event_name,sync_state,duration_hold,duration_release);
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
          log_event(event_name,sync_state,duration_hold,duration_release);
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
    time_difference = millis() - timekeep;
    if (time_difference >= duration_iti) {
      if(weight_check == false) {
          timekeep = millis();
          state = "wait";
          //digitalWrite(HOLDOUT, HIGH);
          digitalWrite(WAITOUT,HIGH);
          log_event(state,sync_state,duration_hold,duration_release);
          }
      else{
        timekeep = millis();
        }
      }
    }
  else if(state == "wait") {
    time_difference = millis() - timekeep;
    if(weight_check) {
      if(time_difference >= duration_wait){
        digitalWrite(WAITOUT,LOW);
        digitalWrite(HOLDOUT, HIGH);
        pull_time += 1;
        timekeep = millis();
        state = "hold";
        log_event(state,sync_state,duration_hold,duration_release);
        }
      }
    }
  else if(state == "hold") {
    time_difference = millis() - timekeep;
    if(time_difference >= duration_hold){
      timekeep = millis();
      digitalWrite(HOLDOUT,LOW);
      digitalWrite(REWARDOUT,HIGH);
      state = "reward";
      log_event(state,sync_state,duration_hold,duration_release);
    }
    else if(weight_check == false) {
      digitalWrite(HOLDOUT,LOW);
      timekeep = millis();
      state = "iti";
      log_event(state,sync_state,duration_hold,duration_release);
      }
    }
  else if(state == "reward"){
    time_difference = millis() - timekeep;
    if(time_difference >= duration_reward){
      digitalWrite(REWARDOUT,LOW);
      timekeep = millis();
      state = "iti";
      log_event(state,sync_state,duration_hold,duration_release);
      }
    }
  return state;
  }
  
void loop() {
  // first thing to check always is the weight and the calibration
  lever.set_scale(calibration_factor); //Adjust to this calibration factor
  loadcell.set_scale(calibration_factor); //Adjust to this calibration factor
  sync_state = digitalRead(SYNC);
  weight_lever = lever.get_units(); // lbs
  weight_loadcell = loadcell.get_units();
  log_event("weight_check",sync_state,duration_hold,duration_release);
  analogWrite(AOUT, weight_lever/5);
  weight_check = check_weight(weight_lever, weight_lever_threshold);
  state = state_transition();
}
