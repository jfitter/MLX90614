#ifndef _CRC8_H_
#define _CRC8_H_

/*********************************************************************************************/
/**
 *  \brief      8 bit CRC helper/utility class - CPP Header file.
 *  \file       CRC8.H
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
