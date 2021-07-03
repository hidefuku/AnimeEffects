# AnimeEffects
A 2D animation tool which doesn't require a carefully thought-out plan.  
It provides various animation functions based on deformation of polygon mesh.<br><br>
Official Website:<br>
http://animeeffects.org/en/


Unofficial AnimeEffects boards (but run by developers still has interest in AE.)

Discord: <a href='https://discord.gg/sKp8Srm'>AnimeEffects Community Server</a> (courtesy of @Jose-Moreno)<br>
IRC: <a href='https://webchat.freenode.net/?channels=#animeeffects-dev'>#animeeffects-dev on freenode</a> (courtesy of @Larpon)

Note: For the present, there may be incompatible changes without notice in advance.

## Development Environment
* Windows/Linux/Mac
* Qt5.7 or later
* MSVC2015/MinGW/GCC/Clang (32-bit or 64-bit)

## Runtime Requirements
* OpenGL4.0 CoreProfile or later
  * On linux, you can check whether your graphics card supports OpenGL4.0 CoreProfile or not, run `glxinfo | grep "OpenGL core profile version"` on your terminal
* FFmpeg (Please install ffmpeg on your own for video exporting, you can also place a ffmpeg executable in /tools.)

## Linux

### Installing Dependencies
#### Debian / Ubuntu
* first update and install dependencies:
```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git gcc libglib2.0-0 qt5-default make
```
#### Arch / Manjaro
* first update and install dependencies:  
```
sudo pacman -Syu
sudo pacman -S git gcc glib2 qt5 make
```

### Clone / Building

* clone AnimeEffects git repo and go to the "src" folder:  
```
git clone https://github.com/herace/AnimeEffects  
cd AnimeEffects/src
qmake AnimeEffects.pro
make
```

* When building is done, run AnimeEffects:
```
./AnimeEffects  
```
If you have any issues feel free to ask questions on our <a href='https://discord.gg/sKp8Srm'>Discord</a>!
