## Arduino Library for the MLX90614 Temperature Sensor

This library was written to enable remote sensing of the temperature of the rotors of outrunner style brushless DC motors used in remotely piloted aircraft, for the purpose of real time data logging and air to ground telemetry.

These sensors use the SMB bus protocol to communicate. This is similar, though not identical, to the I2C bus. There is enough similarity to enable the Arduino standard Wire library to communicate with the device, however not all features can be implemented, for example it is not possible to read the flags register with standard Wire functions. 2 pins are required to interface the device to an Arduino - the SDA and SCL lines.

### Installing

Download the distribution package and decompress it.  
Rename the uncompressed folder ***/mlx90614***.  
Check that the ***/mlx90614*** folder contains the following files;

> MLX90614.cpp  
> MLX90614.h  
> MLX90614.chm  
> MLX90614.pdf  
> Crc8.cpp  
> Crc8.h  
> property.h  
> doxyfile  

Place the ***/mlx90614*** library folder into your ***arduinosketchfolder/libraries/*** folder.  
You may need to create the libraries subfolder if its your first library.
Restart the IDE.

### Documentation

*MLX90614.chm* and *MLX90614.pdf* contain the documentation for the classes.  
A Doxygen script is included to enable generation of documentation. You will need the graph tool, the dot tool, and the help compiler, in addition to editing the paths to these tools in the script to suit your environment. 

### Author

John Fitter B.E., Eagle Air Australia Pty. Ltd.  
This library was inspired by a library written by Adafruit Industries.

### License

This program is licensed under the terms of the GNU Lesser General Public License as published by the Free Software Foundation. See the GNU Lesser General Public License for more details at <http://www.gnu.org/copyleft/gpl.html>

