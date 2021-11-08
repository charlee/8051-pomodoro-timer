8051 Timer
===========

This is a DIY project that creates a "Pomodoro Timer" which helps you focus on your work.

The MCU used in this project is an AT89C52, with a 12MHz crystal oscillator. You can use any MCS51 compatible MCU.


Schematics
------------

![Schematic](schematics/8051-timer-schematic.png)


Development
--------------

This project is originally created under Windows 10.
To build the project, you need to install [SDCC](http://sdcc.sourceforge.net/).

Install make:

```
> chocolatey install make
```

```
> cd src
> make
```

This will generate a `timer-8051.hex` ready to be downloaded to AT89C52.