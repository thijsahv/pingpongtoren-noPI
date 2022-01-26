/**
  Section: Included Files
 */

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "PI.h"
#include <stdlib.h> //for atoi and atof functions
#include <ctype.h> //for toupper kommand

/**
  Section: UART Module APIs
 */

static uint8_t sensorHeight;
static uint8_t setpoint; //240 = top, 0 = bottom
static float ki;
float kp;

static char kommand;
static char data[8]; //"S20" of "p1.25"
static char value[7];

bool mustPrintLogs = true;

#define EUART_READ_LINE_BUFFER_LENGTH 20
char line[EUART_READ_LINE_BUFFER_LENGTH];

#define PRINT_BUFFER_LENGTH 11
char printBuffer[PRINT_BUFFER_LENGTH];




//read the uart and only return after a line feed character is received

char* readLine() {
    static uint8_t index = 0;
    while (1) {
        if (!EUSART_is_rx_ready()) {
            line[index] = 0;
            return line + index;
        }
        if (index >= EUART_READ_LINE_BUFFER_LENGTH - 1) {
            while (EUSART_is_rx_ready()) {
                EUSART_Read();
            }
            printf("input buffer overflow\r\n");
            index = 0;
            line[index ] = 0;
            return line;
        }

        line[index] = EUSART_Read();
        //                EUSART_Write(line[index]);
        if (line[index] == '\n') {
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


//string naar komma getal getal omvormen

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

void printNewLine() {
    EUSART_Write('\r');
    EUSART_Write('\n');
}

// print een geheel getal van max 32 bits naar de COM poort

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
    uint16_t voorComma = value;
    uint32_t naComma = (value - voorComma)*100000;
    printUint32(voorComma, 0);
    printf(",");
    printUint32(naComma, 5);
}

void printLogs(){
    //Write to JAVA
    static uint8_t printCycle = 0; //door static toe te voegen wordt "printCycle" niet elke keer her geinitialiseerd maar behoud het zijn vorige waarde

    if (printCycle++ > 30 && mustPrintLogs) {
        sensorHeight = PI_GetSensorHeight();
        printf("hoogte: "); // data wegschrijven naar COM poort
        printUint32(sensorHeight, 0);
        printf(", setPoint: ");
        printUint32(PI_GetSetPoint(), 0);
        printNewLine(); // nieuwe lijn
        printCycle = 0;
    }
}

void Java(void) {
    

    char * str = readLine();
    switch (*str) {
        case 0:
            break;
        case 'S': //Setpoint                            
        case 's': //Setpoint                            
            setpoint = str2uint8(str + 1); //string omvormen naar getal

            PI_SetSetpoint(setpoint);
            printf("setpoint ");
            printUint32(setpoint, 0);
            printf("\r\n");
            break;
        case 'P': //Proportional                           
        case 'p': //Proportional                           
            kp = str2float(str + 1);
            PI_SetKp(kp);
            printf("Kp ");
            printFloat(PI_GetKp());
            printNewLine();
            break;
        case 'I': //Integrate                                           
        case 'i': //Integrate                                           
            ki = str2float(str + 1);
            PI_SetKi(ki);
            printf("Ki ");
            printFloat(PI_GetKi());
            printNewLine();
            break;
        case 'L': //logs                                        
        case 'l':                                          
            mustPrintLogs = !mustPrintLogs;
            printf("logs toggled\r\n");
            break;
        default:
            printf("Command not supported. ");
            printStr(str);
    };



    //    
    //    //Read from JAVA
    //    if (EUSART_DataReady) {
    //        index = 0;
    //        __delay_ms(30); // wacht tot alle data ontvangen is
    //        while (EUSART_DataReady) {
    //            data[index] = EUSART_Read(); // ontvangen data lezen          
    //            index++;
    //        }
    //        data[index] = '\0'; // \0 toevoegen voor atoi en atof functies
    //        kommand = data[0]; // eerste char = kommand
    //        kommand = (char) toupper(kommand); //converteer naar hoofdletter (voor de zekerheid)
    //        for (int i = 0; i <8; i++){ //value = data vanaf het 2e karakter
    //            value[i] = data[i+1];
    //        }
    //
    //        switch (kommand) {
    //            case 'S': //Setpoint                            
    //                setpoint = (uint8_t) atoi(value); //atoi = ASCII to integer
    //                PI_SetSetpoint(setpoint);
    //                break;
    //            case 'P': //Proportional                           
    //                kp = (float) atof(value); //atof = ASCII to float
    //                PI_SetKp(kp);
    //                break;
    //            case 'I': //Integrate                                           
    //                ki = (float) atof(value);
    //                PI_SetKi(ki);
    //                break;
    //        };
    //    }
}

/**
 End of File
 */
