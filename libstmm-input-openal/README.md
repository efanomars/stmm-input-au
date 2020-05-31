stmm-input-openal                                                  {#mainpage}
=================

Implementation of the stmm-input library for OpenAl playback devices.

The device manager provided by this library handles OpenAL playback devices.
It tracks their creation and removal with the device management events 
defined in the stm-input-ev library. For each device it exposes the playback
capability defined in the stmm-input-au library and sends "sound finished"
events to listeners.
It optionally can be used as a plugin (of stmm-input-dl).

The device manager attaches itself to the Gtk event loop.


Warning
-------
The API of this library isn't stable yet.
