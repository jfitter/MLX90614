/*********************************************************************************************/
/* Brief      Melexis MCX90614BAA Test Program - Sensor test implementation.                 */
/* Details    Arduino test implementation of Melexis MCX90614 PIR temperature sensor driver. */
/*                                                                                           */
/* Note       THIS IS ONLY A PARTIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING      */
/*            ACTIVE DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP   */
/*            THIS IN MIND IF YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.           */
/*                                                                                           */
/* File       MelexisTest.ino                                                                */
/* Author     J. F. Fitter <jfitter@eagleairaust.com.au>                                     */
/* Version    1.0                                                                            */
/* Date       2014-2015                                                                      */
/* Copyright  Copyright (c) 2015 John Fitter.  All right reserved.                           */
/*                                                                                           */
/* License    GNU Public License. Permission is hereby granted, free of charge, to any       */
/*            person obtaining a copy of this software and associated documentation files    */
/*            (the "Software"), to deal in the Software without restriction, including       */
/*            without limitation the rights to use, copy, modify, merge, publish, distribute,*/
/*            sublicense, and/or sell copies of the Software, and to permit persons to whom  */
/*            the Software is furnished to do so, subject to the following conditions:       */
/*                                                                                           */
/*            The above copyright notice and this permission notice shall be included in     */
/*            all copies or substantial portions of the Software.                            */
/*                                                                                           */
/*            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/*            IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/*            FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/*            AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/*            LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/*            OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN      */
/*            THE SOFTWARE.                                                                  */
/*                                                                                           */
/*********************************************************************************************/


#define MELEXISTEST_C
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <Arduino.h>
#include <Wire.h>
#include <MLX90614.h>
#include "MelexisTest.h"
#include "printf.h"

MLX90614 mlx = MLX90614(MLX90614_BROADCASTADDR);      // *** must be only one device on bus ***

/*********************************************************************************************/
/* PROGRAM SETUP                                                                             */
/*********************************************************************************************/

void setup(void) {

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

/*********************************************************************************************/
/* MAIN PROCESSING LOOP                                                                      */
/*********************************************************************************************/

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

// Print a line of temperature, crc, pec, and error string
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

// Print a complete memory dump of the EEPROM
void dumpEEProm() {

    Serial.println(F("EEProm Dump"));
    for(uint8_t j=0; j<8; j++) {
        for(uint8_t i=0; i<4; i++) printf("%02Xh-%04Xh    ", j*4+i, mlx.readEEProm(j*4+i));
        printCRC(mlx.crc8, mlx.pec);
        printErrStr(mlx.rwError);
        Serial.println("");
    }
}

// Utility to stringify a float
char* floatToStr(char *str, double val) {

    sprintf(str, "%4d.%02u", int(val), int(val * 100) % 100);
    return str;
}

// Just print the crc and pec
void printCRC(uint8_t crc, uint8_t pec) {printf("crc=%02Xh pec=%02Xh", crc, pec);}

// Convert error flags to diagnostic strings and print
void printErrStr(uint8_t err) {

    Serial.print(F("  *** "));
    if(err == MLX90614_SUCCESS) Serial.print(F("RW Success"));
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

// Set EEPROM memory contents to factory default values.
// A device with default adress must not be on the bus.
// Only user allowed memory locations are written.

// Default EEPROM data
const struct defaultEEPromData {
    uint8_t  address;
    uint16_t data;
} eDat[] =  {{0x20, 0x9993}, {0x21, 0x62E3}, {0x22, 0x0201}, 
             {0x23, 0xF71C}, {0x24, 0xFFFF}, {0x25, 0x9FB4}, 
             {0x2E, 0xBE5A}, {0x2F, 0x0000}, {0x39, 0x0000}};

void setEEPromDefaults(void) {

    for(uint8_t i = 0; i < sizeof(eDat)/sizeof(defaultEEPromData),
        !mlx.rwError; i++) { 
        mlx.writeEEProm(eDat[i].address, eDat[i].data);
    }
}

