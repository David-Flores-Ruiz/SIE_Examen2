/*
 * @file:			P3_PANEL_ELECTRONICO.c
 * @company:		ITESO
 * @Engineer Team:	D.F.R. / R.G.P.
 * @contact:		ie717807@iteso.mx
 */

#include "MK64F12.h"
#include <stdint.h>
#include <stdio.h>
#include <string_driver.h>
#include "UART_driver.h"
#include "I2C.h"
#include "memoria.h"
#include "bits.h"
#include "NVIC.h"
#include "GPIO.h"
#include "RTC.h"
#include "display.h"	//* 8 x 16 LED matrix */
#include "TeraTerm.h"


//#define DEBUG_ON	 	//** For debugging purposes */


#define SYSTEM_CLOCK (10500000U)
#define BAUD_RATE (9600U)


int main(void)
{
	/**Enables the clock of PortB in order to configures TX and RX of UART peripheral*/
	SIM->SCGC5 = SIM_SCGC5_PORTB_MASK;
	/**Configures the pin control register of pin16 in PortB as UART RX*/
	PORTB->PCR[16] = PORT_PCR_MUX(3);
	/**Configures the pin control register of pin16 in PortB as UART TX*/
	PORTB->PCR[17] = PORT_PCR_MUX(3);
	/**Configures UART 0 to transmit/receive at 11520 bauds with a 21 MHz of clock core*/


	UART_init (UART_0,  10200000, BD_115200);
	printf("UART is configured");
	/**Enables the UART 0 interrupt*/
	UART_interrupt_enable(UART_0);
	/**Enables the UART 0 interrupt in the NVIC*/
	NVIC_enable_interrupt_and_priotity(UART0_IRQ, PRIORITY_10);
	/**Enables interrupts*/
	NVIC_global_enable_interrupts;


	TeraTerm_Menu(); /**Show the initial Menu for the User*/

	I2C_init(I2C_0, SYSTEM_CLOCK, BAUD_RATE);

	RTC_write_Seconds();	/** Configurado para Inicializar el registro de Segundos*/
	RTC_write_Minutes();	/** Configurado para Inicializar el registro de Minutos */
	RTC_write_Hours();		/** Configurado para Inicializar el registro de Horas */
	RTC_write_DATE();
	RTC_write_MONTH();
	RTC_write_YEAR();

	display_init();		//** Inicializa matriz de LEDS*/
	display_clear();	//** Limpia pantalla 		  */

	while (1) {

#ifdef DEBUG_ON
		RTC_Print_in_MCUXpresso_Console();	//** Only for Debug the RTC registers */
#endif

		TeraTerm_MENU_FSM();

	}

    return 0 ;
}



