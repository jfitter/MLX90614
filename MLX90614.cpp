/***********************************************************************************************//**
 *  \brief      Melexis MLX90614 Family Device Driver Library - CPP Source file
 *  \par
 *  \par        Details
 *              Based on the Melexis MLX90614 Family Data Sheet 3901090614 Rev 004 09jun2008.
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
 *  \date       2014-2017
 *  \copyright  Copyright (c) 2017 John Fitter.  All right reserved.
 *
 *  \par        License
 *              This program is free software; you can redistribute it and/or modify it under
 *              the terms of the GNU Lesser General Public License as published by the Free
 *              Software Foundation; either version 2.1 of the License, or (at your option)
 *              any later version.
 *  \par
 *              This Program is distributed in the hope that it will be useful, but WITHOUT ANY
 *              WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *              PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details
 *              at http://www.gnu.org/copyleft/gpl.html
 *  \par
 *              You should have received a copy of the GNU Lesser General Public License along
 *              with this library; if not, write to the Free Software Foundation, Inc.,
 *              51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *//***********************************************************************************************/

#include "MLX90614.h"

/**************************************************************************************************/
/*  MLX90614 Device class functions.                                                              */
/**************************************************************************************************/

/**
 *  \brief               MLX90614 Device class constructor.
 *  \param [in] i2caddr  Device address (default: published value).
 */
MLX90614::MLX90614(uint8_t i2caddr) {

    busAddr.Set_Class(this);
    busAddr.Set_Get(&MLX90614::getAddr);
    busAddr.Set_Set(&MLX90614::setAddr);

    rwError.Set_Class(this);
    rwError.Set_Get(&MLX90614::getRwError);

    pec.Set_Class(this);
    pec.Set_Get(&MLX90614::getPEC);

    crc8.Set_Class(this);
    crc8.Set_Get(&MLX90614::getCRC8);

    _addr = i2caddr;
    _ready = false;
}

/**
 *  \brief  Initialize the device and the i2c interface.
 */
boolean MLX90614::begin(void) {

    _rwError = _pec = _crc8 = 0;
    return _ready = true;
}

/**
 *  \brief             Return a temperature from the specified source in specified units.
 *  \remarks
 *  \li                Temperature is stored in ram as a 16 bit absolute value to a resolution of 0.02K
 *  \li                Linearized sensor die temperature is available as Ta (ambient).
 *  \li                One or two object temperatures are linearized to the range -38.2C...125C
 *  \param [in] tsrc   Internal temperature source to read, default #1.
 *  \param [in] tunit  Temperature units to convert raw data to, default deg Celsius.
 *  \return            Temperature.
 */
double MLX90614::readTemp(tempSrc_t tsrc, tempUnit_t tunit) {
    double temp;

    _rwError = 0;
    switch(tsrc) {
        case MLX90614_SRC01 : temp = read16(MLX90614_TOBJ1); break;
        case MLX90614_SRC02 : temp = read16(MLX90614_TOBJ2); break;
        default : temp = read16(MLX90614_TA);
    }
    temp *= 0.02;
    switch(tunit) {
        case MLX90614_TC : return convKtoC(temp);
        case MLX90614_TF : return convKtoC(convCtoF(temp));
    }
    return temp;
}

/**
 *  \brief             Set the emissivity of the object.
 *  \remarks           The emissivity is stored as a 16 bit integer defined by the following:
 *  \n<tt>             emissivity = dec2hex[round(65535 x emiss)]</tt>
 *  \param [in] emiss  Physical emissivity value in range 0.1 ...1.0, default 1.0
 */
void MLX90614::setEmissivity(float emiss) {

    _rwError = 0;
    uint16_t e = int(emiss * 65535. + 0.5);
    if((emiss > 1.0) || (e < 6553)) _rwError |= MLX90614_INVALIDATA;
    else writeEEProm(MLX90614_EMISS, e);
}
/**
 *  \brief             Get the emissivity of the object.
 *  \remarks           The emissivity is stored as a 16 bit integer defined by the following:
 *  \n<tt>             emissivity = dec2hex[round(65535 x emiss)]</tt>
 *  \return            Physical emissivity value in range 0.1 ...1.0
 */
float MLX90614::getEmissivity(void) {

    _rwError = 0;
    uint16_t emiss = readEEProm(MLX90614_EMISS);
    if(_rwError) return (float)1.0;
    return (float)emiss / 65535.0;
}

/**
 *  \brief            Set the coefficients of the IIR digital filter.
 *  \remarks          The IIR digital filter coefficients are set by the LS 3 bits of ConfigRegister1
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
void MLX90614::setIIRcoeff(uint8_t csb) {

    _rwError = 0;

    // Ensure legal range by clearing all but the LS 3 bits.
    csb &= 7;

    // Get the current value of ConfigRegister1
    uint16_t reg = readEEProm(MLX90614_CONFIG);

    // Clear bits 2:0, mask in the new value, then write it back.
    if(!_rwError) {
        reg &= 0xfff8;
        reg |= (uint16_t)csb;
        writeEEProm(MLX90614_CONFIG, reg);
    }
}

/**
 *  \brief            Get the coefficients of the IIR digital filter.
 *  \remarks          The IIR digital filter coefficients are set by the LS 3 bits of ConfigRegister1
 *  \return           Filter coefficient table index. Range 0...7
 */
uint8_t MLX90614::getIIRcoeff(void) {

    _rwError = 0;

    // Get the current value of ConfigRegister1 bits 2:0
    uint8_t iir = readEEProm(MLX90614_CONFIG) & 7;

    if(_rwError) return 4;
    return iir;
}

/**
 *  \brief            Set the coefficients of the FIR digital filter.
 *  \remarks          The FIR digital filter coefficient N is bits 10:8 of ConfigRegister1
 *  \n                The value of N is set as follows:  <tt>N = 2 ^ (csb + 3)</tt>
    \n                The manufacturer does not recommend <tt>N < 128</tt>
 *  \param [in] csb   See page 12 of datasheet. Range 0...7, default = 7 (N = 1024)
 */
void MLX90614::setFIRcoeff(uint8_t csb) {

    _rwError = 0;

    // Ensure legal range by clearing all but the LS 3 bits.
    csb &= 7;

    // Get the current value of ConfigRegister1
    uint16_t reg = readEEProm(MLX90614_CONFIG);

    // Clear bits 10:8, mask in the new value, then write it back.
    if(!_rwError) {
        reg &= 0xf8ff;
        reg |= (uint16_t)csb << 8;
        writeEEProm(MLX90614_CONFIG, reg);
    }
}

/**
 *  \brief            Get the coefficients of the FIR digital filter.
 *  \remarks          The FIR digital filter coefficient N is bits 10:8 of ConfigRegister1
 *  \n                The value of N is set as follows:  <tt>N = 2 ^ (csb + 3)</tt>
    \n                The manufacturer does not recommend <tt>N < 128</tt>
 */
uint8_t MLX90614::getFIRcoeff(void) {

    _rwError = 0;

    // Get the current value of ConfigRegister1 bits 10:8
    uint8_t fir = (readEEProm(MLX90614_CONFIG) >> 8) & 7;

    if(_rwError) return 7;
    return fir;
}

/**
 *  \brief            Set device SMBus address.
 *  \remarks
 *  \li               Must be only device on the bus.
 *  \li               Must power cycle the device after changing address.
 *  \param [in] addr  New device address. Range 1...127
 */
void MLX90614::setAddr(uint8_t addr) {

    _rwError = 0;

    // It is assumed we do not know the existing slave address so the broadcast address is used.
    // First ensure the new address is in the legal range (1..127)
    if(addr &= 0x7f) {
        _addr = MLX90614_BROADCASTADDR;
        writeEEProm(MLX90614_ADDR, addr);
        
        // There will always be a r/w error using the broadcast address so we cannot respond
        // to r/w errors. We must just assume this worked.
        _addr = addr;
        
    } else _rwError |= MLX90614_INVALIDATA;
}

/**
 *  \brief            Return the device SMBus address.
 *  \remarks
 *  \li               Must be only device on the bus.
 *  \li               Sets the library to use the new found address.
 *  \return           Device address.
 */
uint8_t MLX90614::getAddr(void) {
    uint8_t tempAddr = _addr;

    _rwError = 0;

    // It is assumed we do not know the existing slave address so the broadcast address is used.
    // This will throw a r/w error so errors will be ignored.
    _addr = MLX90614_BROADCASTADDR;

    // Reload program copy with the existing slave address.
    _addr = lowByte(readEEProm(MLX90614_ADDR));

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

    // Send the slave address then the command and set any error status bits returned by the write.
    Wire.beginTransmission(_addr);
    Wire.write(cmd);
    _rwError |= (1 << Wire.endTransmission(false)) >> 1;

    // Experimentally determined delay to prevent read errors (manufacturer's data sheet has 
    // left something out).
    delayMicroseconds(MLX90614_XDLY);

    // Resend slave address then get the 3 returned bytes.
    Wire.requestFrom(_addr, (uint8_t)3);

    // Data is returned as 2 bytes little endian.
    val = Wire.read();
    val |= Wire.read() << 8;

    // Rread the PEC (CRC-8 of all bytes).
    _pec = Wire.read();

    // Clear r/w errors if using broadcast address.
    if(_addr == MLX90614_BROADCASTADDR) _rwError &= MLX90614_NORWERROR;
    
    // Build our own CRC-8 of all received bytes.
    crc.crc8(_addr << 1);
    crc.crc8(cmd);
    crc.crc8((_addr << 1) + 1);
    crc.crc8(lowByte(val));
    _crc8 = crc.crc8(highByte(val));

    // Set error status bit if CRC mismatch.
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

    // Build the CRC-8 of all bytes to be sent.
    crc.crc8(_addr << 1);
    crc.crc8(cmd);
    crc.crc8(lowByte(data));
    _crc8 = crc.crc8(highByte(data));

    // Send the slave address then the command.
    Wire.beginTransmission(_addr);
    Wire.write(cmd);

    // Write the data low byte first.
    Wire.write(lowByte(data));
    Wire.write(highByte(data));

    // Then write the crc and set the r/w error status bits.
    Wire.write(_pec = _crc8);
    _rwError |= (1 << Wire.endTransmission(true)) >> 1;

    // Clear r/w errors if using broadcast address.
    if(_addr == MLX90614_BROADCASTADDR) _rwError &= MLX90614_NORWERROR;
}

/**
 *  \brief            Return a 16 bit value read from EEPROM.
 *  \param [in] addr  Register address to read from.
 *  \return           Value read from EEPROM.
 */
uint16_t MLX90614::readEEProm(uint8_t addr) {return read16(addr | 0x20);}

/**
 *  \brief            Write a 16 bit value to EEPROM after first clearing the memory.
 *  \remarks
 *  \li               Erase and write time 5ms per manufacturer specification
 *  \li               Manufacturer does not specify max or min erase/write times
 *  \param [in] reg   Address to write to.
 *  \param [in] data  Value to write.
 */
void MLX90614::writeEEProm(uint8_t reg, uint16_t data) {
    uint16_t val;
    reg |= 0x20;

    // Read current value, compare to the new value, and do nothing on a match or if there are
    // read errors set the error status flag only.
    val = read16(reg);
    if((val != data) && !_rwError) {

        // On any R/W errors it is assumed the memory is corrupted.
        // Clear the memory and wait Terase (per manufacturer's documentation).
        write16(reg, 0);
        delay(5);
        if(_rwError) _rwError |= MLX90614_EECORRUPT;

        // Write the data and wait Twrite (per manufacturer's documentation) 
        // and set the r/w error status bits.
        write16(reg, data); 
        delay(5);
        if(_rwError) _rwError |= MLX90614_EECORRUPT;
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
double MLX90614::convCtoF(double degC) {return (degC * 1.8) + 32.0;}

/**
 *  \brief            Retrieve the chip ID bytes.
 *  \return           Chip ID as a 64 bit word.
 */
uint64_t MLX90614::readID(void) {
    uint64_t ID = 0;

    // If we are lucky the compiler will optimise this.
    for(uint8_t i = 0; i < 4; i++) ID = (ID <<= 16) | readEEProm(MLX90614_ID1 + i);
    return ID;
}

