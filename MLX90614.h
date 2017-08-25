#ifndef _MLX90614_H_
#define _MLX90614_H_

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

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif
#include <Wire.h>
#include "Property.h"
#include "Crc8.h"

/**************************************************************************************************/
/* Definitions                                                                                    */
/**************************************************************************************************/

#define MLX90614_I2CDEFAULTADDR 0x5A    /**< Device default slave address */
#define MLX90614_BROADCASTADDR  0       /**< Device broadcast slave address */
#define MLX90614_CRC8POLY       7       /**< CRC polynomial = X8+X2+X1+1 */
#define MLX90614_XDLY           25      /**< Experimentally determined delay to prevent read
                                             errors after calling Wire.endTransmission()
                                             <em>(possibly due to incompatibility between Wire
                                             library and SMBus protocol)</em>. */
/** RAM addresses. */
#define MLX90614_RAWIR1         0x04    /**< RAM reg - Raw temperature, source #1 */
#define MLX90614_RAWIR2         0x05    /**< RAM reg - Raw temperature, source #2 */
#define MLX90614_TA             0x06    /**< RAM reg - Linearized temperature, ambient */
#define MLX90614_TOBJ1          0x07    /**< RAM reg - Linearized temperature, source #1 */
#define MLX90614_TOBJ2          0x08    /**< RAM reg - Linearized temperature, source #2 */

/** EEPROM addresses. */
#define MLX90614_TOMAX          0x00    /**< EEPROM reg - Customer dependent object temperature range maximum */
#define MLX90614_TOMIN          0x01    /**< EEPROM reg - Customer dependent object temperature range minimum */
#define MLX90614_PWMCTRL        0x02    /**< EEPROM reg - Pulse width modulation output control register */
#define MLX90614_TARANGE        0x03    /**< EEPROM reg - Customer dependent ambient temperature range */
#define MLX90614_EMISS          0x04    /**< EEPROM reg - Object emissivity register */
#define MLX90614_CONFIG         0x05    /**< EEPROM reg - Configuration register */
#define MLX90614_ADDR           0x0E    /**< EEPROM reg - SMBus address */
#define MLX90614_ID1            0x1C    /**< EEPROM reg - ID numer (w1) */
#define MLX90614_ID2            0x1D    /**< EEPROM reg - ID numer (w2) */
#define MLX90614_ID3            0x1E    /**< EEPROM reg - ID numer (w3) */
#define MLX90614_ID4            0x1F    /**< EEPROM reg - ID numer (w4) */

#define MLX90614_RFLAGCMD       0xF0    /**< Read R/W Flags register command */

/** Read flags - bitmask. */
#define MLX90614_EEBUSY         0x80    /**< R/W flag bitmask - EEProm is busy (writing/erasing) */
#define MLX90614_EE_DEAD        0x20    /**< R/W flag bitmask - EEProm double error has occurred */
#define MLX90614_INIT           0x10    /**< R/W flag bitmask - POR initialization is still ongoing */

/** R/W Error flags - bitmask. */
#define MLX90614_NORWERROR      0       /**< R/W error bitmask - No Errors */
#define MLX90614_DATATOOLONG    1       /**< R/W error bitmask - Data is too long */
#define MLX90614_TXADDRNACK     2       /**< R/W error bitmask - TX address not acknowledged */
#define MLX90614_TXDATANACK     4       /**< R/W error bitmask - TX data not acknowledged */
#define MLX90614_TXOTHER        8       /**< R/W error bitmask - Unknown error */
#define MLX90614_RXCRC          0x10    /**< R/W error bitmask - Receiver CRC mismatch */
#define MLX90614_INVALIDATA     0x20    /**< R/W error bitmask - RX/TX Data fails selection criteria */
#define MLX90614_EECORRUPT      0x40    /**< R/W error bitmask - The EEProm is likely to be corrupted */
#define MLX90614_RFLGERR        0x80    /**< R/W error bitmask - R/W flags register access error */

/**************************************************************************************************/
/* MLX90614 Device class.                                                                         */
/**************************************************************************************************/

class MLX90614 {
public:
    MLX90614(uint8_t i2caddr = MLX90614_I2CDEFAULTADDR);

    boolean  begin();
    boolean  isReady(void) { return _ready; };
    uint64_t readID(void);                                  /**< Chip ID getter */

    uint8_t  getIIRcoeff(void);                             /**< IIR coefficient getter */
    uint8_t  getFIRcoeff(void);                             /**< IIR coefficient getter */
    float    getEmissivity(void);                           /**< Emissivity getter */
                                 
    void     setIIRcoeff(uint8_t csb = 4);                  /**< IIR coefficient setter */
    void     setFIRcoeff(uint8_t csb = 7);                  /**< IIR coefficient setter */
    void     setEmissivity(float emiss = 1.0);              /**< Emissivity setter */

    uint16_t readEEProm(uint8_t);
    void     writeEEProm(uint8_t, uint16_t);

    Property<uint8_t, MLX90614> busAddr;                    /**< SMBus address property */
    Property<uint8_t, MLX90614> rwError;                    /**< R/W error flags property */
    Property<uint8_t, MLX90614> crc8;                       /**< 8 bit CRC property */
    Property<uint8_t, MLX90614> pec;                        /**< PEC property */

    /** Enumerations for temperature units. */
    enum tempUnit_t {MLX90614_TK,                           /**< degrees Kelvin */
                     MLX90614_TC,                           /**< degrees Centigrade */
                     MLX90614_TF                            /**< degrees Fahrenheit */
    };
    /** Enumerations for temperature measurement source. */
    enum tempSrc_t  {MLX90614_SRCA,                         /**< Chip (ambient) sensor */
                     MLX90614_SRC01,                        /**< IR source #1 */
                     MLX90614_SRC02                         /**< IR source #2 */
                    };

    double   readTemp(tempSrc_t = MLX90614_SRC01, tempUnit_t = MLX90614_TC);
    double   convKtoC(double);
    double   convCtoF(double);

private:
    boolean  _ready;
    uint8_t  _addr;                                         /**< Slave address */
    uint8_t  _rwError;                                      /**< R/W error flags */
    uint8_t  _crc8;                                         /**< 8 bit CRC */
    uint8_t  _pec;                                          /**< PEC */

    uint16_t read16(uint8_t);
    void     write16(uint8_t, uint16_t);

    uint8_t  getRwError(void)   {return _rwError;}          /**< R/W error flags getter */
    uint8_t  getCRC8(void)      {return _crc8;}             /**< 8 bit CRC getter */
    uint8_t  getPEC(void)       {return _pec;}              /**< PEC getter */

    uint8_t  getAddr(void);                                 /**< SMB bus address getter */
    void     setAddr(uint8_t);                              /**< SMB bus address setter */

};

#endif /* _MLX90614_H_ */
