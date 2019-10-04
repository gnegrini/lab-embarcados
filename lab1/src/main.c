#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
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

float duty_cycle[MAX];
float frequencia_onda[MAX];
float periodo_onda[MAX];

float FACTOR = 20.0836;       //p/24MHz
//float FACTOR = 100.418;

void main(void){

  SysTickPeriodSet(FREQ_TIVA_24); 
  
  //PF1 Pino de entrada do sinal
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); // Habilita GPIO K
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)); // Aguarda final da habilitação
  
  GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_7); // Pino PF1 como entrada
  GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    
  amostrar_sinal();
  imprime(); 
}


void amostrar_sinal(){
  
  uint32_t ct_sample = 0;
  
  uint32_t count_a[MAX] = {0};
  uint32_t count_b[MAX] = {0};
  uint32_t clocks_onda[MAX] = {0};
  uint32_t sinal_inicio[MAX] = {0};
             
  while(ct_sample < MAX){
    
    sinal_inicio[ct_sample] = GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_7);
    
    count_a[ct_sample] = 0;
    count_b[ct_sample] = 0;
    clocks_onda[ct_sample] = 0;    
    
    // espera um subida de borda
    while(GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_7) == sinal_inicio[ct_sample] 
          || GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_7) != GPIO_PIN_7);
    
    while(GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_7) == GPIO_PIN_7) {
        count_a[ct_sample]++;
    }
    while(GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_7) != GPIO_PIN_7) {
        count_b[ct_sample]++;
    }
    
//    UARTprintf("Amostrado a: %d, b: %d \n", count_a, count_b);
    
    clocks_onda[ct_sample] = count_a[ct_sample] + count_b[ct_sample];
    duty_cycle[ct_sample] = (float) count_a[ct_sample] / (float) clocks_onda[ct_sample];
    periodo_onda[ct_sample] = ((float) clocks_onda[ct_sample] / (float) FREQ_TIVA_24) * FACTOR;
    frequencia_onda[ct_sample] = 1.0/periodo_onda[ct_sample];
      
    ct_sample++;    
  }
}

void imprime() {
  UARTInit(); 
  UARTprintf("Inicializado\n");
  
  char buffer[70];
  char duty[50];
  for (int i = 0; i < MAX; ++i) {
    sprintf(buffer, "Amostra: %d, Freq: %.2f, DutyC: %.2f %%, Per: %f\n", i, frequencia_onda[i], 
               duty_cycle[i]*100, periodo_onda[i]);
    sprintf(duty, "%.2f,", duty_cycle[i]*100);
    UARTprintf("%s", buffer);
  }
  
  UARTprintf("\nFim da amostragem\n");
  
  UARTDisable(UART0_BASE);
}

//Calcula estatísticas das medidas(média,desvio padrão e erro médio percentual) de sinaisde entrada

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
} // UARTInit

void UART0_Handler(void){
  UARTStdioIntHandler();
} // UART0_Handler