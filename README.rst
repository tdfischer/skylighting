Noisebridge's skylights are boring and dull.

Lets make them shiny shiny!

The Idea
========

In all of Noisebridge's skylights, a set of RGB LED strips at the base.
Each controlled by a standalone microprocessor connected via ZigBee,
with the network as a whole controlled by a RaspberryPI base station.

The system should be as `Anti-Fragile <Anti-Fragile>`__ as possible:

-  If the power goes out, it should come back on and look neat
-  Each skylight can act independently of hardware failure of any other
   skylight
-  Multiple base station designs can be used to control simultaneously,
   as long as can speak a protocol over ZigBee
-  Wireless by default, wired if available
-  Use of DNS-SD so you don't have to scan the network to find a damn
   basestation
-  Replacing parts is really cheap and easy to do
-  Solar powered, so as to not rely on The Grid
-  Some sort of "Sleep Mode" that is launched after a period of
   inactivity or missing heartbeat

Some really neat uses:

-  Clicking a spot on a map on `Infobanana <Infobanana>`__ turns the
   overhead lighting into a magical directional indicator
-  When someone buzzes the door, perhaps a certain pattern could be
   shown
-  A UI that lets one configure each cell's RGB for an event
-  Automatically turning on/off based on a motion sensor grid in the
   space

The Software
============

-  **`Skyglighting on
   Github <https://github.com/tdfischer/skylighting>`__ - Main issue
   tracker**
-  `Graviton <http://git.phrobo.net/cgit/graviton.git/>`__ -
   Internet+RPI network control, interactivity, and discoverability
-  `gpiotty <https://github.com/phrobo/gpiotty>`__ - RPI+ZigBee
   connectivity

The Hardware
============

.. figure:: Skylighting_-_First_Blinkings.jpeg
   :alt: Skylighting_-_First_Blinkings.jpeg

   Skylighting\_-\_First\_Blinkings.jpeg
Currently in the prototype stage, but here's what is allocated:

-  Two XBee radios w/ USB serial boards
-  Two RaspberryPIs with wifi dongles
-  Three LED strips
-  Two solar panels
-  One battery

The Plan
========

This is being developed by User:Tdfischer using the following
milestones. Help is welcome and encouraged :)

#. Blink some LED lights via Arduino - First prototype
#. XBee to XBee communications between two linux systems over USB -
   Establish control environment
#. XBee to XBee communications over RaspberryPi GPIO pins - Development
   of gpiotty
#. XBee to Arduino communications - Wireless control prototype
#. Skylight power supply - Hardware design
#. Realtime display update protocol - Development of Graviton/implement
   pixelpusher
#. Control interfaces - Providing an API for super neat interactivity

Parts Needed
============

-  Arduino/ZigBee shields
-  Arduinos
-  The full length of LPD8806 LED Strips for all skylights - 45 meters!

   -  `Aliexpress <http://www.aliexpress.com/item/LPD8806-5050smd-dream-color-LED-strip-36LEDs-m-12Pixels-m-no-waterproof-free-shipping/1956523244.html>`__

-  RPI/ZigBee shield
-  Microcontroller selection for skylight units
-  ZigBee chips
-  Battery charger/regulator circuit

