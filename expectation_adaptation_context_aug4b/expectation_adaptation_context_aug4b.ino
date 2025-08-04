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
#define SOUND 8
#define GREENOUT  9
#define BLUEOUT 10
#define REWARDCUE 11
#define REWARDOUT 7
#define CONTEXT 12
#define SYNC 13

// 2. define the state string
String state;
bool baseline = true;
unsigned long baseline_last_switch = 0;
const unsigned long baseline_interval = 3600000; // 1 hour in milliseconds
bool weight_check_lever = false;
bool weight_check_loadcell = false;
bool weight_check = false;
int pull_time = 0;
bool sync_state = 0;
bool reward_check = false;
bool random_check = false;
int trial_total = 0;
int correct_trial = 0;
int reward_total = 0;
int which_light;
int reward_times;
int reward_count = 0;
// 3. define float type timekeeping variables:
unsigned long timekeep;
unsigned long timekeep_weight;
unsigned long time_difference;

// 4. define float type duration variables:
unsigned long duration_iti = 10000; // add 5 seconds everytime they trigger
unsigned long duration_wait = 10;
unsigned long duration_hold = 2000;
unsigned long duration_reward = 2000;
long duration_hold_random;

// variable to manipulate reward amount if certain amoung of trials are correct
int consecutive_correct_hold = 0;

// 5. define float type required input for the task:
float weight_lever_threshold = 20.0;
float weight_loadcell_threshold = 5.0;

// 6. define/initialize float type behavior input:
float weight_lever = 0.0;
float weight_loadcell = 0.0;

// 7. define sound
float high_frequency = 10000;
float low_frequency = 1000;
String event_name = "";

HX711 lever;
HX711 loadcell;

float calibration_factor = 7050; //-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(921600);
  Serial.println("time,trial_total,correct_trial,event,weight_lever,weight_loadcell,which_light,duration_hold,reward_total,sync_state,weight_lever_threshold");
  pinMode(GREENOUT, OUTPUT); // ON: lever available, OFF: lever unavailable
  pinMode(BLUEOUT, OUTPUT); // ON: indicate to keep holding, OFF: indicate to release
  pinMode(REWARDOUT, OUTPUT); // ON: reward available, OFF: reward unavailable
  pinMode(REWARDCUE, OUTPUT);
  pinMode(CONTEXT, OUTPUT); // ON: reversal, OFF: baseline
  pinMode(SYNC, INPUT);
  pinMode(SOUND, OUTPUT);
  lever.begin(DOUT1, CLK1);
  lever.set_scale();
  lever.tare(); //Reset the scale to 0
  loadcell.begin(DOUT2, CLK2);
  loadcell.set_scale();
  loadcell.tare(); //Reset the scale to 0

  long zero_factor_lever = lever.read_average(); //Get a baseline reading
  long zero_factor_loadcell = loadcell.read_average(); //Get a baseline reading
  timekeep = millis();
  state = "iti";
  //digitalWrite(GREENOUT,HIGH);
  log_event(state,sync_state,duration_hold);
}

void log_event(String event, bool sync_state, unsigned long duration_hold) {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(trial_total);
  Serial.print(",");
  Serial.print(correct_trial);
  Serial.print(",");
  Serial.print(event);
  Serial.print(",");
  Serial.print(weight_lever, 2);
  Serial.print(",");
  Serial.print(weight_loadcell, 2);
  Serial.print(",");
  Serial.print(which_light);
  Serial.print(",");
  Serial.print(duration_hold);
  Serial.print(",");
  Serial.print(reward_total);
  Serial.print(",");
  Serial.print(sync_state);
  Serial.print(",");
  Serial.print(weight_lever_threshold);
  Serial.println();
  }
bool check_weight_lever(float weight, float weight_threshold) {
  if(weight>weight_threshold)
  {
    timekeep_weight = millis();
    weight_check_lever = true;
    if(event_name != "pressed")
    {
      if(event_name != "hold")
      {
        event_name = "pressed";
        //log_event(event_name,sync_state,duration_hold);
        }

    }
  }
  else
  {
      timekeep_weight = millis();
      weight_check_lever = false;
      if (state == "iti"){
        if(event_name == "pressed"){
          event_name = "released";
          //log_event(event_name,sync_state,duration_hold);
        }
      }
  }
  return weight_check_lever;
  }  

bool check_weight_loadcell(float weight, float weight_threshold) {
  if(weight>weight_threshold)
  {
    timekeep_weight = millis();
    weight_check_loadcell = true;
    if(event_name != "pressed")
    {
      if(event_name != "hold")
      {
        event_name = "pressed";
        //log_event(event_name,sync_state,duration_hold);
        }

    }
  }
  else
  {
      timekeep_weight = millis();
      weight_check_loadcell = false;
      if (state == "iti"){
        if(event_name == "pressed"){
          event_name = "released";
          //log_event(event_name,sync_state,duration_hold);
        }
      }
  }
  return weight_check_loadcell;
  }  

int blue_green_generator(){
  which_light = random(2);
  if (which_light == 0){
    digitalWrite(GREENOUT,HIGH);
    if (baseline) {
      digitalWrite(CONTEXT,LOW);
      reward_times = 3;
    }
    else{
      digitalWrite(CONTEXT,HIGH);
      reward_times = 1;
    }
    }
  else{
    digitalWrite(BLUEOUT,HIGH);
    if (baseline) {
      reward_times = 1;
    }
    else {
      reward_times = 3;
    }
    }
  return which_light;
  }  

void light_off(int light_color){
  if (light_color == 0){ // light_color is 0 if it's green
    digitalWrite(GREENOUT,LOW);
    }
  else{ // light_color is 1 if it's blue
    digitalWrite(BLUEOUT,LOW);
    }
  reward_check = true;
  }
void deliver_reward(){
  delay(500);
  digitalWrite(REWARDOUT,HIGH);
  delay(1000);
  digitalWrite(REWARDOUT,LOW);
  reward_total += 1;
  }  
String state_transition() {
  if(state == "iti"){
    time_difference = millis() - timekeep;
    if(weight_check_lever){
      timekeep = millis();
      }
    else if (time_difference >= duration_iti) {
      if(weight_check_lever == false) {
          timekeep = millis();
          state = "wait";
          which_light = blue_green_generator();
          //digitalWrite(GREENOUT,HIGH);
          log_event(state,sync_state,duration_hold);
          }
      else{
        timekeep = millis();
        }
      }
    }
  else if(state == "wait") {
    time_difference = millis() - timekeep;
    if(weight_check_lever) {
      if(time_difference >= duration_wait){
        if(baseline){
          if(which_light == 0){
            tone(SOUND,high_frequency);
          }
          else {
            tone(SOUND,low_frequency);
          }
        }
        else{
          if(which_light == 1){
            tone(SOUND,high_frequency);
          }
          else {
            tone(SOUND,low_frequency);
          }
        }

        pull_time += 1;
        timekeep = millis();
        state = "hold";
        log_event(state,sync_state,duration_hold);
        }
      }
    }
  else if(state == "hold") {
    time_difference = millis() - timekeep;
    if(time_difference >= duration_hold){
      timekeep = millis();
      light_off(which_light);
      //digitalWrite(GREENOUT,LOW);
      digitalWrite(REWARDCUE,HIGH);
      noTone(SOUND);
      state = "reward_available";
      log_event(state,sync_state,duration_hold);
    }
    else if(weight_check_lever == false) {
      light_off(which_light);
      noTone(SOUND);
      //digitalWrite(BLUEOUT,LOW);
      timekeep = millis();
      state = "iti";
      trial_total += 1;
      log_event(state,sync_state,duration_hold);
      }
    }
  else if(state == "reward_available"){
    
    if(weight_check_loadcell) {
      timekeep = millis();
      if(reward_times>1){
        tone(SOUND,high_frequency);
      }
      else {
        tone(SOUND,low_frequency);
      }
      state = "sound_cue_delivery";
      log_event(state,sync_state,duration_hold);
      }
  }
  else if(state == "sound_cue_delivery"){
    time_difference = millis() - timekeep;
    if(time_difference > 200){
      if(time_difference > 2200){
        state = "reward_delivery";
        log_event(state,sync_state,duration_hold);
      }
      else{
        noTone(SOUND);
      }
    }
  }
  else if(state == "reward_delivery"){
    if(reward_count<reward_times){
      deliver_reward();
      reward_count +=1;
      //Serial.println(reward_count);
      timekeep = millis();
            if(consecutive_correct_hold >= 5){
        duration_hold+=100;
        consecutive_correct_hold = 0;
      }
      else if(duration_hold < 2000){
        consecutive_correct_hold += 1;
      }
      event_name = "reward_delivered";
      log_event(event_name,sync_state,duration_hold);
      }
    else{
      reward_count = 0;
      digitalWrite(REWARDCUE,LOW);
      timekeep = millis();
      state = "iti";
      trial_total += 1;
      correct_trial += 1;
      log_event(state,sync_state,duration_hold);
    }
  }

  return state;
  }
  
void loop() {
  // Toggle baseline every hour
  if (millis() - baseline_last_switch >= baseline_interval) {
    baseline = !baseline;
    baseline_last_switch = millis();
    log_event(baseline ? "baseline_on" : "baseline_off", sync_state, duration_hold);
  }
  // first thing to check always is the weight and the calibration
  lever.set_scale(calibration_factor); //Adjust to this calibration factor
  loadcell.set_scale(calibration_factor); //Adjust to this calibration factor
  sync_state = digitalRead(SYNC);
  weight_lever = lever.get_units(); // lbs
  weight_loadcell = loadcell.get_units();
  log_event("weight_check",sync_state,duration_hold);
  analogWrite(AOUT, weight_lever/5);
  weight_check_lever = check_weight_lever(weight_lever, weight_lever_threshold);
  weight_check_loadcell = check_weight_loadcell(weight_loadcell, weight_loadcell_threshold);
  state = state_transition();
}
