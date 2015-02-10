/*********************************************************************************************/
/**
 *  \brief      8 bit CRC helper/utility class - CPP Source file.
 *  \file       CRC8.CPP
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

#include "Crc8.h"

/*********************************************************************************************/
/*  CRC8 helper class functions.                                                             */
/*********************************************************************************************/

/**
 *  \brief            CRC8 class constructor.
 *  \param [in] poly  8 bit CRC polynomial to use.
 */
CRC8::CRC8(uint8_t poly) {crc8Start(poly);}

/**
 *  \brief            Return the current value of the CRC.
 *  \return           8 bit CRC current value.
 */
uint8_t CRC8::crc8(void) {return _crc;}

/**
 *  \brief            Update the current value of the CRC.
 *  \param [in] data  New 8 bit data to be added to the CRC.
 *  \return           8 bit CRC current value.
 */
uint8_t CRC8::crc8(uint8_t data) {
    uint8_t i = 8;
    
    _crc ^= data;
    while(i--) _crc = _crc & 0x80 ? (_crc << 1) ^ _poly : _crc << 1;
    return _crc;
}

/**
 *  \brief            Initialize the CRC8 object.
 *  \param [in] poly  8 bit CRC polynomial to use.
 */
void CRC8::crc8Start(uint8_t poly) {
    _poly = poly;
    _crc = 0;
}

