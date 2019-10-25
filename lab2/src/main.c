#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "system_TM4C1294.h" 
#define MAX (10)

extern void UARTStdioIntHandler(void);
void UARTInit(void);
void UART0_Handler(void);
void amostrar_sinal();
void imprime();

uint32_t FREQ_TIVA_24 = 24000000;    //24000000        // f = 1Hz para clock = 24MHz
uint32_t FREQ_TIVA_120 = 120000000;   //120000000       // f = 100MHz


void handlerA();
void handlerB();

void main(void){

  //SysTickPeriodSet(FREQ_TIVA_24);
  
  //SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
  //                 SYSCTL_XTAL_8MHZ);
  
  //Ativa o timer0 e espera ficar pronto
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){
  }
  
  //TimerA half-width one-shot timer, timerB half-width edge capture counter
  TimerConfigure(TIMER0_BASE, 
        (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT |TIMER_CFG_B_CAP_COUNT));
  
  IntMasterEnable();
  
  
  
  
  //Ajusta o tempo do OneShot
  TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() * 3);
  
  
  //Configura o TimerB para ambas bordas, mudar para TIMER_CFG_A_CAP_TIME_U
  TimerControlEvent(TIMER0_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
  
  
  //Ativa as interrupcoes nos timers
  
  IntEnable(INT_TIMER0A);
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT | TIMER_CAPB_EVENT);
  
  
  //Configura as interrupcoes nos timers
  TimerIntRegister(TIMER0_BASE, TIMER_A, handlerA);
  TimerIntRegister(TIMER0_BASE, TIMER_B, handlerB);
  
  //Ativa os timers
  TimerEnable(TIMER0_BASE, TIMER_BOTH);
  
  UARTInit();
 }

//Handler de quando ocorre um timeout
void handlerA(){
  
  //TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  //enviar mensagem pela uart  
  
  UARTprintf("Time Out\n");
  
  UARTDisable(UART0_BASE);
  
  //travar programa
  while(1);
}

void handlerB(){
}

//Handler de quando ocorre uma mudanca de borda
//void handlerB(){
////Recupera o tempo do TimerB
//   TimerValueGet(TIMER0_BASE, TIMER_B);
//
//   ...
//
//  //Limpla o flag para permitir novas interrupções
//  TimerIntClear(TIMER0_BASE, TIMER_CAPB_EVENT);
//}


void imprime() {
  UARTprintf("Time Out\n");
  
  UARTDisable(UART0_BASE);
}


void UARTInit(void){
  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Initialize the UART for console I/O.
  UARTStdioConfig(0, 9600, SystemCoreClock);
  
  UARTprintf("UART Init\n");
  
} // UARTInit

void UART0_Handler(void){
  UARTStdioIntHandler();
} // UART0_Handler