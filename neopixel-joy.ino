#include <Adafruit_NeoPixel.h>

// number of neopixels
#define NPIX 16

// Arduino pin connections
#define PIN_MODE 9
#define PIN_VRX A3
#define PIN_VRY A1
#define PIN_NEO 6

// mode enum
#define N_MODES 5
#define MODE_OFF 0
#define MODE_WHITE 1
#define MODE_COLOR 2
#define MODE_RAINBOW 3
#define MODE_PRIDE 4

bool mode_switch_state = false;
byte mode = MODE_OFF;

unsigned int t;
Adafruit_NeoPixel pixels (NPIX, PIN_NEO, NEO_GRB + NEO_KHZ800);

uint32_t buf[NPIX];
int hue = 0;
int brightness = 4;

// joystick calibration
int cal_x;
int cal_y;

void setup() {
  Serial.begin(9600);

  // joystick pins
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(PIN_VRX, INPUT);
  pinMode(PIN_VRY, INPUT);

  // calibrate joystick
  cal_x = analogRead(PIN_VRX);
  cal_y = analogRead(PIN_VRY);

  // initialize neopixels
  pixels.begin();
}

void loop() {

  // update hue and brightness based on joystick position
  
  int vrx = (analogRead(PIN_VRX) - cal_x) / 16;
  int vry = (cal_y - analogRead(PIN_VRY)) / 16;

  if (vrx > 2) {
    hue += 1;
  } else if (vrx < -2) {
    hue += 255; // -1 after modulo 256
  }

  if (vry > 2) {
    brightness += 1;
  } else if (vry < -2) {
    brightness -= 1;
  }
  
  hue = hue % 256;
  brightness = constrain(brightness, 4, 255);

  // loop once per "frame"
  
  unsigned long m = millis();
  while (millis() - m < 20) {

    // set pixel buffer values based on mode
    for (unsigned int i = 0; i < NPIX; ++i) {
      if (mode == MODE_OFF) {
        buf[i] = 0x00000000;
      } else if (mode == MODE_WHITE) {
        buf[i] = 0x00FFFFFF;
      } else if (mode == MODE_COLOR) {
        buf[i] = pixels.ColorHSV(256 * hue);
      } else if (mode == MODE_RAINBOW) {
        buf[i] = pixels.ColorHSV(256 * (hue + t));
      } else if (mode == MODE_PRIDE) {
        buf[i] = pixels.ColorHSV(256 * (hue + t + 256 * i / NPIX));
      }
    }
    
    // update pixels
    pixels.clear();
    for (unsigned int i = 0; i < NPIX; ++i) {
      pixels.setPixelColor(i, buf[i]);
    }
    pixels.setBrightness(brightness);
    pixels.show();

    // change mode
    if (!digitalRead(PIN_MODE) && !mode_switch_state) {
      mode_switch_state = true;
      mode = (mode + 1) % N_MODES;
    } else if(digitalRead(PIN_MODE) && mode_switch_state) {
      mode_switch_state = false;
    }
  
  }

  // increase 8-bit timer per "frame"
  t = (t + 4) % 256;
}
