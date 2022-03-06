# Ground-tracking LED animations for Longboards with Bluetooth support

[![wakatime](https://wakatime.com/badge/user/ee0b2e88-680b-47cf-ba7c-afd0e1637329/project/fd3403ed-9d54-4074-bf71-3785127634e4.svg)](https://wakatime.com/badge/user/ee0b2e88-680b-47cf-ba7c-afd0e1637329/project/fd3403ed-9d54-4074-bf71-3785127634e4)

This is the repository for my Glowboard Code with Bluetooth support. If you couldn't care less about Bluetooth, there is also a version without Bluetooth in [this repo of mine](https://www.github.com/LSchlierf/Glowboard).  
That one was the first version I made.

## Showcase

Here are some Imgur links to show what the finished project will look like:

[Videos of the ground tracking effects](https://imgur.com/gallery/86cq9q0)

[Videos of the animated effects]() TODO: add Imgur link

[Videos of me riding the longboard](https://imgur.com/gallery/xfXjWVt)

[Pictures of the curcuitry with Bluetooth support](https://imgur.com/gallery/vMbaO5J)

[Pictures of the first version of my board](https://imgur.com/gallery/5WJpUBA)

---

## Functionality

### General

This project uses an ESP-32 microcontroller. It handles everything, from tracking the wheel, controlling the LEDs, to Bluetooth communication. It's programmed using the Arduino framework.  
My setup uses two WS2812B LED strips in parallel with a length of 50 LEDs.  
For tracking the ground, I use a hall effect sensor that tracks magnets on one of the wheels. Everytime the wheel rotates by 1/8, the microcontroller forwards the light effect and updates the LEDs. In my experience, this works pretty well, even with higher cruising speeds.

There are currently six ground tracking effects: Red, Green, Blue, Rainbow, Multicolor, and Police Colors (Red & Blue).  

There are also  four animated modes (technically five). These do not use wheel tracking but are continuously animated. You can set the speed of these in the app.  
The first four animated modes are: a running rainbow, a unicolor rainbow, a bouncing rainbow strip and a red & green Christmas mode (doesn't really look that great when the LEDs are facing the floor).  
There is also a strobe light effect, though that one is disabled by default for safety reasons. See [Disclaimer](#disclaimer-epilepsy-warning).

You can switch between ground tracking modes and animated modes using the hardware switch, and cycle through the respective modes using the hardware button. A short press (<1.5s) changes the mode, a long press (>=1.5s) turns the lights "off" to the deafault mode. This mode displays a white strip on the front and a red strip on the rear, which I like to use for traffic safety reasons. 
This mode is also displayed on start up by default.

I'm currently also working on adding functionality to both this code and the app to make it possible to directly set the color of the ground tracking mode.

### Bluetooth support

I wrote a companion app that can control the mode, brightness and speed of the LED setup.  
Currently, there is only an Android version, however I am currently working on a Flutter version, which would work with both iOs and Android.  
The source code can be found in my corresponding [GitHub repository](https://www.github.com/LSchlierf/Glowboard-App-Android).   
You can find all the information on how to use the app there.

---

## Hardware setup

### General

As mentioned before, the project uses an ESP-32 microcontroller as the brains.  
On one wheel there are magnets that are read by a hall effect sensor (I use an OH137). This way, the microcontroller can notice when the wheel turns.   
Also connected to the microcontroller are one or more WS2812B LED strip(s in parallel).  
Since all the components operate on 5V, I use a simple power bank mounted on the bottom my longboard for power. All the other electronics (except for the hall effect sensor obviously) are mounted on a Veroboard. This makes the main curcuitry sturdier and it makes it easier to connect all the components to the microcontroller and to power.

To switch the whole thing off, I added a power switch between the power bank and the main curcuitry.

### Resistors and Capacitors

Whenever dealing with LED strips, it is recommended to use a 1000 μF capacitor across the main power lines, so add one from GND to Vdd. Watch out for polarity, negative goes to GND.

You'll also need some resistors:  
* A pull-up resistor on the hall effect sensor's data pin, I use 6KΩ.
* A pull-down resistor on the buttons data pin, I use 6KΩ here as well.
* A 470Ω resistor between the ESP-32 (pin 27 in my setup) and the LEDs' "din" pin.

>pull-up  
pull-down

"pull-up" means that a resistor is connected to high potential, aka positive power at the other end.  
"pull-down" means it's connected to low potential, aka negative power.  
So a pull-up resistor is a resistor that is connected to Vdd, a pull-down resistor is connected to GND.  
Also see the [Wikipedia entry](https://en.wikipedia.org/wiki/Pull-up_resistor).

### Wiring Diagram

Here is a wiring diagram to illustrate all the mentioned connections:

![wiring diagram](https://i.imgur.com/Xy3EWLW.png)

Most of these connections can be configured freely, see [below](#code-constants).

---

## Code

### General

The main loop of the microcontroller basically does three things:
* Run the animation.  
This includes checking the hall effect sensor if applicable, else checking the passed time for the animated modes, and updating the LEDs if necessary.
* Check for Bluetooth input.  
This checks if there is new information sent form the companion app to change the mode, speed or brightness.
* Check for hardware input.  
This includes cycling the modes with the hardware button or switching between ground tracking modes and animated modes using the hardware switch.

### Code constants

At the top of the code, there are several constants to configure:

```c++
#define button_pin 32
```
This sets the ESP-32 pin the mode hardware button is connected to.

```c++
#define switch_pin 15
```
This sets the ESP-32 pin that the mode hardware switch is connected to.

```c++
#define hall_pin 25
```
This sets the ESP-32 pin that the hall effect sensor is connected to.

```c++
#define led_pin 27
```
This sets the ESP-32 pin connected to the data pin of the WS2812B strip.

```c++
#define LED_COUNT 50
```
This sets the number of pixels your LED strip has.

```c++
#define effect_length 10 
```
This sets the length of the light effects, measured in pixels.

```c++
#define effect_direction true
```
This sets the direction that the ground tracking light effects are going to run. Unfortunately, I could only make the light effects work one way, so adjust this value to make the effects run in the direction you normally ride your longboard in. This also adjusts the positioning of the default mode.

```c++
#define pixel_density 60
#define wheel_diameter 7
#define hall_resolution 8
```
These three constants set the pixel density of your WS2812B strip (how many pixels per meter), the wheel diameter (in centimeters) and the hall resolution (how often the hall sensor updates per full rotation of the wheel) respectively.  
These three values are used to calculate at compile time how far the light effect should move per hall effect sensor update.

It is important you set all these values correctly, otherwise the whole thing will either not work or not track the ground correctly.

### Disclaimer: Epilepsy warning

I added a strobe light effect as an animated mode.  
This effect might trigger seizures in individuals with photosensitive epilepsy.  
For safety reasons, I have commented out the code that runs the animation for this mode (currently lines 215 - 225).
In this state, activating this mode with the companion app or the button will just turn the LEDs off.  
If you are absolutely certain that you want your light setup to support the strobe light effect, you can uncomment these lines.

**ONLY USE THIS MODE IF YOU ARE _ABSOLUTELY CERTAIN_ THAT NOBODY WITH PHOTOSENSITIVE EPILEPSY IS PRESENT.**

Please be careful!  
Thank you.

---

## Contact

Please do not hesitate to reach out to me if you need help, have any questions about this project or found an error.  
I can be reached via Email: [LucasSchlierf@gmail.com](mailto:LucasSchlierf@gmail.com)

Cheers :)  
Lucas
