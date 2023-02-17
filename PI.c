/**
  Section: Included Files
 */

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "PI.h"

/**
  Section: PI Module APIs
 */

static uint8_t sensorHeight = 0;
static uint8_t setpoint = 80; 
static int error = 0;
static float integral = 0;
static float ki = 0.004;
static float kp = 0.4;
static int dutycycle;

uint8_t PI_GetSensorHeight(void) {
    return sensorHeight;
}

void PI_SetSetpoint(uint8_t value) {
    setpoint = value;
}
uint8_t PI_GetSetPoint(void) {
    return setpoint;
}


int PI_GetDutycycle(void) {
    return dutycycle;
}

void PI_SetKp(float value) {
    kp = value;
}
float PI_GetKp(void) {
    return kp;
}

void PI_SetKi(float value) {
    ki = value;
}
float PI_GetKi(void) {
    return ki;
}

void PI(void) {
    sensorHeight = (uint8_t) (ADC_GetConversionResult() >> 2); //resultaat van ADC (8 bit )

    //Hier dient jullie code toegevoegd te worden
    error = sensorHeight - setpoint
    dutycycle = error * PI_GetKp() + PI_GetKi()  // moeten hier dus een waarde terugvinden om te geven aan ventilator om blaassterkte aan te passen, zou ifv error moeten zijn
    
            

    PWM5_LoadDutyValue( (uint16_t) dutycycle); // output pwm signaal voor hoogte 10 bit (van 0 tot 0x3FF)
}

/**
 End of File
 */
