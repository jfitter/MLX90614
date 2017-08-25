/***********************************************************************************************//**
 *  \brief      Melexis MCX90614BAA Test Program - Sensor test implementation.                
 *  \details    Arduino test implementation of Melexis MCX90614 PIR temperature sensor driver.
 *
 *  \note       THIS IS ONLY A PARTIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING
 *              ACTIVE DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP 
 *              THIS IN MIND IF YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.
 *
 *  \file       MelexisTest.ino                                     
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

#define MELEXISTEST_C
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <Arduino.h>
#include <Wire.h>
#include <MLX90614.h>
#include "printf.h"

MLX90614 mlx = MLX90614(MLX90614_BROADCASTADDR);      // *** must be only one device on bus ***

/**
 *  \brief  Program setup.
 */
void setup(void) {

    Wire.begin(); // library does not do this by default
    Serial.begin(115200);
    printf_begin();
    mlx.begin();  

    Serial.println(F("\nMelexis MLX90614 Temperature Sensor Test Program"));
    Serial.print(F("SMBus address ="));
    printf(" %02Xh", (uint8_t)mlx.readEEProm(MLX90614_ADDR));
    Serial.print(F("  Chip ID ="));

    uint64_t id = mlx.readID();
    printf(" %04X-%04X-%04X-%04X\n\n", (uint16_t)(id >> 48), (uint16_t)(id >> 32), 
                                       (uint16_t)(id >> 16), (uint16_t)id);
    dumpEEProm();
    Serial.println("");
}

/**
 *  \brief  Main processing loop.
 */
void loop(void) {
    static uint16_t smpcount = 0, errcount = 0;

    // read ambient temperature from chip and print out
    printlnTemp(mlx.readTemp(MLX90614::MLX90614_SRCA, MLX90614::MLX90614_TK), 'A');
    if(mlx.rwError) ++errcount;

    // read object temperature from source #1 and print out
    printlnTemp(mlx.readTemp(MLX90614::MLX90614_SRCO1, MLX90614::MLX90614_TK), 'O');
    if(mlx.rwError) ++errcount;

    // print running total of samples and errors
    Serial.print(F("        Samples:Errors "));
    printf("%u:%u\r\n", smpcount += 2, errcount);

    // slow down to human speed
    delay(250);
}

/**
 *  \brief           Print a line of temperature, crc, pec, and error string.
 *  \param [in] temp Temperature
 *  \param [in] src  Temperature source
 */
void printlnTemp(double temp, char src) {
    char str[20];

    if(mlx.rwError) Serial.print(F("No valid temperatures                              "));
    else {
        if(src == 'A') Serial.print(F("Ambient temperature"));
        else Serial.print(F("Object  temperature"));
        printf(" = %sK ", floatToStr(str, temp));
        printf("%sC ",    floatToStr(str, mlx.convKtoC(temp)));
        printf("%sF    ", floatToStr(str, mlx.convCtoF(mlx.convKtoC(temp))));
    }
    printCRC(mlx.crc8, mlx.pec);
    printErrStr(mlx.rwError);
    Serial.println("");
}

/**
 *  \brief Print a complete memory dump of the EEPROM.
 */
void dumpEEProm() {

    Serial.println(F("EEProm Dump"));
    for(uint8_t j=0; j<8; j++) {
        for(uint8_t i=0; i<4; i++) printf("%02Xh-%04Xh    ", j*4+i, mlx.readEEProm(j*4+i));
        printCRC(mlx.crc8, mlx.pec);
        printErrStr(mlx.rwError);
        Serial.println("");
    }
}

/**
 *  \brief          Utility to stringify a float.
 *  \param [in] str String to receive converted result
 *  \param [in] val Float value
 *  \return         Float as string
 */
char* floatToStr(char *str, double val) {

    sprintf(str, "%4d.%02u", int(val), int(val * 100) % 100);
    return str;
}

/**
 *  \brief          Just print the crc and pec.
 *  \param [in] crc CRC
 *  \param [in] pec PEC
 */
void printCRC(uint8_t crc, uint8_t pec) {printf("crc=%02Xh pec=%02Xh", crc, pec);}

/**
 *  \brief          Convert error flags to diagnostic strings and print.
 *  \param [in] err Error flags
 */
void printErrStr(uint8_t err) {

    Serial.print(F("  *** "));
    if(err == MLX90614_NORWERROR) Serial.print(F("RW Success"));
    else {
        Serial.print(F("Errors: "));
        if(err &  MLX90614_DATATOOLONG) Serial.print(F("Data too long / "));
        if(err &  MLX90614_TXADDRNACK)  Serial.print(F("TX addr NACK / "));
        if(err &  MLX90614_TXDATANACK)  Serial.print(F("TX data NACK / "));
        if(err &  MLX90614_TXOTHER)     Serial.print(F("Unknown / "));
        if(err &  MLX90614_RXCRC)       Serial.print(F("RX CRC / "));
        if(err &  MLX90614_INVALIDATA)  Serial.print(F("Invalid data / "));
        if(err &  MLX90614_EECORRUPT)   Serial.print(F("EEPROM / "));
        if(err &  MLX90614_RFLGERR)     Serial.print(F("RFlags / "));
    }
}

/**
 *  \brief  EEPROM memory contents factory default values.
 */
const struct defaultEEPromData {
    uint8_t  address;
    uint16_t data;
} eDat[] =  {{0x20, 0x9993}, {0x21, 0x62E3}, {0x22, 0x0201}, 
             {0x23, 0xF71C}, {0x24, 0xFFFF}, {0x25, 0x9FB4}, 
             {0x2E, 0xBE5A}, {0x2F, 0x0000}, {0x39, 0x0000}};

/**
 *  \brief    Set EEPROM memory contents to factory default values.
    \remarks  A device with default adress must not be on the bus.
    \n<tt>    Only user allowed memory locations are written.
 */
void setEEPromDefaults(void) {

    for(uint8_t i = 0; i < sizeof(eDat)/sizeof(defaultEEPromData),
        !mlx.rwError; i++) { 
        mlx.writeEEProm(eDat[i].address, eDat[i].data);
    }
}

