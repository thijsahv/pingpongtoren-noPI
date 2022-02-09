/**
 * Author:    Cedric Baijot
 * Created:   09/02/2022
 */

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "PI.h"
#include <stdlib.h> //for atoi and atof functions
#include <ctype.h> //for toupper kommand

/**
  Section: UART Module APIs
 */

bool mustPrintLogs = true;

#define EUART_READ_LINE_BUFFER_LENGTH 20
char line[EUART_READ_LINE_BUFFER_LENGTH];

#define PRINT_BUFFER_LENGTH 11
char printBuffer[PRINT_BUFFER_LENGTH];



void printNewLine() {
    EUSART_Write('\r');
    EUSART_Write('\n');
}

//read the uart to a string and only return the string if a line feed character is received, otherwise return a empty string but keep the string in memmory for the next reading
char* readLine() {
    static uint8_t index = 0;
    while (1) {
        if (!EUSART_is_rx_ready()) {
            line[index] = 0;
            return line + index;//return empty string
        }
        if (index >= EUART_READ_LINE_BUFFER_LENGTH - 1) {
            while (EUSART_is_rx_ready()) {
                EUSART_Read();
            }
            printf("input buffer overflow");
            printNewLine();
            index = 0;
            line[index ] = 0;
            return line;
        }

        line[index] = EUSART_Read();
        if (line[index] == '\n') {//end of command
            line[index + 1] = 0;
            index = 0;
            return line;
        }
        index++;
    }
}

//string naar geheel getal van 8 bits omvormen
uint8_t str2uint8(char *str) {
    uint8_t getal = 0;
    for (int i = 0; i < 3; i++) {
        char cijfer = *str++;
        if ('0' <= cijfer && cijfer <= '9') {
            getal *= 10;

            getal += cijfer - '0';
        } else {
            return getal;
        }
    }
    return getal;
}


//string naar komma getal omvormen
float str2float(char *str) {
    float getal = 0;
    bool kommaPresent = false;
    uint8_t kommaCounter;
    while (1) {
        char cijfer = *str++;
        if ('0' <= cijfer && cijfer <= '9') {
            getal *= 10;
            getal += cijfer - '0';
            kommaCounter++;
        } else if (cijfer == ',' || cijfer == '.') {
            kommaPresent = true;
            kommaCounter = 0;
        } else {
            break;
        }

    }
    for (int i = 0; kommaPresent && i < kommaCounter; i++) {
        getal /= 10;
    }
    return getal;
}



// een string naar de COM poort printen
void printStr(char*str) {
    while (*str != 0) {
        EUSART_Write(*str++);
    }
}


// print een geheel getal van max 32 bit naar de COM poort
void printUint32(uint32_t getal, uint8_t leadingZeros) {
    uint8_t i;
    for (i = 1; i <= PRINT_BUFFER_LENGTH - 1; i++) {
        printBuffer[PRINT_BUFFER_LENGTH - 1 - i] = getal % 10 + '0';
        getal /= 10;
        if (getal == 0 && i >= leadingZeros) {
            break;
        }
    }
    printBuffer[PRINT_BUFFER_LENGTH - 1] = 0;
    printStr(printBuffer + PRINT_BUFFER_LENGTH - 1 - i);
}


// print een komma getal met 5 cijfers na de komma
void printFloat(float value) {
    uint16_t voorComma = (uint16_t) value;
    uint32_t naComma =(uint32_t) ((value - voorComma)*100000);
    printUint32(voorComma, 0);
    printf(",");
    printUint32(naComma, 5);
}

void printLogs() {
    //Write to com port
    if (mustPrintLogs) {
        printf("hoogte: ");
        printUint32(PI_GetSensorHeight(), 3); //print sensorheight als geheel getal van 3 cijfers
        printf(", setPoint: ");
        printUint32(PI_GetSetPoint(), 3); 
        printf(", duty cycle: ");
        printUint32((uint16_t) PI_GetDutycycle(), 3); 
        printf(", ki: ");
        printFloat(PI_GetKi()); //print ki als komma getal (5 cijfers na de komma)
        printf(", kp: ");
        printFloat(PI_GetKp()); 
        printNewLine();
    }
}

void uartHandler(void) {
    char * str = readLine();
    switch (*str) {//empty string
        case 0:
            break;
        case 'S': //Setpoint                            
        case 's':
            PI_SetSetpoint(str2uint8(str+1));
            printf("setpoint ");
            printUint32(PI_GetSetPoint(), 0);
            printNewLine();
            break;
        case 'P': //Proportional                           
        case 'p':
            PI_SetKp(str2float(str + 1));
            printf("Kp ");
            printFloat(PI_GetKp());
            printNewLine();
            break;
        case 'I': //Integrate                                           
        case 'i':
            PI_SetKi(str2float(str + 1));
            printf("Ki ");
            printFloat(PI_GetKi());
            printNewLine();
            break;
        case 'L': //logs                                        
        case 'l':
            mustPrintLogs = !mustPrintLogs;
            printf("logs toggled");
            printNewLine();
            break;
        default:
            printf("Command not supported. ");
            printStr(str);
    };
}

/**
 End of File
 */
