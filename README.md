# Maelstrom
Maelstrom Controller firmware. There are no keys, so if you want to use the DFU you will have to generate keys
This uses the Nordic Secured Bootloader

The Beta units of the Maelstrom fan controller are made up of several elements all prepackaged in an enclosure. You may need to c


# Code
Edit the make fiiles to redirect to the SDK. This builds with nrf5 SDK 15.3.0 but may build with newer.
Known bug with nrfconnect programmer that seems to prevent 15.3.0 dfu/bootloader from programing correctly. Use commandline / messy fixes that you need to do all the time to get nordic stuff working. Manual full erase, write softdevice, write bootloader (without sector erase! or it breaks bootloader), then generate the DFU.

This does not incorporate the buttonless DFU. If I do then you must do firmware via BLE (as far as I know.)

#NRF52 SDK

Compatible with
S332 softdevice
S213 softdevice (ANT+ only) with limitations (no Over the air firmware update, only manual programing with a ARM Cortex Debugger)