# pector: Percussive sound detector

pector allows to detect percussive onsets in an audio stream. pector works on traditional computers, in the browser and on embedded devices like the ESP32. It does this by using portable ANSI C as its main programming language.

The main goal of pector is to react to double claps. With some glue code and pector, you can clap twice to put on a light, push code to github, control ventilation, start a dishwasher, ... 

![Clap for light](media/clap_for_light.gif)

Fig: an example of the intended use: clap for light

# pector: how?

pector expects a mono stream of audio samples in the form of 32 bit floating point. These are grouped into blocks and for each block the loudness is calculated and how 'percussive' the sound is. If the loudness is over a threshold **and** the 'precussiveness' is over another theshold then a percussive sound is detected.

There is some additional processing to prevent false positives. A 'double clap' is simply two percussive sounds within a small time window.

The input audio for pector can come from an audio file but it is more likely to come from a microphone to react to environmental sounds.

## pector on traditional computers

On traditional computers a c compiler and the make build tool are both needed. To make pector more easy to use [ffmpeg](http://ffmpeg.org) and Ruby should also be available. To install and use pector:

```
git clone 
cd pector
make
ffmpeg -hide_banner -loglevel panic  -f avfoundation -i 'none:default' -ac 1 -ar 16000 -f f32le -acodec pcm_f32le pipe:1 | bin/pector_c
```

The last command listens to incoming audio from the default microphone and pipes it to the pector binary which is called `bin/pector_c`. This `ffmpeg` command expects a macOS system and probably looks slightly different for your system. See here to e.g [capture audio with pulseaudio](https://trac.ffmpeg.org/wiki/Capture/PulseAudio).

Another use is of pector is to execute commands on double clap. The following code snippet is used to commit code to a repository by executing a set of commands after receiving a clap. The `bin/pector_c on doubleclap` part waits until a double clap is received. The  

```
alias microphone="ffmpeg -hide_banner -loglevel panic  -f avfoundation -i 'none:default' -ac 1 -ar 16000 -f f32le -acodec pcm_f32le pipe:1"
while true
do
  microphone | bin/pector_c on doubleclap
  git add .
  git commit -m "Code commit on double clap"
  git push origin
done
```

There is also a [`build.zig`](build.zig) file which allows pector to be compiled with zig. With zig installed:

```
git clone 
cd pector
zig build
file zig-out/bin/pector_c
```

The main advantage is to crosscompile pector for e.g. Windows or other systems. The following allows to compile an exe. Other systems can be targeted in a similar way.

```
zig build -Dtarget=x86_64-windows-gnu
file zig-out/bin/pector_c.exe
#zig-out/bin/pector_c.exe: PE32+ executable (console) x86-64, for MS Windows
```

## pector on embedded devices

pector has an [example Arduino patch](ESP32/clap_detector/clap_detector.ino). The example is build for an ESP32 microcontroller with a digital MEMS microphone. The MEMS microphone audio is read through an I2S bus, a standard way to communicate with audio devices.

There is a [helper script](ESP32/setup_arduino_patch.rb) to setup a new Arduino project. The helper script needs a folder name and a skeleton is created automatically. The skeleton can be opened in the Arduino IDE or similar tools.

```
cd ESP32
ruby setup_arduino_patch.rb my_new_clap_detector
```

## pector in the browser

pector works in the browser via WebAssembley. The `Makefile` contains build instructions for an emscripten version of pector. This can be called in a Web Audio API `Audio Worklet` 


## Contributing to pector

Please do use the issue tracker or send in merge requests or fork! Small disclaimer: only [commits triggered by double claps](#pector-on-traditional-computers) will be accepted. 

## pector credits

pector is written by Joren Six

pector uses [PFFFT](https://bitbucket.org/jpommier/pffft) by Julien Pommier. Some code in the precussion detection algorithm is based on TarsosDSP, by Joren Six, which is again based on aubio.



