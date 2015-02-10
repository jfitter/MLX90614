/*********************************************************************************************/
/**
 *  \brief      Melexis MCX90614 Family Device Driver Library - CPP Source file
 *  \details    Based on the Melexis MLX90614 Family Data Sheet 3901090614 Rev 004 09jun2008.
 *  \li         The current implementation does not manage PWM (only digital data by I2C).
 *  \li         Sleep mode is not implemented yet.
 *
 *  \note       THIS IS ONLY A PARTIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING
 *              ACTIVE DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP 
 *              THIS IN MIND IF YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.
 *
 *  \file       MLX90614.CPP
 *  \author     J. F. Fitter <jfitter@eagleairaust.com.au>
 *  \version    1.0
 *  \date       2014-2015
 *  \copyright  Copyright (c) 2015 John Fitter.  All right reserved.
 *
 *  \par License
 *             GNU Public License. Permission is hereby granted, free of charge, to any
 *             person obtaining a copy of this software and associated documentation files
 *             (the "Software"), to deal in the Software without restriction, including
 *             without limitation the rights to use, copy, modify, merge, publish, distribute,
 *             sublicense, and/or sell copies of the Software, and to permit persons to whom
 *             the Software is furnished to do so, subject to the following conditions:
 *  \par
 *              The above copyright notice and this permission notice shall be included in
 *              all copies or substantial portions of the Software.
 *  \par
 *              THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *              IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *              FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *              AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *              LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *              OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *              THE SOFTWARE.
 *
 *********************************************************************************************/

#include "MLX90614.h"

/*********************************************************************************************/
/*  MLX90614 Device class functions.                                                         */
/*********************************************************************************************/

/**
 *  \brief               MLX90614 Device class constructor.
 *  \param [in] i2caddr  Device address (default: published value).
 */
MLX90614::MLX90614(uint8_t i2caddr) {
    rwError.Set_Class(this);
    rwError.Set_Get( &MLX90614::getRwError);
    _rwError = 0;

    pec.Set_Class(this);
    pec.Set_Get( &MLX90614::getPEC);
    _pec = 0;

    crc8.Set_Class(this);
    crc8.Set_Get( &MLX90614::getCRC8);
//  crc8.Set_Set( &MLX90614::setCRC8);  // template for setter
    _crc8 = 0;

    _addr = i2caddr;
}

/**
 *  \brief  Initialize the device and the i2c interface.
 */
boolean MLX90614::begin(void) {
    Wire.begin();
    return true;
}

/**
 *  \brief             Return a temperature from the specified source in specified units.
 *  \li                Teperature is stored in ram as a 16 bit absolute value to a resolution of 0.02K
 *  \li                Linearized sensor die temperature is available as Ta (ambient).
 *  \li                One or two object temperatures are linearized to the range -38.2C...125C
 *  \param [in] tsrc   Internal temperature source to read.
 *  \param [in] tunit  Temperature units to convert raw data to.
 *  \return            Temperature.
 */
double MLX90614::readTemp(tempSrc_t tsrc, tempUnit_t tunit) {
    double temp;

    _rwError = 0;
    switch(tsrc) {
        case    MLX90614_SRCO1 : temp = read16(MLX90614_TOBJ1); break;
        case    MLX90614_SRCO2 : temp = read16(MLX90614_TOBJ2); break;
        default : temp = read16(MLX90614_TA);
    }
    temp *= 0.02;
    switch(tunit) {
        case    MLX90614_TC : return convKtoC(temp);
        case    MLX90614_TF : return convKtoC(convCtoF(temp));
        default : return temp;
    }
}

/**
 *  \brief             Set the emissivity of the object.
 *  \details           The emissivity is stored as a 16 bit integer defined by the following:
 *  \n<tt>             emissivity = dec2hex[round(65535 x emiss)]</tt>
 *  \param [in] emiss  Physical emissivity value in range 0.1 ...1.0, default 1.0
 */
void MLX90614::setEmissivity(float emiss = 1.0) {

    _rwError = 0;
    uint16_t e = int(emiss * 65535 + 0.5);
    if((emiss > 1.0) || (e < 6553)) _rwError |= MLX90614_INVALIDATA;
    else writeEEProm(MLX90614_EMISS, e);
}

/**
 *  \brief            Set the coefficients of the IIR digital filter.
 *  \details          The IIR digital filter coefficients are set by the LS 3 bits of ConfigRegister1
 *  \n                The value of the coefficients is set as follows: 
 *  \n <tt> \verbatim
 csb = 0   a1 = 0.5    a2 = 0.5
       1        0.25        0.75
       2        0.167       0.833
       3        0.125       0.875
       4        1           0 (IIR bypassed)
       5        0.8         0.2
       6        0.67        0.33
       7        0.57        0.43 \endverbatim </tt>
 *  \param [in] csb   See page 12 of datasheet. Range 0...7, default = 4 (IIR bypassed)
 */
void MLX90614::setIIRcoeff(uint8_t csb = 4) {

    _rwError = 0;

    // ensure legal range by clearing all but the LS 3 bits
    csb &= 7;

    // get the current value of ConfigRegister1
    uint16_t reg = readEEProm(MLX90614_CONFIG);

    // clear bits 2:0, mask in the new value, then write it back
    if(!_rwError) {
        reg &= 0xfff8;
        reg |= (uint16_t)csb;
        writeEEProm(MLX90614_CONFIG, reg);
    }
}

/**
 *  \brief            Set the coefficients of the FIR digital filter.
 *  \details          The FIR digital filter coefficient N is bits 10:8 of ConfigRegister1
 *  \n                The value of N is set as follows:  <tt>N = 2 ^ (csb + 3)</tt>
    \n                The manufacturer does not recommend <tt>N < 128</tt>
 *  \param [in] csb   See page 12 of datasheet. Range 0...7, default = 7 (N = 1024)
 */
void MLX90614::setFIRcoeff(uint8_t csb = 7) {

    _rwError = 0;

    // ensure legal range by clearing all but the LS 3 bits
    csb &= 7;

    // get the current value of ConfigRegister1
    uint16_t reg = readEEProm(MLX90614_CONFIG);

    // clear bits 10:8, mask in the new value, then write it back
    if(!_rwError) {
        reg &= 0xf8ff;
        reg |= (uint16_t)csb << 8;
        writeEEProm(MLX90614_CONFIG, reg);
    }
}

/**
 *  \brief            Set device SMBus address.
 *  \li               Must be only device on the bus.
 *  \li               Must power cycle the device after changing address.
 *  \param [in] a     New device address. Range 1...127, default = 0x5a
 */
void MLX90614::setSMBusAddr(uint8_t addr = MLX90614_I2CDEFAULTADDR) {

    _rwError = 0;

    // it is assumed we do not know the existing slave address
    // so the broadcast address is used
    // first ensure the new address is in legal range (1..127)
    if(addr &= 0x7f) {
        _addr = 0;
        writeEEProm(MLX90614_ADDR, addr);
    } else _rwError |= MLX90614_INVALIDATA;
}

/**
 *  \brief            Return the device SMBus address.
 *  \li               Must be only device on the bus.
 *  \li               Sets the library to use the new found address.
 *  \return           Device address.
 */
uint8_t MLX90614::getSMBusAddr(void) {
    uint8_t tempAddr = _addr;

    _rwError = 0;

    // it is assumed we do not know the existing slave address
    // so the broadcast address is used
    _addr = 0;

    // reload program copy with existing slave address
    _addr = lowByte(readEEProm(MLX90614_ADDR));

    // on any R/W error restore program copy of slave address
    if(_rwError) _addr = tempAddr;
    return _addr;
}

/**
 *  \brief            Return a 16 bit value read from RAM or EEPROM.
 *  \param [in] cmd   Command to send (register to read from).
 *  \return           Value read from memory.
 */
uint16_t MLX90614::read16(uint8_t cmd) {
    uint16_t val;
    CRC8 crc(MLX90614_CRC8POLY);

    // send the slave address then the command and set any
    // error status bits returned by the write
    Wire.beginTransmission(_addr);
    Wire.write(cmd);
    _rwError |= (1 << Wire.endTransmission(false)) >> 1;

    // experimentally determined delay to prevent read errors
    // (manufacturer's data sheet has left something out)
    delayMicroseconds(MLX90614_XDLY);

    // resend slave address then get the 3 returned bytes
    Wire.requestFrom(_addr, (uint8_t)3);

    // data is returned as 2 bytes little endian
    val = Wire.read();
    val |= Wire.read() << 8;

    // read the PEC (CRC-8 of all bytes)
    _pec = Wire.read();

    // build our own CRC-8 of all received bytes
    crc.crc8(_addr << 1);
    crc.crc8(cmd);
    crc.crc8((_addr << 1) + 1);
    crc.crc8(lowByte(val));
    _crc8 = crc.crc8(highByte(val));

    // set error status bit if CRC mismatch
    if(_crc8 != _pec) _rwError |= MLX90614_RXCRC;

    return val;
}

/**
 *  \brief            Write a 16 bit value to memory.
 *  \param [in] cmd   Command to send (register to write to).
 *  \param [in] data  Value to write.
 */
void MLX90614::write16(uint8_t cmd, uint16_t data) {
    CRC8 crc(MLX90614_CRC8POLY);

    // build the CRC-8 of all bytes to be sent
    crc.crc8(_addr << 1);
    crc.crc8(cmd);
    crc.crc8(lowByte(data));
    _crc8 = crc.crc8(highByte(data));

    // send the slave address then the command
    Wire.beginTransmission(_addr);
    Wire.write(cmd);

    // write the data low byte first
    Wire.write(lowByte(data));
    Wire.write(highByte(data));

    // then write the crc and set the error status bits
    Wire.write(_pec = _crc8);
    _rwError |= (1 << Wire.endTransmission(false)) >> 1;
}

/**
 *  \brief            Return a 16 bit value read from EEPROM.
 *  \param [in] addr  Register address to read from.
 *  \return           Value read from EEPROM.
 */
uint16_t MLX90614::readEEProm(uint8_t addr) {return read16(addr | 0x20);}

/**
 *  \brief            Write a 16 bit value to EEPROM after first clearing the memory.
 *  \li               Erase and write time 5ms per manufacturer specification
 *  \li               Manufacturer does not specify max or min erase/write times
 *  \param [in] reg   Address to write to.
 *  \param [in] data  Value to write.
 */
void MLX90614::writeEEProm(uint8_t reg, uint16_t data) {
    uint16_t val;
    reg |= 0x20;

    // read current value, compare to the new value, and do nothing on a match
    // or if there are read errors set the error status flag only
    val = read16(reg);
    if((val != data) && !_rwError) {

        // on any R/W errors it is assumed the memory is corrupted
        // clear the memory and wait Terase (per manufacturer's documentation)
        write16(reg, 0);
        delay(5);

        // if no write errors then write the new value
        if(_rwError) _rwError |= MLX90614_EECORRUPT;
        else {

            // write the data and wait Twrite (per manufacturer's documentation)
            write16(reg, data); 
            delay(5);
            if(_rwError) _rwError |= MLX90614_EECORRUPT;
        }
    }
}

/**
 *  \brief            Convert temperature in degrees K to degrees C.
 *  \param [in] degK  Temperature in degrees Kelvin.
 *  \return           Temperature in degrees Centigrade.
 */
double MLX90614::convKtoC(double degK) {return degK - 273.15;}

/**
 *  \brief            Convert temperature in degrees C to degrees F.
 *  \param [in] degC  Temperature in degrees Centigrade.
 *  \return           Temperature in degrees Fahrenheit.
 */
double MLX90614::convCtoF(double degC) {return (degC * 9. / 5.) + 32.;}

/**
 *  \brief            Retrieve the chip ID bytes.
 *  \return           Chip ID as a 64 bit word.
 */
uint64_t MLX90614::readID(void) {
    uint64_t ID = 0;

    for(uint8_t i = 0; i < 4; i++) ID = (ID <<= 16) | readEEProm(MLX90614_ID1 + i);
    return ID;
}

