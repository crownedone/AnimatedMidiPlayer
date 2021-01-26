# PipeDreamMidiPlayer
Animusic (https://www.youtube.com/watch?v=HR8Oz8Pp8hI) inspired OpenGL Project to play animation for **any** midi input file using CXXMidi for playback.

## Build instructions

### Windows
Unzip the vcpkg-export*.zip archive to the current location (you can also use your local vcpkg installation. Just change the -DCMAKE_TOOLCHAIN_FILE).
Execute make.bat. 
Your VS-Solutions will be built in /build and loaded automatically.
### Linux
Linux build is currently not available. There are some minor things that are not compatible with linux.

## Usage
./myproject andre-sonatine.mid

### Dependencies
- boost-filesystem
- STL2
- STB 
- GLEW
- glm
- git (fetching of cxxmidi)

## Todo's
1. Box around flying lights to stay in a Bounding Box
2. Complete 3D model's 
3. Water on the ground?