#include <BluetoothSerial.h>
#include <Adafruit_NeoPixel.h>

//adjust the following values to fit your setup:

//the length of your ws2812b strips in pixels:
#define LED_COUNT 50

//the arduino pin your mode switch is connected to:
#define switch_pin 15

//the arduino pin that the hall effect sensors data pin is connected to:
#define hall_pin 25

//the arduino pin your button is connected to:
#define button_pin 32

//the arduino pin the LEDS are connected to:
#define led_pin 27

//wheel diameter in cm:
#define wheel_diameter 7

//number of pixels per meter:
#define pixel_density 60 

//how often the hall effect sensor updates per full rotation:
#define hall_resolution 8

//how many pixels long you want the running strip to be:
#define effect_length 10 

//adjust the above values to fit your setup

const float effect_increment = (pixel_density * wheel_diameter * 0.0314 / hall_resolution);

int tempo[] = {0, 20, 20, 30, 40};
int tempoincrement[] = {1, 1, 1, 1};

int modelookup[] = {0, 1, 2, 3, 4, 5, 6, -1, -2, -3, -4, -5};
/*
modes:
-5: off
-4: strobe lights
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
      position -= effect_increment;
      if (position < 0) {
        position += limit;
      }
      hall = digitalRead(hall_pin);
      runAnimation();
    }
  }

  if(Bluetooth.available()){
    readBluetooth();
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
    if (mode < -3) {
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
  if(mode == -5){
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

    case -5: //off
    
      break;

    case -4: //strobe lights
      	color++;
        if(color > 23){
          color = 0;
        }
        if(!(color%6) || !((color - 2)%6)){
          strip.fill(strip.Color(255, 255, 255));
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

      colorbuf = strip.Color(255, 0, 0);
      strip.fill(colorbuf, 0, effect_length);
      colorbuf = strip.Color(255, 255, 255);
      strip.fill(colorbuf, slast, effect_length);

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

void readBluetooth() {
  switch(Bluetooth.read()){

    case 0: //new mode
      mode = modelookup[Bluetooth.read()];
      tracking = (mode > 0);
      runAnimation();
      break;

  }
}
