/*******************************************************************************************************************************//**
 *
 * @file		DR_UART0.c
 * @brief		Breve descripción del objetivo del Módulo
 * @date		28 de oct. de 2018
 * @author		Saldivia, Luciano
 *
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** INCLUDES
 **********************************************************************************************************************************/

#include "DR_UART0.h"

/***********************************************************************************************************************************
 *** DEFINES PRIVADOS AL MODULO
 **********************************************************************************************************************************/

#define MODEM               0
#define TX                  1
#define RX                  2
#define ERRORES             3

#define U0_BORRAR_FLAG_RX
#define U0_BORRAR_FLAG_TX

#define U0_REGISTRO_RX		U0RBR
#define U0_REGISTRO_TX		U0THR
#define U0_LSR				U0LSR

#define OE                  1
#define PE                  2
#define FE                  4
#define BI                  8

#define DATA_BITS_5         5
#define DATA_BITS_6         6
#define DATA_BITS_7         7
#define DATA_BITS_8         8

#define PARITY_NONE         0
#define PARITY_ODD          1
#define PARITY_EVEN         2
#define PARITY_FORCE_1      3
#define PARITY_FORCE_0      4

#define STOP_BITS_1         1
#define STOP_BITS_2         2


/***********************************************************************************************************************************
 *** MACROS PRIVADAS AL MODULO
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** TIPOS DE DATOS PRIVADOS AL MODULO
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** TABLAS PRIVADAS AL MODULO
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** VARIABLES GLOBALES PUBLICAS
 **********************************************************************************************************************************/

volatile uint8_t U0_Buf_RX[U0_MAX_RX];
volatile uint8_t U0_Buf_TX[U0_MAX_TX];

volatile uint8_t U0_Index_In_Rx = 0;
volatile uint8_t U0_Index_Out_Rx = 0 ;

volatile uint8_t U0_Index_In_Tx = 0;
volatile uint8_t U0_Index_Out_Tx = 0 ;

volatile uint8_t U0_TxEnCurso;   // Flag de TX en curso

/***********************************************************************************************************************************
 *** VARIABLES GLOBALES PRIVADAS AL MODULO
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** PROTOTIPO DE FUNCIONES PRIVADAS AL MODULO
 **********************************************************************************************************************************/

 /***********************************************************************************************************************************
 *** FUNCIONES PRIVADAS AL MODULO
 **********************************************************************************************************************************/

 /***********************************************************************************************************************************
 *** FUNCIONES GLOBALES AL MODULO
 **********************************************************************************************************************************/

void UART0_Init ( void ){
    //1.- Registro PCONP - bit 3 en 1 energiza la UART0
    PCONP |= ( 0x01 << 3 );

	//2.- Registro PCLKSEL0 - bits 6 y 7 en 0 seleccionan que el clk de la UART0 sea 25MHz
    PCLKSEL0 &= ~( 0x03 << 6 );

	//3.- Set-up: Largo de trama,Paridad, Stop, etc.
    U0LCR |= ( 0x03 << 0 );         // Bit 0:1. Largo de trama. 11 es 8 bits
    U0LCR &= ~( 0x01 << 2 );		// Bit 2. Stop bits. 0 es 1 bit
    U0LCR &= ~( 0x01 << 3 );		// Bit 3. Parity enable. 0 es desactivada
    U0LCR &= ~( 0x01 << 6 );		// Bit 6. Break Control. 0 es disabled

	//4.- Configuración del BaudRate.

	/*
	* 		 	   MULVAL					   PCLK [Hz]
	* Baud = --------------------	X	--------------------------
	*		  MULVAL + DIVADDVAL		 16 x (256 x DLM + DLL )
	*/

	// Setup para 115200 baudios

    // DLL y DLM
    U0LCR |= ( 0x01 << 7 );     // Bit 7. DLAB == 1 para acceder a registros DLM y DLL

    U0DLM = 0;                  // DLM = 0
    U0DLL = 12;                 // DLL = 12

    U0FDR = 0;
    U0FDR |= ( 0x0E << 4 );     // Bits 4:7 . MULVAL = 14
    U0FDR |= 0x05;              // Bits 0:3 . DIVADDVAL = 5

	//5.- Habilito las funciones especiales de los pines TX y RX
    //TXD0 : PIN ??	-> 	P0[2]
    SetPINSEL( P0, 2, PINSEL_FUNC1 );
    //RXD0 : PIN ??	-> 	P0[3]
    SetPINSEL( P0, 3, PINSEL_FUNC1 );

    //6.- Registro U0LCR, pongo DLAB(bit7) en 0
    U0LCR &= ~( 0x01 << 7 );

	//7. Habilito las interrupciones de RX y TX en la UART0 (Registro U0IER)
	U0IER = 0x03;

	//8. Habilito la interrupción de la UART0 en el NVIC (Registro ISER0)
    ISER0 |= ( 0x01 << 5 );
}

void UART0_Inicializar( uint8_t PCLK, uint8_t BaudRate, uint8_t Data_Bits, uint8_t Parity, uint8_t Stop_Bits ){
    //1.- Registro PCONP - bit 3 en 1 habilita la UART0
    PCONP |= ( 0x01 << 3 );

    //2.- Registro PCLKSEL0 - bits 6 y 7 seleccionan el PCLK de la UART0
    PCLKSEL0 &= ~( 0x03 << 6 ); // Bits 6 y 7 en 0, para "reiniciar" el registro
    switch( PCLK ){
        case PCLK_CCLK_1:       // PCLK = CCLK
            PCLKSEL0 |= ( 0x01 << 6 );
            break;

        case PCLK_CCLK_2:       // PCLK = CCLK / 2
            PCLKSEL0 |= ( 0x02 << 6 );
            break;

        case PCLK_CCLK_4:       // PCLK = CCLK / 4
            PCLKSEL0 &= ~( 0x03 << 6 );
            break;

        case PCLK_CCLK_8:       // PCLK = CCLK / 8
            PCLKSEL0 |= ( 0x03 << 6 );
            break;

        default:                // PCLK = CCLK / 4
            PCLKSEL0 &= ~( 0x03 << 6 );
            break;
    }

    //3.- Set-up: Largo de trama, Paridad, Stop, etc.

    switch( Data_Bits ){        // Bits 1:0 Largo del dato
        case DATA_BITS_5:
            U0LCR |= ( 0x00 << 0 );
            break;

        case DATA_BITS_6:
            U0LCR |= ( 0x01 << 0 );
            break;

        case DATA_BITS_7:
            U0LCR |= ( 0x02 << 0 );
            break;

        case DATA_BITS_8:
            U0LCR |= ( 0x03 << 0 );
            break;

        default:                // default => DATA_BITS_8
            U0LCR |= ( 0x03 << 0 );
            break;
    }

    switch( Stop_Bits ){        // Bit 2    <0> 1 StopBit, <1> 2 StopBits
        case STOP_BITS_1:
            U0LCR &= ~( 0x01 << 2 );
            break;

        case STOP_BITS_2:
            U0LCR |= ( 0x01 << 2 );
            break;

        default:                // default => STOP_BITS_1
            U0LCR &= ~( 0x01 << 2 );
            break;
    }

    switch( Parity ){           // Bit 3 PARITY_ENABLE, 4:5 PARITY_SELECT
        case PARITY_NONE:
            U0LCR &= ~( 0x01 << 3 );
            break;

        case PARITY_ODD:
            U0LCR |= ( 0x01 << 3 );
            U0LCR &= ~( 0x02 << 4 );
            break;

        case PARITY_EVEN:
            U0LCR |= ( 0x01 << 3 );
            U0LCR &= ~( 0x02 << 4 );
            U0LCR |= ( 0x01 << 4 );
            break;

        case PARITY_FORCE_1:
            U0LCR |= ( 0x01 << 3 );
            U0LCR &= ~( 0x02 << 4 );
            U0LCR |= ( 0x03 << 4 );
            break;

        case PARITY_FORCE_0:
            U0LCR |= ( 0x01 << 3 );
            U0LCR |= ( 0x02 << 4 );
            break;

        default:                // default => PARITY_NONE
            U0LCR &= ~( 0x01 << 3 );
            break;
    }

    U0LCR &= ~( 0x01 << 6 );	// Bit 6. Break Control. <0> es disabled (por default)
                                                      // <1> es enabled


    //4.- Configuración del BaudRate.

    /*
    * 		 	   MULVAL					   PCLK [Hz]
    * Baud = --------------------	X	--------------------------
    *		  MULVAL + DIVADDVAL		 16 x (256 x DLM + DLL )
    */

    // Setup para BaudRate ( DLM, DLL, DivAddVall y MulVal ) según una tabla son Error % Máximo de 0.160247 %

    U0LCR |= ( 0x01 << 7 );		// Bit 7. DLAB = 1 para acceder a registros DLM y DLL

    switch ( BaudRate ) {       // DLM, DLL, DivAddVal y MulVal tabulados
        case BAUD_RATE_600:
        /*
                                                            DLM     DLL     DAV     MULVAL
    600     120000000       600.000000      0.000000 %      48      212     0       1
             60000000       600.000000      0.000000 %      24      106     0       1
             30000000       600.000000      0.000000 %      12      53      0       1
             15000000       600.000000      0.000000 %      4       226     1       4
        */
            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x30;               // DLM = 48
                    U0DLL = 0xD4;               // DLL = 212

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 0
                    U0FDR |= ( 0x01 << 4 );     // MulVal = 1
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x18;               // DLM = 24
                    U0DLL = 0x6A;               // DLL = 106

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 0
                    U0FDR |= ( 0x01 << 4 );     // MulVal = 1
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x0C;               // DLM = 12
                    U0DLL = 0x35;               // DLL = 53

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 0
                    U0FDR |= ( 0x01 << 4 );     // MulVal = 1
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x04;               // DLM = 4
                    U0DLL = 0xE2;               // DLL = 226

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x0C;               // DLM = 12
                    U0DLL = 0x35;               // DLL = 53

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 0
                    U0FDR |= ( 0x01 << 4 );     // MulVal = 1
                    break;
            }
            break;

        case BAUD_RATE_1200:
        /*
                                                            DLM     DLL     DAV     MULVAL
    1200    120000000       1200.000000     0.000000 %      24      106     0       1
             60000000       1200.000000     0.000000 %      12      53      0       1
             30000000       1200.000000     0.000000 %      4       226     1       4
             15000000       1200.000000     0.000000 %      2       113     1       4
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x18;               // DLM = 24
                    U0DLL = 0x6A;               // DLL = 106

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 0
                    U0FDR |= ( 0x01 << 4 );     // MulVal = 1
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x0C;               // DLM = 12
                    U0DLL = 0x35;               // DLL = 53

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 0
                    U0FDR |= ( 0x01 << 4 );     // MulVal = 1
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x04;               // DLM = 4
                    U0DLL = 0xE2;               // DLL = 226

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x02;               // DLM = 2
                    U0DLL = 0x71;               // DLL = 113

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x04;               // DLM = 4
                    U0DLL = 0xE2;               // DLL = 226

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;
            }
            break;

        case BAUD_RATE_2400:
        /*
                                                            DLM     DLL     DAV     MULVAL
    2400    120000000       2400.000000     0.000000 %      12      53      0       1
             60000000       2400.000000     0.000000 %      4       226     1       4
             30000000       2400.000000     0.000000 %      2       113     1       4
             15000000       2400.153564     0.006399 %      1       23      2       5
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x0C;               // DLM = 12
                    U0DLL = 0x35;               // DLL = 53

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 0
                    U0FDR |= ( 0x01 << 4 );     // MulVal = 1
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x04;               // DLM = 4
                    U0DLL = 0xE2;               // DLL = 226

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x02;               // DLM = 2
                    U0DLL = 0x71;               // DLL = 113

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x01;               // DLM = 1
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x02;               // DLM = 2
                    U0DLL = 0x71;               // DLL = 113

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;
            }
            break;

        case BAUD_RATE_4800:
        /*
                                                            DLM     DLL     DAV     MULVAL
    4800    120000000       4800.000000     0.000000 %      4       226     1       4
             60000000       4800.000000     0.000000 %      2       113     1       4
             30000000       4800.307129     0.006399 %      1       23      2       5
             15000000       4800.977539     0.020365 %      0       179     1       11
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x04;               // DLM = 4
                    U0DLL = 0xE2;               // DLL = 226

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x02;               // DLM = 2
                    U0DLL = 0x71;               // DLL = 113

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x01;               // DLM = 1
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xB3;               // DLL = 179

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x01;               // DLM = 1
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;
            }
            break;

        case BAUD_RATE_9600:
        /*
                                                            DLM     DLL     DAV     MULVAL
    9600    120000000       9600.000000     0.000000 %      2       113     1       4
             60000000       9600.614258     0.006399 %      1       23      2       5
             30000000       9601.955078     0.020365 %      0       179     1       11
             15000000       9603.073242     0.032013 %      0       71      3       8
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x02;               // DLM = 2
                    U0DLL = 0x71;               // DLL = 113

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x01;               // DLM = 1
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xB3;               // DLL = 179

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x47;               // DLL = 71

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x03 << 0 );     // DivAddVal = 3
                    U0FDR |= ( 0x08 << 4 );     // MulVal = 8
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xB3;               // DLL = 179

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;
            }
            break;

        case BAUD_RATE_14400:
        /*
                                                            DLM     DLL     DAV     MULVAL
    14400   120000000       14400.418945    0.002909 %      1       81      6       11
             60000000       14400.919922    0.006388 %      0       217     1       5
             30000000       14400.921875    0.006402 %      0       93      2       5
             15000000       14406.027344    0.041857 %      0       47      5       13
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x01;               // DLM = 1
                    U0DLL = 0x51;               // DLL = 81

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x06 << 0 );     // DivAddVal = 6
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xD9;               // DLL = 217

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x5D;               // DLL = 93

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x2F;               // DLL = 47

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x5D;               // DLL = 93

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;
            }
            break;

        case BAUD_RATE_19200:
        /*
                                                            DLM     DLL     DAV     MULVAL
    19200   120000000       19201.228516    0.006399 %      1       23      2       5
             60000000       19203.910156    0.020365 %      0       179     1       11
             30000000       19206.146484    0.032013 %      0       71      3       8
             15000000       19188.595703    0.059397 %      0       38      2       7
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x01;               // DLM = 1
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xB3;               // DLL = 179

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x47;               // DLL = 71

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x03 << 0 );     // DivAddVal = 3
                    U0FDR |= ( 0x08 << 4 );     // MulVal = 8
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x26;               // DLL = 38

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x07 << 4 );     // MulVal = 7
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x47;               // DLL = 71

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x03 << 0 );     // DivAddVal = 3
                    U0FDR |= ( 0x08 << 4 );     // MulVal = 8
                    break;
            }
            break;

        case BAUD_RATE_28800:
        /*
                                                            DLM     DLL     DAV     MULVAL
    28800   120000000       28801.839844    0.006388 %      0       217     1       5
             60000000       28801.843750    0.006402 %      0       93      2       5
             30000000       28812.054688    0.041857 %      0       47      5       13
             15000000       28782.892578    0.059401 %      0       24      5       14
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xD9;               // DLL = 217

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x5D;               // DLL = 93

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x2F;               // DLL = 47

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x18;               // DLL = 24

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x2F;               // DLL = 47

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;
            }
            break;

        case BAUD_RATE_38400:
        /*
                                                            DLM     DLL     DAV     MULVAL
    38400   120000000       38407.820312    0.020365 %      0       179     1       11
             60000000       38412.292969    0.032013 %      0       71      3       8
             30000000       38377.191406    0.059397 %      0       38      2       7
             15000000       38377.191406    0.059397 %      0       19      2       7
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xB3;               // DLL = 179

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x47;               // DLL = 71

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x03 << 0 );     // DivAddVal = 3
                    U0FDR |= ( 0x08 << 4 );     // MulVal = 8
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x26;               // DLL = 38

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x07 << 4 );     // MulVal = 7
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x13;               // DLL = 19

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x07 << 4 );     // MulVal = 7
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x26;               // DLL = 38

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x07 << 4 );     // MulVal = 7
                    break;
            }
            break;

        case BAUD_RATE_56000:
        /*
                                                            DLM     DLL     DAV     MULVAL
    56000   120000000       56000.000000    0.000000 %      0       125     1       14
             60000000       56034.484375    0.061579 %      0       58      2       13
             30000000       56034.484375    0.061579 %      0       29      2       13
             15000000       56089.738281    0.160247 %      0       9       6       7
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x7D;               // DLL = 125

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x3A;               // DLL = 58

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x1D;               // DLL = 29

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x09;               // DLL = 9

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x06 << 0 );     // DivAddVal = 6
                    U0FDR |= ( 0x07 << 4 );     // MulVal = 7
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x1D;               // DLL = 29

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;
            }
            break;

        case BAUD_RATE_57600:
        /*
                                                            DLM     DLL     DAV     MULVAL
    57600   120000000       57603.687500    0.006402 %      0       93      2       5
             60000000       57624.109375    0.041857 %      0       47      5       13
             30000000       57565.785156    0.059401 %      0       24      5       14
             15000000       57565.785156    0.059401 %      0       12      5       14

        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x5D;               // DLL = 93

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x2F;               // DLL = 47

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x18;               // DLL = 24

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x0C;               // DLL = 12

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x18;               // DLL = 24

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;
            }
            break;

        case BAUD_RATE_115200:
        /*
                                                            DLM     DLL     DAV     MULVAL
    115200  120000000       115248.218750   0.041857 %      0       47      5       13
             60000000       115131.570312   0.059401 %      0       24      5       14
             30000000       115131.570312   0.059401 %      0       12      5       14
             15000000       115131.570312   0.059401 %      0       6       5       14
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x2F;               // DLL = 47

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0D << 4 );     // MulVal = 13
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x18;               // DLL = 24

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x0C;               // DLL = 12

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x06;               // DLL = 6

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x0C;               // DLL = 12

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;
            }
            break;

        case BAUD_RATE_128000:
        /*
                                                            DLM     DLL     DAV     MULVAL
    128000  120000000       128022.765625   0.017786 %      0       37      7       12
             60000000       128105.593750   0.082495 %      0       23      3       11
             30000000       127840.914062   0.124286 %      0       12      2       9
             15000000       127840.914062   0.124286 %      0       6       2       9
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x25;               // DLL = 37

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x07 << 0 );     // DivAddVal = 7
                    U0FDR |= ( 0x0C << 4 );     // MulVal = 12
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x03 << 0 );     // DivAddVal = 3
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x0C;               // DLL = 12

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x09 << 4 );     // MulVal = 9
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x06;               // DLL = 6

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x09 << 4 );     // MulVal = 9
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x0C;               // DLL = 12

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x09 << 4 );     // MulVal = 9
                    break;
            }
            break;

        case BAUD_RATE_230400:
        /*
                                                            DLM     DLL     DAV     MULVAL
    230400  120000000       230263.140625   0.059401 %      0       24      5       14
             60000000       230263.140625   0.059401 %      0       12      5       14
             30000000       230263.140625   0.059401 %      0       6       5       14
             15000000       230263.140625   0.059401 %      0       3       5       14
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x18;               // DLL = 24

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x0C;               // DLL = 12

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x06;               // DLL = 6

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x03;               // DLL = 3

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x06;               // DLL = 6

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x05 << 0 );     // DivAddVal = 5
                    U0FDR |= ( 0x0E << 4 );     // MulVal = 14
                    break;
            }
            break;

        case BAUD_RATE_256000:
        /*
                                                            DLM     DLL     DAV     MULVAL
    256000  120000000       256211.187500   0.082495 %      0       23      3       11
             60000000       255681.828125   0.124286 %      0       12      2       9
             30000000       255681.828125   0.124286 %      0       6       2       9
             15000000       255681.828125   0.124286 %      0       3       2       9
        */

            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x03 << 0 );     // DivAddVal = 3
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x0C;               // DLL = 12

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x09 << 4 );     // MulVal = 9
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x06;               // DLL = 6

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x09 << 4 );     // MulVal = 9
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x03;               // DLL = 3

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x09 << 4 );     // MulVal = 9
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x06;               // DLL = 6

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x09 << 4 );     // MulVal = 9
                    break;
            }
            break;

        default:                                // default => BAUD_RATE_9600
            switch( PCLK ){
                case PCLK_CCLK_1:
                    U0DLM = 0x02;               // DLM = 2
                    U0DLL = 0x71;               // DLL = 113

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x01 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x04 << 4 );     // MulVal = 4
                    break;

                case PCLK_CCLK_2:
                    U0DLM = 0x01;               // DLM = 1
                    U0DLL = 0x17;               // DLL = 23

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x02 << 0 );     // DivAddVal = 2
                    U0FDR |= ( 0x05 << 4 );     // MulVal = 5
                    break;

                case PCLK_CCLK_4:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xB3;               // DLL = 179

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;

                case PCLK_CCLK_8:
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0x47;               // DLL = 71

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x03 << 0 );     // DivAddVal = 3
                    U0FDR |= ( 0x08 << 4 );     // MulVal = 8
                    break;

                default:                        // default => PCLK = CCLK / 4
                    U0DLM = 0x00;               // DLM = 0
                    U0DLL = 0xB3;               // DLL = 179

                    U0FDR = 0;                  // Pongo en 0 para después hacer U0FDR |=
                    U0FDR |= ( 0x00 << 0 );     // DivAddVal = 1
                    U0FDR |= ( 0x0B << 4 );     // MulVal = 11
                    break;
            }
            break;
    }

    //5.- Habilito las funciones especiales de los pines TX y RX
    //TXD0 : PIN ??	-> 	P0[2]
    SetPINSEL( P0, 2, PINSEL_FUNC1 );
    //RXD0 : PIN ??	-> 	P0[3]
    SetPINSEL( P0, 3, PINSEL_FUNC1 );

    //6.- Bit 7. DLAB = 0 para acceder a registros RBR y THR
    U0LCR &= ~( 0x01 << 7 );

    //7. Habilito las interrupciones de RX y TX en la UART0 (Registro U0IER)
    U0IER = ( 0x00 << 0 );   // Sin Interrupciones
    U0IER |= ( 0x01 << 0 );  // Habilito interrupciones RX
    U0IER |= ( 0x02 << 0 );  // Habilito interrupciones TX

    //8. Habilito la interrupción de la UART0 en el NVIC (Registro ISER0)
    ISER0 |= ( 0x01 << 5 );

}

void UART0_IRQHandler ( void ){

    uint8_t iir, lsr, Interrupcion; /* EN CASO DE HACER ANALISIS DE ERRORES DECLARAMOS AnalisisError; */
    int16_t dato;

    do {
        iir = U0IIR;
        Interrupcion = (iir >> 1) & 3;

        // b2  b1
        //  0   0   Modem
        //  0   1   Tx
        //  1   0	Rx
        //  1   1   Error

        // THR EMPTY (Interrupción por TX)
        if ( Interrupcion == TX ) {
            U0_BORRAR_FLAG_TX;

            UART0_PopTx();
        }

        // Data Ready (Interrupción por RX)

        if ( Interrupcion == RX ) {
            U0_BORRAR_FLAG_RX;

            UART0_PushRx();	// Guardo el dato recibido en el BufferRx
        }

        /*  EN CASO DE HACER ANALISIS DE ERRORES
        if ( Interrupcion == ERRORES )
        {
            lsr = U0_LSR;
            AnalisisError = (lsr >> 1) & 0x0f;

            switch ( AnalisisError )
            {
                case OE:
                    // Avisar por OE
                    break;
                case PE:
                    // Avisar por PE
                    break;
                case FE:
                    // Avisar por FE
                    break;
                case BI:
                    // Avisar por BI
                    break;
            }
        }   */

    } while ( ! ( iir & 0x01 ) );	/* Me fijo si cuando entré a la ISR había otra
                                    int. pendiente de atención: b0=1 (ocurre únicamente
                                    si dentro del mismo espacio temporal llegan dos
                                    interrupciones a la vez) */
}

void UART0_StartTx( void ){
    if( U0_Index_In_Tx != U0_Index_Out_Tx ){
        U0_REGISTRO_TX = dato;	// Fuerzo la transmisión del primer dato
    }
}

void UART0_PopTx( void ){
    if( U0_Index_In_Tx != U0_Index_Out_Tx ){                // Si hay dato para transmitir
        U0_REGISTRO_TX = U0_Buf_TX[ U0_Index_Out_Tx ];    // Transmito el dato
        U0_Index_Out_Tx ++;                             // Muevo el índice de salida de Tx
        U0_Index_Out_Tx %= U0_MAX_TX;
    }
    else{                                           // Si no hay más datos a enviar, terminó
        U0_TxEnCurso = 0;                           // la transmisión, entonces limpio el flag
    }
}

void UART0_PushRx( void ){
    U0_Buf_RX[ U0_Index_In_Rx ] = U0_REGISTRO_RX;
    U0_Index_In_Rx ++;
    U0_Index_In_Rx %= U0_MAX_RX;
}

