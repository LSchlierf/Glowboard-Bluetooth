#include <BluetoothSerial.h>
#include <Adafruit_NeoPixel.h>

//adjust the following values to fit your setup:

//the arduino pin your button is connected to:
#define button_pin 32

//the arduino pin your mode switch is connected to:
#define switch_pin 15

//the arduino pin that the hall effect sensors data pin is connected to:
#define hall_pin 25

//the arduino pin the LEDS are connected to:
#define led_pin 27

//the length of your ws2812b strips in pixels:
#define LED_COUNT 50

//how many pixels long you want the running strip to be:
#define effect_length 10 

//which direction the effect should run
#define effect_direction true

//number of pixels per meter:
#define pixel_density 60 

//wheel diameter in cm:
#define wheel_diameter 7

//how often the hall effect sensor updates per full rotation:
#define hall_resolution 8

//adjust the above values to fit your setup

#define MODE_OFF -6

const float effect_increment = (pixel_density * wheel_diameter * 0.0314 / hall_resolution);

int tempo[] = {0, 20, 20, 30, 600, 40};
int tempoincrement[] = {0, 3, 3, 3, 50, 3};
int tempomin[] = {0, 0, 0, 0, 0, 0};
int tempomax[] = {0, 100, 100, 100, 1000, 50};
/*
commands:
00: new tracking mode
01: new running mode
02: speed up
03: slow down
04: custom speed setting
05: set brightness
*/
/*
modes:
-5: strobe lights
-4: christmas lights
-3: rainbow strip
-2: unicolor rainbow
-1: running rainbow
 0: street lights
 1: highlight red
 2: highlight green
 3: highlight blue
 4: tracking rainbow
 5: highlight multi color
 6: highlight red & blue
*/
int pixel0, pixel1, timer, progress, mode, lasttime, timepassed;
const int length = effect_length - 1;
const int last = LED_COUNT - 1;
const int slast = last - length;
const int limit = LED_COUNT + LED_COUNT;
volatile float position = 22;
float increment = 65536 / limit;
bool switchmode, direction, tracking, hall;
uint16_t color;
uint32_t colorbuf;

BluetoothSerial Bluetooth;
Adafruit_NeoPixel strip(LED_COUNT, led_pin, NEO_GRB + NEO_KHZ800);

void setup() {
  Bluetooth.begin("Longboard");
  pinMode(switch_pin, INPUT);
  pinMode(button_pin, INPUT);
  pinMode(hall_pin, INPUT);
  switchmode = digitalRead(switch_pin);
  lasttime = millis();
  strip.begin();
  runAnimation();
}

void loop() {
  if(tracking){
    if(digitalRead(hall_pin) != hall){
      if(effect_direction){
        position -= effect_increment;
        if (position < 0) {
          position += limit;
        }
      }
      else{
        position += effect_increment;
        if(position >= limit){
          position -= limit;
        }
      }
      hall = digitalRead(hall_pin);
      runAnimation();
    }
  }

  if(Bluetooth.available()){
    readBluetooth();
    runAnimation();
  }
  
  if (digitalRead(button_pin) == HIGH) {
    timer = 0;
    while (digitalRead(button_pin) == HIGH) {
      delay(100);
      timer++;
      if (timer > 14) {
        longPress(true);
        while (digitalRead(button_pin) == HIGH) {
        }
        delay(100);
        break;
      }
    }
    if (timer < 15) {
      shortPress();
    }
  }

  if (digitalRead(switch_pin) != switchmode) {
    if (mode != 0) {
      longPress(false);
      shortPress();
    }
    switchmode = digitalRead(switch_pin);
  }

  if (mode < 0) {
    checkTime();
  }
}

void longPress(bool updateLeds) {
  if (mode != 0) {
    if (mode > 0) {
      tracking = false;
    }
    mode = 0;
  }
  else {
    if (digitalRead(switch_pin) == LOW) {
      mode = 1;
      tracking = true;
    }
    else {
      mode = -1;
    }
  }
  if(updateLeds)
  {
    runAnimation();
  }
}

void shortPress() {
  if (mode > 0) {
    mode++;
    if (mode > 6) {
      mode = 1;
    }
  }
  else if (mode < 0) {
    mode--;
    if (mode < (MODE_OFF + 1)) {
      mode = -1;
    }
  }
  else {
    if (digitalRead(switch_pin) == LOW) {
      mode = 1;
      tracking = true;
    }
    else {
      mode = -1;
    }
  }
  runAnimation();
}

void checkTime() {
  if(mode == MODE_OFF){
    return;
  }
  timepassed = millis() - lasttime;
  if(timepassed > tempo[abs(mode)]){
    runAnimation();
    lasttime = millis();
  }
}

void runAnimation() {
  strip.clear();
  switch (mode) {
    case MODE_OFF: //off
      break;

/* //EPILEPSY WARNING: READ DISCLAIMER IN README FIRST!
    case -5: //strobe lights
      	color++;
        if(color > 5){
          color = 0;
        }
        if((color == 2) || (color == 4)){
          strip.fill(strip.Color(127, 127, 127));
        }
        break;
*/

    case -4: //weihnachten
      direction = !direction;
      for(int i = 0; i < LED_COUNT; i += 5){
        if((i%10 == 0) == direction){
          for(int j = 0; j < 5; j++){
            strip.setPixelColor((i + j), strip.Color(255, 0, 0));
          }
        }
        else {
          for(int j = 0; j < 5; j++){
            strip.setPixelColor((i + j), strip.Color(0, 255, 0));
          }
        }
      }
      break;

    case -3: //bouncing strip
      color += 120;
      colorbuf = strip.ColorHSV(color);
      if (direction == true) {
        pixel0++;
        if ((pixel0 + length) > last - 1) {
          direction = false;
        }
      }
      else {
        pixel0--;
        if (pixel0 < 1) {
          direction = true;
        }
      }
      strip.fill(colorbuf, pixel0, effect_length);
      break;

    case -2: //unicolor rainbow
      color += 120;
      colorbuf = strip.ColorHSV(color);
      strip.fill(colorbuf);
      break;

    case -1: //running rainbow
      color += 120;
      for(int i = 0; i < LED_COUNT; i++){
        colorbuf = strip.ColorHSV((color + (i * increment)));
        strip.setPixelColor(i, colorbuf);
      }
      break;

    case 0: //Street lights
      if(effect_direction){
        colorbuf = strip.Color(255, 0, 0);
        strip.fill(colorbuf, 0, effect_length);
        colorbuf = strip.Color(255, 255, 255);
        strip.fill(colorbuf, slast, effect_length);
      }
      else{
        colorbuf = strip.Color(255, 255, 255);
        strip.fill(colorbuf, 0, effect_length);
        colorbuf = strip.Color(255, 0, 0);
        strip.fill(colorbuf, slast, effect_length);
      }
      break;

    case 1: //highlight red
      colorbuf = strip.Color(255, 0, 0);
      drawStrip();
      break;

    case 2: //highlight green
      colorbuf = strip.Color(0, 255, 0);
      drawStrip();
      break;

    case 3: //highlight blue
      colorbuf = strip.Color(0, 0, 255);
      drawStrip();
      break;

    case 4: //rainbow
      color = abs(position * 655.36);
      for(int i = 0; i < LED_COUNT; i++){
        colorbuf = strip.ColorHSV((color + (i * increment)));
        strip. setPixelColor((last - i), colorbuf);
      }
      break;

    case 5: //color changing highlights
      color += 120;
      colorbuf = strip.ColorHSV(color);
      drawStrip();
      break;

    case 6: //highlights blue & red
      pixel0 = abs(position);
      pixel1 = pixel0 + length;
      if (pixel1 > limit) {
        pixel1 -= limit;
      }
      colorbuf = strip.Color(255, 0, 0);
      if (pixel0 < pixel1) {
        strip.fill(colorbuf, pixel0, effect_length);
      }
      else {
        strip.fill(colorbuf, 0, (pixel1 + 1));
      }
      pixel0 += LED_COUNT;
      if (pixel0 > limit) {
        pixel0 -= limit;
      }
      pixel1 = pixel0 + length;
      if (pixel1 > limit) {
        pixel1 -= limit;
      }
      colorbuf = strip.Color(0, 0, 255);
      if (pixel0 < pixel1) {
        strip.fill(colorbuf, pixel0, effect_length);
      }
      else {
        strip.fill(colorbuf, 0, (pixel1 + 1));
      }
      break;

    default:
      mode = 0;
      runAnimation();
      break;
  }
  strip.show();
}

void drawStrip() {
  pixel0 = abs(position);
  if (pixel0 > last) {
    pixel0 -= LED_COUNT;
  }
  strip.fill(colorbuf, pixel0, effect_length);
  if(pixel0 > slast){
    strip.fill(colorbuf, 0, (length - (last - pixel0)));
  }
}

int gettempo(int val) {
  return map(val, 255, 0, tempomin[abs(mode)], tempomax[abs(mode)]);
}

void readBluetooth() {
  switch(Bluetooth.read()){
    case 0: //new tracking mode
      mode = Bluetooth.read();
      tracking = true;
      break;

    case 1: //new running mode
      mode = 0 - Bluetooth.read();
      tracking = false;
      if(mode == 0) {
        mode = MODE_OFF;
      }
      break;

    case 2: //speed up
      tempo[abs(mode)] -= tempoincrement[abs(mode)];
      if(tempo[abs(mode)] < 0){
        tempo[abs(mode)] = 0;
      }
      break;
    
    case 3: //slow down
      tempo[abs(mode)] += tempoincrement[abs(mode)];
      break;

    case 4: //custom speed
      tempo[abs(mode)] = gettempo(Bluetooth.read());
      break;

    case 5: //set brightness
      strip.setBrightness(Bluetooth.read());
      break;
  }
}
