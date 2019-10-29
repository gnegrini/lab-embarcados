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

uint32_t time_out = 0;

void handlerA();
void handlerB();
void TIMER0Init();
void TIMER2Init();


void TIMER0Init(){

  //Ativa o timer0 e espera ficar pronto
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){
  }
  
  //TimerA half-width one-shot timer
  TimerConfigure(TIMER0_BASE, TIMER_CFG_A_ONE_SHOT);
    
  //Ajusta o tempo do OneShot
  TimerLoadSet(TIMER0_BASE, TIMER_A, SystemCoreClock * 5.75);
  
  
  //Configura o TimerB para ambas bordas, mudar para TIMER_CFG_A_CAP_TIME_U
  //TimerControlEvent(TIMER0_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);  
  
  //Ativa as interrupcoes nos timers
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void TIMER2Init(){
  //Ativa o timer0 e espera ficar pronto
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2)){
  }
  
  //TimerA half-width one-shot timer
  TimerConfigure(TIMER2_BASE, TIMER_CFG_A_CAP_TIME_U);
    
  //Ajusta o tempo do OneShot
  //TimerLoadSet(TIMER2_BASE, TIMER_A, SystemCoreClock * 5.75);
  
  
  //Configura o TimerB para ambas bordas, mudar para TIMER_CFG_A_CAP_TIME_U
  TimerControlEvent(TIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);  
  
  //Ativa as interrupcoes nos timers
  TimerIntEnable(TIMER2_BASE, TIMER_CAPA_EVENT);
  
  
}

void main(void){

 
  UARTInit();
  
  
  TIMER0Init();
  TIMER2Init();
  
  
  
  //ativa interrupcoes
  IntEnable(INT_TIMER0A);
  IntMasterEnable();
  
  //Ativa os timers
  TimerEnable(TIMER0_BASE, TIMER_A);
  
  
  
  while(1){
   
    if(time_out) {
      UARTprintf("Time Out\n");
      UARTDisable(UART0_BASE);
    }
    
  } 
  
  
 }

//Handler de quando ocorre um timeout
void TIMER0A_Handler(){
  
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  //enviar mensagem pela uart  
  
  time_out = 1;
    
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