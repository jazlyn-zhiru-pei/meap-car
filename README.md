# Meap Car

An interactive engine-sound and generative-music system built with a Meap board and a Max patch. Changes in engine speed affect the music tempo and visual playback. Four touch-pad modes select different musical and visual scenes.

## Repository contents

- `firmware/Meapcar/Meapcar.ino`: embedded synthesis and music-generation code. The sketch is inside a matching Arduino folder.
- `max/Meapcar.maxpat`: editable Max patch for visual playback, MIDI controls, and serial communication.
- `max/bg0.mp4` through `max/bg3.mp4`: the four video backgrounds loaded by the patch. Keep these names and keep the files alongside the patch.
- `docs/Meap Car Design Overview.docx`: design overview and control mappings.

## Requirements

- The Meap hardware used for the project.
- Arduino development environment configured for that board, including the `Meap.h` library and its dependencies. Board and library versions should be documented from the original working setup before claiming reproducibility.
- Max with Jitter video playback support. The patch file records the original Max version in its `appversion` metadata.
- USB/serial connection and MIDI routing appropriate to the original hardware setup.

## Opening the project

1. Download the complete repository rather than only the patch file.
2. Open `firmware/Meapcar/Meapcar.ino` in the configured Arduino environment and select the correct board and port before uploading.
3. Open `max/Meapcar.maxpat` in Max. Ensure the four background videos can be found. If Max cannot locate them, add the `max/` folder to its file search path.
4. The patch currently uses `serial c 115200`. Select the connected board's actual serial port rather than assuming port `c` is correct on another computer.
5. Configure MIDI output for the board. The firmware uses CC 1 for engine speed and CC 2 for ignition. Check these against the actual routing when connecting the setup.

## Controls and behavior

The firmware combines engine-like wavetable synthesis and modulation with generative musical parts. Engine speed is smoothed and linked to musical timing. Touch pads select Major, Minor, Cyber, and Tension modes; the two potentiometers control note release and musical complexity.

The Max patch receives `SCENE` and `CHAOS` messages over serial at 115200 baud. It switches and blends background videos, adjusts playback rate through the control mapping, and applies a complexity-dependent visual effect.

## Demo

The original demonstration movie is kept outside this repository. Add the hosted video link here after upload; the same video can be embedded on the portfolio page.

## Status

This folder was prepared from the supplied project files. The patch JSON and media references have been checked. Firmware compilation, hardware behavior, and Max playback have not been tested in this environment.
