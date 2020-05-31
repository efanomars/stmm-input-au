stmm-input-au
=============

Sound playback devices based on a subset of OpenAL.

For more info visit http://www.efanomars.com/libraries/stmm-input-au

This source package contains:

- libstmm-input-au:
    library that defines a sound playback capability based on stmm-input.
    Listeners receive sound finished events from devices implementing the capability.

- libstmm-input-openal:
    this library provides an open-al device manager that handles OpenAL playback
    devices such as plugged in ear phones, etc.
    The exposed devices implement the interfaces defined in the libstmm-input-au
    library. The device manager can also be installed as a stmm-input plug-in.


Read the INSTALL file for installation instructions.

An example can be found in the libstmm-input-openal subfolder.


Warning
-------
The APIs of the library isn't stable yet.
