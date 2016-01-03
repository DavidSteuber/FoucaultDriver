# FoucaultDriver
C code and other things necessary to make a DIY Foucault Pendulum

The current state of this project is not useable. I will remove this bit when it is. I will also try to provide an electrical schematic although I have never done one.

If you don't know what a Foucault Pendulum is, visit this wikipedia page:

  https://en.wikipedia.org/wiki/Foucault_pendulum
  
The C code in this project is written in the Arduino IDE for an UNO compatible board. The pendulum itself currently resides in my basement. Air drag stops it from swinging inside of an hour or so. The purpose of this code is to drive a circuit that will control an electromagnet underneath the bob of the pendulum to give it a small "kick" periodically to keep it going.

The timeing for the magnet being turned on and off is based on the readings from a photoresistor. The length of the pendulum or local gravity is not taken into account. All timing data comes from the photoresistor. The board will use that timing data and sensor information to control the magnet. For this to work, an overhead light will be needed so that the pendulum bob can cast a shadow over the photoresistor.

Alignment is extremely important. A function will be provided to set the magnet pulsing on and off. If the pendulum starts to move, the magnet will need to be appropriately realigned.
