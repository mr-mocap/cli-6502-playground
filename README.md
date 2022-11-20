# cli-6502-playground

A project intended for creating a 6502 emulator combined with editing capabilities for playing around with a basic 6502 system.  Uses the **terminal** as the output device.

The FTXUI terminal effects library is used for providing the UI and structure.

## Features

* A single-page window into the 64KB address space, complete with simple editing capabilities
* Disassembly window
* Register View window, also with the ability to edit the values
* Single-step one clock cycle
* Run at full speed
* Reset the processor (perform the 6502 reset sequence)
* Runs in a terminal only!

## Why do this?

Why else, but to learn!  I also was struck by the uniqueness of the FTXUI library, so I decided to try and see what I could do with it.

There seems to be a bit of a focus on the GUI nowadays, so I wanted something more minimal, and hopefully this will be a bit more portable
across platforms because of that.
