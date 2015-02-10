This is an Arduino library for the MLX90614 temperature sensor

This library was written to enable remote sensing of the
temperature of the rotors of outrunner style brushless DC motors
used in remotely piloted aircraft, for the purpose of real time
data logging and air to ground telemetry.

These sensors use the SMB bus protocol to communicate. This is
similar though not identical to the I2C bus. There is enough
similarity to enable the Arduino standard Wire library to
communicate with the device, however not all features can be
implemented, for example it is not possible to read the flags
register with standard Wire functions. 2 pins are required to
interface the device to an Arduino - the SDA and SCL lines.

Written by John Fitter, Eagle Air Australia p/l. and inspired
by a library written by Adafruit Industries.

BSD license, all text above must be included in any redistribution

Download the distribution package and decompress it.
Rename the uncompressed folder MLX90614. Check that the MLX90614
folder contains the following files;
MLX90614.cpp
MLX90614.h
MLX90614.chm
Crc8.cpp
Crc8.h
property.h

Place the MLX90614 library folder your arduinosketchfolder/libraries/
folder. You may need to create the libraries subfolder if its your
first library.
Restart the IDE.

MLX90614.chm contains the documentation for the classes.
