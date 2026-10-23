# Meap Car

Meap Car is an interactive sound and visual project built with a Meap board, Arduino code, and Max. A synthesized engine drives the musical tempo, while touch pads and knobs change the musical mode, note behavior, and visual scene.

The project connects engine synthesis, generative music, and video through a shared set of controls. Increasing the throttle changes the engine sound and speeds up the music; changing modes switches the musical rules and the video background.

## Demo

[Watch the Meap Car demonstration on YouTube](https://youtu.be/Y0rzvjF_9ew)

## Sound and interaction

### Engine synthesis

Two wavetable oscillators are multiplied to create a rough, engine-like texture. One follows the engine-speed value; the other runs at 2.5 times that frequency. Smoothing gives speed changes some inertia, and filtered noise adds a wind-like layer as the engine accelerates.

### Generative music

The musical layer combines chords with a generated melody. Its timing follows engine speed. Four touch-pad modes select different scales, progressions, waveforms, and note-generation rules:

| Pad | Mode | Waveform | Musical behavior |
| --- | --- | --- | --- |
| 0 | Major | Sine | Repeating arpeggios with passing notes and octave changes as complexity increases |
| 1 | Minor | Triangle | A lyrical pattern with additional notes and tension at higher complexity |
| 2 | Cyber | Sawtooth | Lower-register patterns, syncopation, and octave jumps |
| 3 | Tension | Square | Probabilistic note triggers and a wider register at higher complexity |

The first potentiometer changes note release. The second changes complexity, affecting note density, rhythmic variation, and pitch choices according to the selected mode. Touching a pad starts its musical mode; releasing the active pad stops the music.

### Visuals in Max

The Max patch plays four looping backgrounds and fades between them when the musical mode changes. The throttle control changes video playback speed, while the complexity value controls a strobe effect.

The firmware sends `SCENE` and `CHAOS` messages to Max over serial at 115200 baud. MIDI control messages handle engine speed and ignition.

## Controls

| Control | Function |
| --- | --- |
| MIDI CC 1 | Engine speed and musical tempo |
| MIDI CC 2 | Ignition on/off |
| Touch pads 0–3 | Major, Minor, Cyber, and Tension modes |
| Potentiometer 0 | Note release time |
| Potentiometer 1 | Musical complexity and visual strobe |

## Files

```text
meap-car/
├── firmware/
│   └── Meapcar/
│       └── Meapcar.ino
├── max/
│   ├── Meapcar.maxpat
│   ├── bg0.mp4
│   ├── bg1.mp4
│   ├── bg2.mp4
│   └── bg3.mp4
├── docs/
│   └── Meap Car Design Overview.docx
└── README.md
```

The Arduino sketch contains the synthesis and music-generation logic. The Max patch handles video and communication. The design overview provides more detail about the sound layers and mappings.

## Requirements

- A Meap board with its potentiometers and touch pads.
- An Arduino development environment configured for the board, with the Meap library (`Meap.h`) and its dependencies installed.
- Max with Jitter support. The supplied patch was saved in **Max 8.5.8**.
- Serial and MIDI connections configured for the board.
- Headphones or speakers connected to the board's audio output.

## Running the project

1. Download or clone the complete repository so the Max patch and all four video files are included.
2. Open `firmware/Meapcar/Meapcar.ino` in the Arduino environment. Select the appropriate board and port, then compile and upload the sketch.
3. Close the Arduino Serial Monitor before opening the serial connection in Max.
4. Open `max/Meapcar.maxpat`. Keep `bg0.mp4` through `bg3.mp4` alongside it. If the videos are not found, add the `max/` directory to Max's file search path or load them through the patch's `read` messages.
5. Configure the serial object for the connected board. The saved patch uses `serial c 115200`; the port identifier may need to change on another computer. Use the object's port-listing function to identify the correct port.
6. Configure the MIDI output used by the patch to reach the board's MIDI input. Confirm CC 1 controls speed and CC 2 controls ignition.
7. Start the patch's visual and serial polling controls. Turn on the ignition and raise the throttle to hear the engine. Touch a mode pad to start the musical layer, then use the potentiometers to change release and complexity.

The firmware runs on the Meap board; opening the Max patch alone does not run the synthesis. The serial port and MIDI routing are specific to the connected setup and should be checked before use.

## Author

Jazlyn Pei
