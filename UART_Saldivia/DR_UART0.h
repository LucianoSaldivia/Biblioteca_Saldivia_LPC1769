/*******************************************************************************************************************************//**
 *
 * @file		DR_UART0.h
 * @brief		Breve descripción del objetivo del Módulo
 * @date		28 de oct. de 2018
 * @author		Saldivia, Luciano
 *
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** MODULO
 **********************************************************************************************************************************/

#ifndef DR_UART0_H_
#define DR_UART0_H_

/***********************************************************************************************************************************
 *** INCLUDES GLOBALES
 **********************************************************************************************************************************/

#include "Regs_LPC176x.h"
#include "FW_PINSEL.h"
#include "DR__UARTS.h"
#include "DR_Tipos.h"

/***********************************************************************************************************************************
 *** DEFINES GLOBALES
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** MACROS GLOBALES
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** TIPO DE DATOS GLOBALES
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** VARIABLES GLOBALES
 **********************************************************************************************************************************/

extern volatile uint8_t U0_Buf_RX[U0_MAX_RX];
extern volatile uint8_t U0_Buf_TX[U0_MAX_TX];

extern volatile uint8_t U0_Index_In_Rx = 0;
extern volatile uint8_t U0_Index_Out_Rx = 0 ;

extern volatile uint8_t U0_Index_In_Tx = 0;
extern volatile uint8_t U0_Index_Out_Tx = 0 ;

extern volatile uint8_t U0_TxEnCurso;

/***********************************************************************************************************************************
 *** PROTOTIPOS DE FUNCIONES GLOBALES
 **********************************************************************************************************************************/

void UART0_Init( void );
void UART0_Inicializar( uint8_t PCLK, uint8_t BaudRate, uint8_t Data_Bits, uint8_t Parity, uint8_t Stop_Bits );

void UART0_StartTx( void );
void UART0_PopTx( void );
void UART0_PushRx( void );

#endif // DR_UART0_H_
