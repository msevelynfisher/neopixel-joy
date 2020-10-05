#include <Adafruit_NeoPixel.h>

// number of neopixels
#define NPIX 16

// Arduino pin connections
#define PIN_MODE 9
#define PIN_VRX A3
#define PIN_VRY A1
#define PIN_NEO 6

// mode enum
#define N_MODES 4
#define MODE_OFF 0
#define MODE_WHITE 1
#define MODE_COLOR 2
#define MODE_RAINBOW 3

bool mode_switch_state = false;
byte mode = MODE_OFF;

unsigned int t;
Adafruit_NeoPixel pixels (NPIX, PIN_NEO, NEO_GRB + NEO_KHZ800);

byte buf[NPIX][3];
int hue = 0;
int brightness = 4;

// joystick calibration
int cal_x;
int cal_y;

// convert a hue value (0 .. 255) to rgb
// brightness and saturation are not held constant
void hue2rgb(int hue, int* rgb) {
  hue += 256;
  hue %= 256;

  rgb[0] = hue;
  rgb[1] = (hue + 85) % 256;
  rgb[2] = (hue + 170) % 256;

  for (int i = 0; i < 3; i++) {
    rgb[i] -= 128;
    rgb[i] = rgb[i] * rgb[i] / 128;
  }
}

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
        buf[i][0] = 0;
        buf[i][1] = 0;
        buf[i][2] = 0;
      } else if (mode == MODE_WHITE) {
        buf[i][0] = 255;
        buf[i][1] = 255;
        buf[i][2] = 255;
      } else if (mode == MODE_COLOR) {
        int rgb[3];
        hue2rgb(hue, rgb);
        buf[i][0] = rgb[0];
        buf[i][1] = rgb[1];
        buf[i][2] = rgb[2];
      } else if (mode == MODE_RAINBOW) {
        int rgb[3];
        hue2rgb(hue + t, rgb);
        buf[i][0] = rgb[0];
        buf[i][1] = rgb[1];
        buf[i][2] = rgb[2];
      }
    }
    
    // update pixels
    pixels.clear();
    for (unsigned int i = 0; i < NPIX; ++i) {
      int r = buf[i][0] * brightness / 256;
      int g = buf[i][1] * brightness / 256;
      int b = buf[i][2] * brightness / 256;
      pixels.setPixelColor(
        i,
        (byte) r,
        (byte) g,
        (byte) b
      );
    }
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
