/*
 * I2C.c
 *
 *  Created on: Nov 6, 2019
 *      Author: David Ruiz
 */

#include "I2C.h"

#define MULTIPLIER (2U)

/*
	void I2C0_IRQHandler(void) {

		I2C0->FLT |= I2C_FLT_STARTF_MASK;
		I2C0->FLT |= I2C_FLT_STOPF_MASK;

		I2C0->S |= I2C_S_IICIF_MASK;		// Limpiamos bandera de interrupcion
	}
*/

void I2C_init(i2c_channel_t channel, uint32_t system_clock, uint16_t baud_rate) {
	if (I2C_0 == channel) {
		SIM->SCGC4 |= I2C0_CLOCK_GATING; /* Bit_6 = 0x40h*/
		/**	Configurar el Clock Gating de los perifericos GPIO a utilizar */
		GPIO_clock_gating(GPIO_B);
		/** Utiliza la alternativa 2 para I2C_SCL(reloj) y I2C_SDA(datos) */
		gpio_pin_control_register_t I2C_config = GPIO_MUX2;
		/** Asignamos los pines de salidas del protocolo de comunicacion I2C */
		GPIO_pin_control_register(GPIO_B, bit_2, &I2C_config);
		GPIO_pin_control_register(GPIO_B, bit_3, &I2C_config);
		GPIO_clear_pin(GPIO_B, bit_2);	// OFF
		GPIO_clear_pin(GPIO_B, bit_3);	// OFF
		GPIO_data_direction_pin(GPIO_B, GPIO_OUTPUT, bit_2); //SCL Reloj
		GPIO_data_direction_pin(GPIO_B, GPIO_OUTPUT, bit_3); //SDA Dato

		I2C0->C1 |= I2C_C1_IICEN_MASK;	// Habilita el mmodulo de I2C

		// Ayuda de otro equipo:
		//I2C0->SMB |= I2C_SMB_FACK_MASK;

		// Ayuda de otro equipo tampoco funciono:
		//uint32_t SCL = (system_clock)/(baud_rate * MULTIPLIER);
		//I2C0->F |= I2C_F_MULT(1);
		//I2C0->F |= I2C_F_ICR( SCL );
		//SCL = (system_clock / baud_rate * MULT);


		//** Para la generacion de Baude Rate = I2C clock speed (Hz) / Mult x SCL divider
		I2C0->F |= I2C_F_MULT(2);	// Valores no se porque
		I2C0->F |= I2C_F_ICR(0);	// Valores no se porque

	}
	if (I2C_1 == channel) {
		SIM->SCGC4 |= SIM_SCGC4_I2C1_MASK; /* Bit_7 = 0x80h*/
	}
	if (I2C_2 == channel) {
		SIM->SCGC1 |= SIM_SCGC1_I2C2_MASK; /* Bit_6 = 0x40h*/
	}
}

uint8_t I2C_busy(void) {
	uint8_t status = 0;
	/* EL bit_5 de BUSY está en: 0 Bus es idle
	 *				   			 1 Bus es Busy
	 */
	status = (I2C0->S & I2C_S_BUSY_MASK)>>I2C_S_BUSY_SHIFT;
	return (status);
}

void I2C_mst_or_slv_mode(master_or_slave_mode_t mst_or_slv) {
	if (mst_or_slv == Slave_mode) {
		I2C0->C1 &= ~(I2C_C1_MST_MASK);	// MST = 0, slave mode
	}
	if (mst_or_slv == Master_mode) {
		I2C0->C1 |= I2C_C1_MST_MASK;	// MST = 1, master mode
	}
}

void I2C_tx_rx_mode(transmit_mode_t tx_or_rx) {
	if (tx_or_rx == I2C_RX_mode) {
		I2C0->C1 &= ~(I2C_C1_TX_MASK); // TX = 0, receive mode
	}
	if (tx_or_rx == I2C_TX_mode) {
		I2C0->C1 |= I2C_C1_TX_MASK;	   // TX = 1, transmit mode
	}
}

void I2C_nack_or_ack(ack_or_nack_t TXAK) {
	if (TXAK == Acknowledge) {
		I2C0->C1 &= ~(I2C_C1_TXAK_MASK); // TXAK = 0, An  acknowledge is sent to the bus
	}
	if (TXAK == Nacknowledge) {
		I2C0->C1 |= I2C_C1_TXAK_MASK;	 // TXAK = 1, An Nacknowledge is sent to the bus
	}
}

void I2C_repeted_start(void) {
	I2C0->C1 |=I2C_C1_RSTA_MASK;	// Write 1 for generate a repeated Start
}

void I2C_write_byte(uint8_t data){
	I2C0->D = data;		// DATA de 8 bits listo para transimitir page. 1504
}

uint8_t I2C_read_byte(void) {
	uint8_t data = 0;
	data = I2C0->D;		// Read data from the I/O buffer
	return (data);
}

void I2C_wait(void) {
	while ((I2C0->S & I2C_S_IICIF_MASK) == FALSE)	// Indica que el bus esta ocupado
		;
	I2C0->S |= I2C_S_IICIF_MASK;		// Limpiamos bandera
}

uint8_t I2C_get_ack_or_nack(void) {
	uint8_t ack = 0;
	ack = I2C0->S & I2C_S_RXAK_MASK;	// RXAK = 0, Acknowledge was received
	if (ack == FALSE) {		// Return 0, if the acknowledge was received
		return (FALSE);
	}
	if (ack == TRUE) {		// Return 1, if the acknowledge was NOT received, (It is a Nacknowledge)
		return (TRUE);
	}
	return TRUE;	// Nunca llega aqui pero quita un Warning! :)
}

void I2C_start(void) {
	I2C_mst_or_slv_mode(Master_mode);	// Change from 0 to 1, START!!!
	I2C_tx_rx_mode(I2C_TX_mode);		// Change to transmitter mode
}

void I2C_stop(void) {
	I2C_mst_or_slv_mode(Slave_mode);	// Change from 1 to 0, STOP!
	I2C_tx_rx_mode(I2C_RX_mode);		// Change to receiver mode
}

void I2C_enable_interrupt(void) {
	I2C0->FLT |= I2C_FLT_SSIE_MASK;		// Habilitamos la ISR para detectar START
}										// y STOP en el bus de la I2C


