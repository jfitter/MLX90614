/***********************************************************************************************//**
 *  \brief      8 bit CRC helper/utility class - CPP Source file.
 *  \file       CRC8.CPP
 *  \author     J. F. Fitter <jfitter@eagleairaust.com.au>
 *  \version    1.0
 *  \date       2014-2017
 *  \copyright  Copyright &copy; 2017 John Fitter.  All right reserved.
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

#include "Crc8.h"

/**************************************************************************************************/
/*  CRC8 helper class functions.                                                                  */
/**************************************************************************************************/

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

