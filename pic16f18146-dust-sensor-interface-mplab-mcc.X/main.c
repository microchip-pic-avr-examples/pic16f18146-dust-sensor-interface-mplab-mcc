 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
*/

/*
© [2023] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
#include "mcc_generated_files/system/system.h"

/***********************************
 Analog (Dust sensor output) channel: RC2
 CLC3 o/p (TMR2 output): RC6  -- for testing purpose
 PWM1 o/p : RC5
 
 PWM clock: 500 kHz
 PWM frequency : 100 Hz
 PWM period : 10 ms
 PWM duty cycle(P1) : 0xA0 (160) -> 320 us ON time  
 PR : 0x1387 (4999) because 500 kHz * (4999 + 1) = 10 ms
 
 TMR2 clock: 500 kHz
 TMR2 period : 0x8B (139) ==> 280 us
 *************************************/
#define ADC_HIGHEST_COUNT (4095U)
#define ADC_REFERENCE_VOLATGE (5.0F)

float ConvertAdcCountToVoltage(uint16_t adcResult);
float CalculateDustDensity(float voltage); 

static float voltageNoDust = 0.6F; //typical output voltage at no dust in volt. 

//sensitivity needs to be adjusted as per calibration for accurate result
static const float sensitivity = 0.5F; //typical sensitivity of dust sensor in volt per 100ug/m3

int main(void)
{
    uint16_t adcCount = 0;
    float outputVoltage = 0;
    float dustDensity = 0; 
    
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts 
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts 
    // Use the following macros to: 

    // Enable the Global Interrupts 
    //INTERRUPT_GlobalInterruptEnable(); 

    // Disable the Global Interrupts 
    //INTERRUPT_GlobalInterruptDisable(); 

    // Enable the Peripheral Interrupts 
    //INTERRUPT_PeripheralInterruptEnable(); 

    // Disable the Peripheral Interrupts 
    //INTERRUPT_PeripheralInterruptDisable(); 
    
    printf("\n\t\t**** Dust (PM) Sensor Interface using PIC16F18146 ****\n\n\r");
    
    //select Dust Sensor as ADC channel
    ADPCH = DustSensor_Vo; 
    
    while (1)
    {
        adcCount = ADCC_GetConversionResult();
        printf("ADC Count: %u, " , adcCount);
        
        outputVoltage = ConvertAdcCountToVoltage(adcCount);    
        printf("Output Voltage: %.1f V, ", outputVoltage);
        
        dustDensity = CalculateDustDensity(outputVoltage);
        printf("Dust Density: %.1f ug/m3\n\n\r", dustDensity);
             
        //delay is added to avoid fast printing. It can be removed or modified according to application needs.
        __delay_ms(1500);
    }  
}

/**
 * @brief This function converts ADC count into equivalent voltage.
 * @param [in] adcResult - ADC count
 * @return equivalent voltage
 */
float ConvertAdcCountToVoltage(uint16_t adcResult)
{
    float voltage = 0;
    
    //compute the output voltage in Volts.
    voltage = (float)(adcResult * ADC_REFERENCE_VOLATGE);     
    voltage = voltage / ADC_HIGHEST_COUNT;
    
    return voltage;
}

/**
 * @brief This function calculates dust density.
 * @param [in] voltage - dust sensor output voltage
 * @return dust density in ug/m3
 */
float CalculateDustDensity(float outputVoltage) 
{
    //difference of output dust and voltage at no dust
    float deltaVoltage = 0;         
    float dustValue = 0;
    
    deltaVoltage = outputVoltage - voltageNoDust;

    //dynamically adjusting output voltage under zero dust conditions
    if (deltaVoltage < 0)     
    {
        deltaVoltage = 0;
        
        //offset correction
        voltageNoDust = outputVoltage;    
    }
    
    dustValue = deltaVoltage / sensitivity * 100.0F;
    
    return dustValue;
}