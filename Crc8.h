#ifndef _CRC8_H_
#define _CRC8_H_

/***********************************************************************************************//**
 *  \brief      8 bit CRC helper/utility class - CPP Header file.
 *  \file       CRC8.H
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

#define CRC8_DEFAULTPOLY  7  /**< Default CRC polynomial = X8+X2+X1+1 */

class CRC8 {
public:
    CRC8(uint8_t polynomial = CRC8_DEFAULTPOLY);
    uint8_t  crc8(void);
    uint8_t  crc8(uint8_t data);
    void     crc8Start(uint8_t poly);
private:
    uint8_t  _crc;
    uint8_t  _poly;
};

#endif /* _CRC8_H_ */
