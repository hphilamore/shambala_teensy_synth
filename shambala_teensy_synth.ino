// Advanced Microcontroller-based Audio Workshop
//
// http://www.pjrc.com/store/audio_tutorial_kit.html
// https://hackaday.io/project/8292-microcontroller-audio-workshop-had-supercon-2015
// 
// Part 2-8: Oscillators
// good sensitivity parameters to use in in touch.c 
//#define current_set 10 //2   // 0 to 15 - current to use, value is 2*(current+1)
//#define nscan_set 31 //9     // number of times to scan, 0 to 31, value is nscan+1
//#define prescale_set 3 //2  // prescaler, 0 to 7 - value is 2^(prescaler+1)

#include <Bounce.h>

//Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);  // 15 = 15 ms debounce time
Bounce button2 = Bounce(2, 15);

double touch0 = 0;
//int touch1 = A5;

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <RollingAverage.h>
#include <CapacitiveSensor.h>

#define NUM_SAMPLES 3 //
int CURRENT;
int NSCAN;
int PRESCALE;


// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=120,177
AudioSynthNoisePink      pink1;          //xy=172,438
AudioSynthWaveformSine   sine1;          //xy=173,361
AudioSynthWaveformSineModulated sine_fm1;       //xy=178,278
AudioMixer4              mixer1;         //xy=403,188
AudioEffectEnvelope      envelope1;      //xy=501,283
AudioMixer4              mixer2;         //xy=588,186
AudioOutputI2S           i2s1;           //xy=729,190
AudioConnection          patchCord1(waveform1, 0, mixer1, 0);
AudioConnection          patchCord2(waveform1, sine_fm1);
AudioConnection          patchCord3(pink1, 0, mixer1, 3);
AudioConnection          patchCord4(sine1, 0, mixer1, 2);
AudioConnection          patchCord5(sine_fm1, 0, mixer1, 1);
AudioConnection          patchCord6(mixer1, 0, mixer2, 0);
AudioConnection          patchCord7(mixer1, envelope1);
AudioConnection          patchCord8(envelope1, 0, mixer2, 1);
AudioConnection          patchCord9(mixer2, 0, i2s1, 0);
AudioConnection          patchCord10(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=412,39
// GUItool: end automatically generated code

CapacitiveSensor   cs_3_4 = CapacitiveSensor(3,4); 
RollingAverage <int, 32> Cap1Average; // power of 2
RollingAverage <int, 64> Cap2Average; 



void setup() {
  cs_3_4.set_CS_AutocaL_Millis(0xFFFFFFFF);
  
  Serial.begin(9600);
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.32);
  
  
  //pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  mixer1.gain(0, 0.75);
  mixer1.gain(1, 0.0);
  mixer1.gain(2, 0.0);
  mixer1.gain(3, 0.0);
  mixer2.gain(0, 0.15);
  mixer2.gain(1, 0.0);
  mixer2.gain(2, 0.0);
  mixer2.gain(3, 0.0);
  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(0.75);
  waveform1.frequency(50);
  waveform1.pulseWidth(0.15);
  sine_fm1.frequency(440);
  sine_fm1.amplitude(0.75);
  sine1.frequency(200);
  sine1.amplitude(0.75);
  pink1.amplitude(0.75);
  envelope1.attack(10);
  envelope1.hold(10);
  envelope1.decay(25);
  envelope1.sustain(0.4);
  envelope1.release(70);

  //setTouchReadSensitivity(5, 5, 5);
}

int waveform_type = WAVEFORM_SAWTOOTH;
int mixer1_setting = 0;
int mixer2_setting = 0;
elapsedMillis timeout = 0;
bool mixer2_envelope = false;

void loop() {
  //button0.update();
  button1.update();
  button2.update();

  // Left changes the type of control waveform
//  if (button0.fallingEdge()) {
//    Serial.print("Control waveform: ");
//    if (waveform_type == WAVEFORM_SAWTOOTH) {
//      waveform_type = WAVEFORM_SINE;
//      Serial.println("Sine");
//    } else if (waveform_type == WAVEFORM_SINE) {
//      waveform_type = WAVEFORM_SQUARE;
//      Serial.println("Square");
//    } else if (waveform_type == WAVEFORM_SQUARE) {
//      waveform_type = WAVEFORM_TRIANGLE;
//      Serial.println("Triangle");
//    } else if (waveform_type == WAVEFORM_TRIANGLE) {
//      waveform_type = WAVEFORM_PULSE;
//      Serial.println("Pulse");
//    } else if (waveform_type == WAVEFORM_PULSE) {
//      waveform_type = WAVEFORM_SAWTOOTH;
//      Serial.println("Sawtooth");
//    }
//    waveform1.begin(waveform_type);
//  }

  // middle button switch which source we hear from mixer1
  if (button1.fallingEdge()) {
    if (mixer1_setting == 0) {
      mixer1.gain(0, 0.75);
      mixer1.gain(1, 0.0);
      mixer1.gain(2, 0.0);
      mixer1.gain(3, 0.0);
      Serial.println("Mixer1: Control oscillator");
      mixer1_setting = 1;
    } else if (mixer1_setting == 1) {
      mixer1.gain(0, 0.0);
      mixer1.gain(1, 0.75);
      mixer1.gain(2, 0.0);
      mixer1.gain(3, 0.0);
      Serial.println("Mixer1: Frequency Modulated Oscillator");
      mixer1_setting = 2;
    } else if (mixer1_setting == 2) {
      mixer1.gain(0, 0.0);
      mixer1.gain(1, 0.0);
      mixer1.gain(2, 0.75);
      mixer1.gain(3, 0.0);
      Serial.println("Mixer1: Regular Sine Wave Oscillator");
      mixer1_setting = 3;
    } else if (mixer1_setting == 3) {
      mixer1.gain(0, 0.0);
      mixer1.gain(1, 0.0);
      mixer1.gain(2, 0.0);
      mixer1.gain(3, 0.75);
      Serial.println("Mixer1: Pink Noise");
      mixer1_setting = 0;
    }
  }

  // Right button activates the envelope
  if (button2.fallingEdge()) {
    mixer2.gain(0, 0.0);
    mixer2.gain(1, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope1.noteOn();
  }
  if (button2.risingEdge()) {
    envelope1.noteOff();
    timeout = 0;
  }

  // after 4 seconds of inactivity, go back to
  // steady listening intead of the envelope
  if (mixer2_envelope == true && timeout > 4000) {
    mixer2.gain(0, 0.15);
    mixer2.gain(1, 0.0);
    mixer2_envelope = false;
  }

  // use the knobs to adjust parameters
  //float knob1 = (float)analogRead(A1) / 1023.0;
  float knob2 = (float)analogRead(A2) / 1023.0;
  float knob3 = (float)analogRead(A3) / 1023.0;

 
 int t = touchRead(touch0);
 int capsense1 = Cap1Average.next(t);
 //int capsense2 = Cap2Average.next((int) ((long) cs_3_4.capacitiveSensor(NUM_SAMPLES)));

 //Serial.print(touchRead(touch0));
 //Serial.print("\t"); 
 Serial.print(capsense1);
 Serial.print("\t");
// Serial.print(capsense2);
// Serial.print("\t");

 Serial.print(t);
 Serial.print("\t");
// Serial.print(cs_3_4.capacitiveSensor(NUM_SAMPLES));
// Serial.print("\t");

 


 
  if (mixer1_setting == 1) 
  {
    Serial.print("Mixer1: Control oscillator");
  } 
  else if (mixer1_setting == 2) 
  {
    Serial.print("Mixer1: Frequency Modulated Oscillator");
  } 
  else if (mixer1_setting == 3) 
  {
    Serial.print("Mixer1: Regular Sine Wave Oscillator");
  } 
  else if (mixer1_setting == 0) 
  {
    Serial.print("Mixer1: Pink Noise");
  }

  Serial.print("\t");

  if (waveform_type == WAVEFORM_SINE) 
  {
    Serial.print("Sine");
  } 
  else if (waveform_type == WAVEFORM_SQUARE) 
  {
    Serial.print("Square");
  } 
  else if (waveform_type == WAVEFORM_TRIANGLE) 
  {
    Serial.print("Triangle");
  } 
  else if (waveform_type == WAVEFORM_PULSE) 
  {
    Serial.print("Pulse");
  } 
  else if (waveform_type == WAVEFORM_SAWTOOTH) 
  {
    Serial.print("Sawtooth");
  }

 
  Serial.print("\t");
  
  
  Serial.print("wave");
  Serial.print("\t");
  Serial.print(knob2);
  Serial.print("\t");
  Serial.print("fm");
  Serial.print("\t");
  Serial.print(knob3);
  
  
  waveform1.frequency(360 * knob2 + 0.25);
  sine_fm1.frequency(knob3 * 1500 + 50);
  sine1.frequency(knob3 * 1500 + 50);


  Serial.println("");

  delay(100);
}

//void setTouchReadSensitivity(uint8_t t_current, uint8_t num_scans, uint8_t t_prescale){
//  //check the new values are in range
//        if(t_current>15) t_current=15; 
//  if(num_scans>31) num_scans=31;
//  if(t_prescale>7) t_prescale=7;
//
//        //update the variables
//  CURRENT=t_current; //0 to 15 - current to use, value is 2*(current+1), default 2
//  NSCAN=num_scans; //number of times to scan, 0 to 31, value is nscan+1, default 9
//  PRESCALE=t_prescale; //prescaler, 0 to 7 - value is 2^(prescaler+1), default 2
//}


