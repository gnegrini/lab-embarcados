#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "system_TM4C1294.h" 
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"

#define MAX (10)


void GPIOInit(){
    
  ///MotorDir GPIO (PE0 e PE1) e Pot Setpoint GPIO (PE4)  
  // Enable and wait for the GPIOE peripheral
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)){};
  
  // Set pins 0 and 1 as output, SW controlled
  GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  // Set pins 4 as input, SW controlled
  GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4);
  
  ///PWM GPIO (PF2)
  // Enable and wait for the GPIOF peripheral
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}
  
  // Configure GPIO Port F pin 2 to be used as PWM.//
  GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
  
  // Enable PWM2 functionality on GPIO Port F pin 2.
  GPIOPinConfigure(0x00050806);   //GPIO_PF2_M0PWM2: 0x00050806
 
}

//initialize  the  PWM2  with  a  50  KHzfrequency, and with a 25% duty cycle
void PWMInit (void){

  // The TM4C1294NCPDT microcontroller contains *one* PWM module, 
  // with four PWM generator blocks and a control block, for a total of 8 PWM outputs.
  // Each PWM generator block produces two PWM signals that share the same timer and frequency
  // PF2_M0PWM2 : Motion Control Module 0 PWM 2. This signal is controlled by Module 0 PWM Generator 1  
  
  // Enable the PWM0 peripheral
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
  
  // Wait for the PWM0 module to be ready.
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)){}
  
  // Configure the PWM generator for count down mode with immediate updates
  // to the parameters. PWM_GEN_MODE_DBG_RUN permite ao contador continuar mesmo no debug
  PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DBG_RUN | PWM_GEN_MODE_NO_SYNC);

 
  // Set the period.  For a 50 KHz frequency, the period = 1/50,000, or 200
  // nano seconds.  For a 20 MHz clock, this translates to 400 clock ticks.
  // 20MHz => 20.000.000 ticks every second or 1 tick every 50 nanosecond
  // 50Khz => 20 microseconds
  // How many clock ticks in 20 mcsec? = 20e-6 / 50e-9 = 400
  // Or: 20Mhz / 50Khz = 400 ticks
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 400);
  
  // Set the pulse width of PWM2 for a 25% duty cycle.
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, 100);
  
  // Start the timers in generator 1.
  PWMGenEnable(PWM0_BASE, PWM_GEN_1);
  
  // Enable the outputs.//
  PWMOutputState(PWM0_BASE, (PWM_OUT_2_BIT), true);

}


void main(void){
    
  GPIOInit();
  PWMInit();
  
  
  // Set the pulse width of PWM2 for a 75% duty cycle.
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, 300);
  
  // Girar motor no Sentido Horario
  GPIOPinWrite(GPIO_PORTE_BASE,(GPIO_PIN_1 | GPIO_PIN_0),(GPIO_PIN_0));
  
  // Girar motor no Sentido Anti-Horario
  GPIOPinWrite(GPIO_PORTE_BASE,(GPIO_PIN_1 | GPIO_PIN_0),(GPIO_PIN_1));

  
  
}