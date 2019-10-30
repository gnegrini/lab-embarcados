#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


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
#define MAX (20)

extern void UARTStdioIntHandler(void);
void UARTInit(void);
void UART0_Handler(void);
void amostrar_sinal();
void imprime();

//uint32_t FREQ_TIVA_24 = 24000000;    //24000000        // f = 1Hz para clock = 24MHz
//uint32_t FREQ_TIVA_120 = 120000000;   //120000000       // f = 100MHz

uint32_t time_out = 0;
uint32_t borda = 0;
uint32_t sample[MAX] = {0};
uint32_t ct_sample = 0; 

void TIMER0Init();
void TIMER2Init();

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


void PORTInit(){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); // Habilita GPIO M
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)); // Aguarda final da habilitação
  
  GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_0); // Pino PM0 como entrada
  //GPIOPadConfigSet(GPIO_PORTM_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_0);
  GPIOPinConfigure(0x000B0003); //GPIO_PM0_T2CCP0
  
}


void TIMER0Init(){

  //Ativa o timer0 e espera ficar pronto
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){
  }
  
  //TimerA full one-shot timer
  TimerConfigure(TIMER0_BASE, TIMER_CFG_A_ONE_SHOT);
    
  //Ajusta o tempo do OneShot 5s
  TimerLoadSet(TIMER0_BASE, TIMER_A, SystemCoreClock * 5);
  
  //Ativa a interrupcao no timer
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


void TIMER2Init(){
  //Ativa o timer2 e espera ficar pronto
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2)){
  }
  
  //Timer2 full-width time capture up
  TimerConfigure(TIMER2_BASE, TIMER_CFG_A_CAP_TIME_UP);    
  
  //Configura o Timer2 para ambas bordas
  TimerControlEvent(TIMER2_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);
  
  TimerLoadSet(TIMER2_BASE, TIMER_A, 0);
  
  //Ativa as interrupcoes no timer2
  TimerIntEnable(TIMER2_BASE, TIMER_CAPA_EVENT);
    
}


//Handler de quando ocorre um timeout
void TIMER0A_Handler(){
  
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  
  time_out = 1;
}

//Handler de quando ocorre uma mudanca de borda
void TIMER2A_Handler(){
//Limpa o flag para permitir novas interrupções
  TimerIntClear(TIMER2_BASE, TIMER_CAPA_EVENT);

//reset no tempo de time_out    
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);            
 
  borda = 1;
}


//calcula e imprime os dados da onda a partir das amostras já obtidas
void imprime() {
  
  float duty_cycle;
  float frequencia_onda;
  float periodo_onda;
 
  
  char buffer[70];
  char duty[50];
  for (int i = 0; i < MAX/2; ++i) {
    
    periodo_onda = sample[i]+ sample[i+1];
    frequencia_onda = 1/periodo_onda;
    duty_cycle = sample[i] / sample[i+1];    
    
    
    sprintf(buffer, "Amostra: %d, Freq: %.2f, DutyC: %.2f %%, Per: %f\n", i, frequencia_onda, 
               duty_cycle*100, periodo_onda);
    sprintf(duty, "%.2f,", duty_cycle*100);
    UARTprintf("%s", buffer);
  }
  
  UARTprintf("\nFim da amostragem\n");
  
  UARTDisable(UART0_BASE);
}


void main(void){
 
  UARTInit();
  PORTInit();   //PM0
  
  TIMER0Init(); //PL4
  TIMER2Init(); //PM0
  
  
  //ativa interrupcoes
  IntEnable(INT_TIMER0A);
  
  
  //Ativa o timer0
  TimerEnable(TIMER0_BASE, TIMER_A);
  
  //espera o sinal ficar em baixa para ativar o timer2  (p/ começar pegando borda de subida)
  while(GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_0) == GPIO_PIN_0);    
  
  //Ativa o timer2
  TimerEnable(TIMER2_BASE, TIMER_A);
  IntEnable(INT_TIMER2A);
  IntMasterEnable();
  
  while(ct_sample<MAX){
   
    if(borda){
      sample[ct_sample] = TimerValueGet(TIMER2_BASE, TIMER_A);
      ct_sample++;
      borda = 0;
    } else
    
    if(time_out) {
      UARTprintf("Time Out\n");
      UARTDisable(UART0_BASE);
      exit(1);
    }
    
    
        
  }   
  
  imprime();
  
 }

