/**
 * MEAP CAR
 * * Controls:
 * - MIDI CC #1: Engine RPM (Speed) & Music Tempo
 * - MIDI CC #2: Ignition Key (Start/Stop Engine)
 * - Pot 0: Envelope Release Time (Space/Reverb effect)
 * - Pot 1: Complexity/Entropy (Chaos, Glitch, Intensity)
 * - Touch Pads 0-3: Switch Music Modes
 */

#define CONTROL_RATE 128
#include <Meap.h>

Meap meap;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

#include <tables/saw8192_int16.h> 
#include <tables/tri8192_int16.h> 
#include <tables/sin8192_int16.h> 
#include <tables/sq8192_int16.h>  

mOscil<saw8192_int16_NUM_CELLS, AUDIO_RATE, int16_t> osc_piston(saw8192_int16_DATA);
mOscil<saw8192_int16_NUM_CELLS, AUDIO_RATE, int16_t> osc_body(saw8192_int16_DATA);

float target_rpm = 0.0;
float current_rpm = 0.0;
float friction = 0.05;

float texture_ratio = 2.5; 

bool ignition_key = false;

int32_t current_wind_val = 0;
int wind_volume = 0;
int filter_amount = 200;

// Ducking
int engine_ducking_level = 255; 
int target_ducking_level = 255;

#define POLYPHONY 8
mOscil<sin8192_int16_NUM_CELLS, AUDIO_RATE, int16_t> osc_bank[POLYPHONY]; 
ADSR<AUDIO_RATE, AUDIO_RATE> env_bank[POLYPHONY];
int current_osc = 0; 

EventDelay metro;
bool music_playing = false;
int current_pad = -1;
long global_beat = 0; 

int style_base_root = 60; 

int scale_major[] = {0, 2, 4, 7, 9, 11, 14}; 
int scale_minor[] = {0, 2, 3, 7, 8, 10, 14}; 
int scale_cyber[] = {0, 3, 5, 7, 10, 12, 15}; 
int scale_tension[] = {0, 1, 6, 7, 10, 12, 13};

int* current_scale = scale_major;
int current_scale_size = 7;

int prog_major[] = {0, -5, 5, 7}; 
int prog_minor[] = {0, 3, -4, -2}; 
int prog_cyber[] = {0, -2, 0, 5}; 
int prog_tension[] = {0, 1, 0, 6}; 
int* current_prog = prog_major;

int pot0_release = 500;
int pot1_complexity = 0; 

void setup()
{
  Serial.begin(115200);
  meap.begin();
  Serial1.begin(31250, SERIAL_8N1, meap.MEAP_MIDI_IN_PIN, meap.MEAP_MIDI_OUT_PIN);

  MIDI.setHandleControlChange(handleControlChange);
  startMozzi(CONTROL_RATE);

  osc_piston.setFreq(10);
  osc_body.setFreq(50);

  for (int i = 0; i < POLYPHONY; i++) {
    osc_bank[i].setTable(sin8192_int16_DATA); 
    env_bank[i].setADLevels(255, 0); 
  }
  metro.start(100); 
}

void loop()
{
  audioHook();
  MIDI.read();
}

void handleControlChange(byte channel, byte number, byte value)
{
  if (number == 1) { 
    target_rpm = map(value, 0, 127, 0, 400);
    if (target_rpm < 10) target_rpm = 0;
  }

  if (number == 2) { 
    ignition_key = (value > 64);
  }
}

void triggerNote(int midi_note, int velocity, int attack, int release) {
    float freq = mtof(midi_note);
    osc_bank[current_osc].setFreq(freq);
    env_bank[current_osc].setADLevels(velocity, 0); //Velocity determines volume
    env_bank[current_osc].setTimes(attack, release, 0, release);
    env_bank[current_osc].noteOn();
    current_osc = (current_osc + 1) % POLYPHONY;
}

void updateControl()
{
  meap.readInputs();

  pot0_release = map(meap.pot_vals[0], 0, 4095, 500, 2000); 
  pot1_complexity = map(meap.pot_vals[1], 0, 4095, 0, 100);

  target_ducking_level = music_playing ? 30 : 255; 
  engine_ducking_level = (engine_ducking_level * 0.9) + (target_ducking_level * 0.1);

  float desired_rpm = ignition_key ? (target_rpm > 0 ? target_rpm : 20.0) : 0;
  current_rpm = (current_rpm * (1.0 - friction)) + (desired_rpm * friction);

  if (current_rpm > 1.0) {
     osc_piston.setFreq(current_rpm);
     osc_body.setFreq(current_rpm * texture_ratio); 
  } else {
     osc_piston.setFreq(0);
     osc_body.setFreq(0);
  }

  if (current_rpm > 10.0) {
    wind_volume = map((int)current_rpm, 0, 400, 0, 255);
    filter_amount = map((int)current_rpm, 0, 400, 240, 140);
  } else {
    wind_volume = 0;
  }

  if (music_playing && metro.ready()) {
    
    int base_tempo = map((int)current_rpm, 0, 400, 200, 80); 
    metro.start(base_tempo); 

    int prog_index = (global_beat / 16) % 4; 
    int current_root = style_base_root + current_prog[prog_index];

    // PAD LAYER
    if (global_beat % 16 == 0) {
        int pad_vel = 180; 
        int pad_rel = (pot0_release < 1500) ? 1500 : pot0_release; 
        
        triggerNote(current_root - 12, pad_vel, 1000, pad_rel); 
        triggerNote(current_root - 5, pad_vel, 1000, pad_rel); 
        triggerNote(current_root + 14, pad_vel * 0.8, 1000, pad_rel); 
    }

    // MELODY LAYER
    bool should_play = false;
    int note_idx = 0;
    int oct_shift = 0;
    int vel = 240; 

    switch (current_pad) {
      
      case 0:
        should_play = true; 
        note_idx = (global_beat % 4) * 2; 
        if (pot1_complexity > 20 && meap.irand(0, 100) < pot1_complexity) note_idx += 1;
        if (pot1_complexity > 60 && (global_beat % 2 == 0)) oct_shift = 12;
        if (pot1_complexity > 90 && (global_beat % 4 == 0)) oct_shift = 24;
        break;

      case 1:
        if (global_beat % 4 == 0) {
            should_play = true;
            note_idx = (meap.irand(0, 100) < 60) ? 0 : 3; 
            vel = 200; 
        }
        else if (pot1_complexity > 20 && (global_beat % 4 == 2 || global_beat % 4 == 3)) {
             if (meap.irand(0, 100) < 50) {
                 should_play = true;
                 int sad_notes[] = {2, 4}; // b3, b6
                 note_idx = sad_notes[meap.irand(0, 2)];
                 if (pot1_complexity > 70 && meap.irand(0, 100) < 40) note_idx = 6;
                 vel = 180; 
             }
        }
        if (pot1_complexity > 85 && should_play) oct_shift = 12; 
        break;

      case 2:
        if (global_beat % 2 == 0) should_play = true; 
        if (pot1_complexity > 30 && (global_beat % 2 != 0)) should_play = true;
        note_idx = (meap.irand(0, 100) < 70) ? 0 : 3; 
        oct_shift = -12; 
        if (pot1_complexity > 70 && (global_beat % 4 == 3)) oct_shift = 12; 
        break;

      case 3:
        if (meap.irand(0, 100) < (20 + pot1_complexity)) should_play = true;
        note_idx = meap.irand(0, current_scale_size);
        if (pot1_complexity > 50) {
            int r = meap.irand(0, 100);
            if (r < 33) oct_shift = 0;
            else if (r < 66) oct_shift = 12;
            else oct_shift = 24; 
        } else {
            oct_shift = 12;
        }
        vel = 200; 
        break;
    }

    if (should_play) {
        int interval = current_scale[note_idx % current_scale_size];
        int final_note = current_root + interval + oct_shift;
        while (final_note > 100) final_note -= 12;
        
        int melody_attack = (current_pad == 1) ? 300 : 20; 
        triggerNote(final_note, vel, melody_attack, pot0_release);
    }

    global_beat++; 
  }
  static int last_complexity = -1;
  if (abs(pot1_complexity - last_complexity) > 2) 
  {
    Serial.print("CHAOS "); 
    Serial.println(pot1_complexity);
    last_complexity = pot1_complexity;
  }
}

AudioOutput_t updateAudio() {
  int32_t engine_signal = 0;
  if (current_rpm >= 1.0) {
    int32_t combined = ((int32_t)osc_piston.next() * osc_body.next()) >> 17; 
    engine_signal = combined; 
  }

  int32_t wind_signal = 0;
  if (wind_volume > 0) {
    int16_t white_noise = rand(255) - 128;
    current_wind_val = ((current_wind_val * filter_amount) + (white_noise * (255 - filter_amount))) >>11; 
    wind_signal = (current_wind_val * wind_volume);
  }

  engine_signal = (engine_signal * engine_ducking_level) >> 8;
  wind_signal = (wind_signal * engine_ducking_level) >> 8;

  int32_t music_signal = 0;
  for(int i = 0; i < POLYPHONY; i++) {
    env_bank[i].update();
    music_signal += (int32_t)osc_bank[i].next() * env_bank[i].next();
  }
  
  music_signal = music_signal >> 11; 

  int32_t mixed_out = engine_signal + wind_signal + music_signal;

  if (mixed_out > 20000) {
    mixed_out = 20000 + (mixed_out - 20000) / 3; 
  } else if (mixed_out < -20000) {
    mixed_out = -20000 + (mixed_out + 20000) / 3;
  }

  if (mixed_out > 30000) mixed_out = 30000;
  if (mixed_out < -30000) mixed_out = -30000;

  int16_t final_out = (int16_t)mixed_out;
  
  return StereoOutput::fromNBit(19, (final_out * meap.volume_val) >> 12, (final_out * meap.volume_val) >> 12);
}

void updateTouch(int number, bool pressed) {
  if (pressed) {
    music_playing = true;
    current_pad = number;
    global_beat = 0; 
    
    // Send to Max
    Serial.print("SCENE "); 
    Serial.println(number);

    switch (number) {
      case 0: // Major
        current_scale = scale_major;
        current_prog = prog_major;
        style_base_root = 60; 
        for (int i=0; i<POLYPHONY; i++) osc_bank[i].setTable(sin8192_int16_DATA);
        Serial.println("Mode: Major");
        break;

      case 1: // Minor
        current_scale = scale_minor;
        current_prog = prog_minor;
        style_base_root = 57; 
        for (int i=0; i<POLYPHONY; i++) osc_bank[i].setTable(tri8192_int16_DATA);
        Serial.println("Mode: Sad Minor");
        break;

      case 2: // Cyber
        current_scale = scale_cyber;
        current_prog = prog_cyber;
        style_base_root = 50; 
        for (int i=0; i<POLYPHONY; i++) osc_bank[i].setTable(saw8192_int16_DATA);
        Serial.println("Mode: Cyber");
        break;

      case 3: // Tension
        current_scale = scale_tension;
        current_prog = prog_tension;
        style_base_root = 49; 
        for (int i=0; i<POLYPHONY; i++) osc_bank[i].setTable(sq8192_int16_DATA);
        Serial.println("Mode: Tension");
        break;
    }
  } else {
    if (number == current_pad) {
      music_playing = false;
      Serial.println("Music Stop");
    }
  }
}

void updateDip(int number, bool up)
{
  
}