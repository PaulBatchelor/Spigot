# Spigot

Spigot is a Composition utility for Sporth. 

It includes:

- A built-in UDP listener for live coding
- A minimalist music tracker
- A minimalist drum sequencer
- An experimental interface based on brainfuck

## Compilation

First, make sure the following libraries are installed:

- [Soundpipe](https://www.github.com/paulbatchelor/soundpipe.git): make sure 
to use the "dev" branch
- [Sporth](https://www.github.com/paulbatchelor/sporth.git): make sure to 
use the "dev" branch
- [Runt](https://www.github.com/paulbatchelor/runt.git)

After these libraries are installed, Spigot is ready to be compiled.

On Linux, Spigot can be compiled for JACK using:

    make linux

On OSX, Spigot can be compiled using:

    make osx

## Live-Coding Setup

Live coding with Sporth can be done using the Vim plugin
[Vorth](https://www.github.com/paulbatchelor/vorth.git). 
