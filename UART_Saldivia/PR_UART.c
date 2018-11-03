/*******************************************************************************************************************************//**
 *
 * @file		PR_UART.c
 * @brief		Descripcion del modulo
 * @date		28 de oct. de 2018
 * @author		Saldivia, Luciano
 *
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** INCLUDES
 **********************************************************************************************************************************/

#include <string.h>
#include "PR_UART.h"

/***********************************************************************************************************************************
 *** DEFINES PRIVADOS AL MODULO
 **********************************************************************************************************************************/

#define ESPERANDO_INICIO        0
#define ESPERANDO_SIGUIENTE     1

/***********************************************************************************************************************************
 *** MACROS PRIVADAS AL MODULO
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** TIPOS DE datoS PRIVADOS AL MODULO
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** TABLAS PRIVADAS AL MODULO
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** VARIABLES GLOBALES PUBLICAS
 **********************************************************************************************************************************/

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

/*  FUNCIONES PARA UART0    */

//!< Funcion para escribir un byte a UART0
void UART0_PushTx( uint8_t dato ){

    U0_Buf_TX[ U0_Index_In_Tx ] = dato;
    U0_Index_In_Tx ++;
    U0_Index_In_Tx %= U0_MAX_TX;

    if (U0_TxEnCurso == 0) {    // Si no había una TX en curso
        U0_TxEnCurso = 1;       // pongo una TX en curso y
        UART0_StartTx();        // fuerzo el inicio de la TX
    }
}

//!< Funcion para leer un byte desde UART0
int16_t UART0_PopRx( void ){

    int16_t dato = BUFFER_VACIO;

    if ( U0_Index_In_Rx != U0_Index_Out_Rx ){
        dato = U0_Buf_RX[ U0_Index_Out_Rx ];
        U0_Index_Out_Rx ++;
        U0_Index_Out_Rx %= U0_MAX_RX;
    }
    return dato;
}

//!< Funcion para Transmitir datos en crudo por UART0
int16_t UART0_Transmitir( const void *Mensaje, uint16_t Size ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){
        return BUFFER_VACIO;
    }

    if( Size >= STR_LEN( mensaje) ){
        return BUFFER_OVERSIZE;
    }

    if( Size >= U0_MAX_TX ){
        return BUFFER_OVERSIZE;
    }

    for( InxAux = 0; InxAux < Size; InxAux++){
        UART0_PushTx( mensaje[ InxAux ] );
    }

    return ENVIADO;
}

//!< Funcion para Transmitir un string UART_0
int16_t UART0_Transmitir_String( uint8_t *Mensaje ){

    uint32_t InxAux;

    if( mensaje[0] == (uint8_t) '\0' ){
        return BUFFER_VACIO;
    }

    for( InxAux = 0; mensaje[ InxAux ] != (uint8_t) '\0'; InxAux++){
        UART0_PushTx( mensaje[ InxAux ] );
    }
    UART0_PushTx( (uint8_t) '\0' );

    return ENVIADO;
}

//!< Funcion para recibir un string desde UART0
int16_t UART0_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max ){

    static uint32_t InxAux = 0;
    int16_t dato;

    dato = UART0_PopRx();

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    if( (uint8_t) dato != '\0' ){
        if( InxAux != Cant_Max - 1 ){
            Mensaje[InxAux] = (uint8_t) dato;
            InxAux ++;
            return RECIBIENDO_MENSAJE;
        }
        else{
            Mensaje[InxAux] = '\0';
            InxAux = 0;
            return MENSAJE_RECIBIDO_CORTADO;
        }
    }
    else{
        Mensaje[InxAux] = '\0';
        InxAux = 0;
        return MENSAJE_RECIBIDO_COMPLETO;
    }
}

//!< Funcion para Transmitir un mensaje como Char_Inicial + Mensaje + Char_Final por UART0
int16_t UART0_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){    // Si el mensaje está vacío
        return BUFFER_VACIO;
    }

    if( STR_LEN( mensaje ) > U0_MAX_TX - 2 ){   // Si mensaje es mas largo que UN_MAX_TX
        return BUFFER_OVERSIZE;
    }

    UART0_PushTx( Char_Inicial );   // Transmito Char_Inicial

    for( InxAux = 0; InxAux < STR_LEN( mensaje ); InxAux++){    // Transmito Mensaje
        UART0_PushTx( mensaje[ InxAux ] );
    }

    UART0_PushTx( Char_Final );     // Transmito Char_Final

    return ENVIADO;
}

//!< Funcion (Máquina de estados) para recibir un mensaje desde Char_Inicial hasta Char_Final desde UART0
int16_t UART0_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    static uint8_t Estado = ESPERANDO_INICIO, buf[ MAX_TRAMA_RECIBIDA ];
    static uint32_t InxAux = 0;
    int16_t dato;


    dato = UART0_PopRx( );

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    switch( Estado ){
        case ESPERANDO_INICIO:
            if( dato == Char_Inicial ){
                Estado = ESPERANDO_SIGUIENTE;
                return RECIBIENDO_MENSAJE;
            }
            else return RECIBI_BASURA;
            break;

        case ESPERANDO_SIGUIENTE:
            if( (uint8_t) dato != Char_Final ){
                if( InxAux != MAX_TRAMA_RECIBIDA - 1 ){
                    buf[InxAux] = (uint8_t) dato;
                    InxAux ++;
                    return RECIBIENDO_MENSAJE;
                }
                else{
                    buf[InxAux] = '\0';
                    Estado = ESPERANDO_INICIO;
                    InxAux = 0;
                    return MENSAJE_RECIBIDO_CORTADO;
                }
            }
            else{
                buf[InxAux] = '\0';
                Estado = ESPERANDO_INICIO;
                InxAux = 0;
                return MENSAJE_RECIBIDO_COMPLETO;
            }
            break;

        default:
            Estado = ESPERANDO_INICIO;
            return RECIBI_BASURA;
            break;
    }
}



/*  FUNCIONES PARA UART1    */

//!< Funcion para escribir un byte a UART1
void UART1_PushTx( uint8_t dato ){

    U1_Buf_TX[ U1_Index_In_Tx ] = dato;
    U1_Index_In_Tx ++;
    U1_Index_In_Tx %= U1_MAX_TX;

    if (U1_TxEnCurso == 0) {    // Si no había una TX en curso
        U1_TxEnCurso = 1;   	// pongo una TX en curso y
        UART1_StartTx();        // fuerzo el inicio de la TX
    }
}

//!< Funcion para leer un byte desde UART1
int16_t UART1_PopRx( void ){

    int16_t dato = BUFFER_VACIO;

    if ( UART1_InxInRx != U1_Index_Out_Rx ){
        dato = UART1_Buf_Rx[ U1_Index_Out_Rx ];
        U1_Index_Out_Rx ++;
        U1_Index_Out_Rx %= U1_MAX_RX;
    }
    return dato;
}

//!< Funcion para Transmitir datos en crudo por UART1
int16_t UART1_Transmitir( const void *Mensaje, uint16_t Size ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){
        return BUFFER_VACIO;
    }

    if( Size >= STR_LEN( mensaje) ){
        return BUFFER_OVERSIZE;
    }

    if( Size >= U1_MAX_TX ){
        return BUFFER_OVERSIZE;
    }

    for( InxAux = 0; InxAux < Size; InxAux++){
        UART1_PushTx( mensaje[ InxAux ] );
    }
}

//!< Funcion para Transmitir un string UART_1
int16_t UART1_Transmitir_String( uint8_t *Mensaje ){

    uint32_t InxAux;

    if( mensaje[0] == (uint8_t) '\0' ){
        return BUFFER_VACIO;
    }

    for( InxAux = 0; mensaje[ InxAux ] != (uint8_t) '\0'; InxAux++){
        UART1_PushTx( mensaje[ InxAux ] );
    }
    UART1_PushTx( (uint8_t) '\0' );

    return ENVIADO;
}

//!< Funcion para recibir un string desde UART1
int16_t UART1_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max ){

    static uint32_t InxAux = 0;
    int16_t dato;

    dato = UART1_PopRx();

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    if( (uint8_t) dato != '\0' ){
        if( InxAux != Cant_Max - 1 ){
            Mensaje[InxAux] = (uint8_t) dato;
            InxAux ++;
            return RECIBIENDO_MENSAJE;
        }
        else{
            Mensaje[InxAux] = '\0';
            InxAux = 0;
            return MENSAJE_RECIBIDO_CORTADO;
        }
    }
    else{
        Mensaje[InxAux] = '\0';
        InxAux = 0;
        return MENSAJE_RECIBIDO_COMPLETO;
    }
}

//!< Funcion para Transmitir un mensaje como Char_Inicial + Mensaje + Char_Final por UART1
int16_t UART1_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){    // Si el mensaje está vacío
        return BUFFER_VACIO;
    }

    if( STR_LEN( mensaje ) > U1_MAX_TX - 2 ){   // Si mensaje es mas largo que UN_MAX_TX
        return BUFFER_OVERSIZE;
    }

    UART1_PushTx( Char_Inicial );   // Transmito Char_Inicial

    for( InxAux = 0; InxAux < STR_LEN( mensaje ); InxAux++){    // Transmito Mensaje
        UART1_PushTx( mensaje[ InxAux ] );
    }

    UART1_PushTx( Char_Final );     // Transmito Char_Final

    return ENVIADO;
}

//!< Funcion (Máquina de estados) para recibir un mensaje desde Char_Inicial hasta Char_Final desde UART1
int16_t UART1_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    static uint8_t Estado = ESPERANDO_INICIO, buf[ MAX_TRAMA_RECIBIDA ];
    static uint32_t InxAux = 0;
    int16_t dato;


    dato = UART1_PopRx( );

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    switch( Estado ){
        case ESPERANDO_INICIO:
            if( dato == Char_Inicial ){
                Estado = ESPERANDO_SIGUIENTE;
                return RECIBIENDO_MENSAJE;
            }
            else return RECIBI_BASURA;
            break;

        case ESPERANDO_SIGUIENTE:
            if( (uint8_t) dato != Char_Final ){
                if( InxAux != MAX_TRAMA_RECIBIDA - 1 ){
                    buf[InxAux] = (uint8_t) dato;
                    InxAux ++;
                    return RECIBIENDO_MENSAJE;
                }
                else{
                    buf[InxAux] = '\0';
                    Estado = ESPERANDO_INICIO;
                    InxAux = 0;
                    return MENSAJE_RECIBIDO_CORTADO;
                }
            }
            else{
                buf[InxAux] = '\0';
                Estado = ESPERANDO_INICIO;
                InxAux = 0;
                return MENSAJE_RECIBIDO_COMPLETO;
            }
            break;

        default:
            Estado = ESPERANDO_INICIO;
            return RECIBI_BASURA;
            break;
    }
}



/*  FUNCIONES PARA UART2    */

//!< Funcion para escribir un byte a UART2
void UART2_PushTx( uint8_t dato ){

    U2_Buf_TX[ U2_Index_In_Tx ] = dato;
    U2_Index_In_Tx ++;
    U2_Index_In_Tx %= U2_MAX_TX;

    if (U2_TxEnCurso == 0) {    // Si no había una TX en curso
        U2_TxEnCurso = 1;       // pongo una TX en curso y
        UART2_StartTx();        // fuerzo el inicio de la TX
    }
}

//!< Funcion para leer un byte desde UART2
int16_t UART2_PopRx( void ){

    int16_t dato = BUFFER_VACIO;

    if ( UART2_InxInRx != U2_Index_Out_Rx ){
        dato = UART2_Buf_Rx[ U2_Index_Out_Rx ];
        U2_Index_Out_Rx ++;
        U2_Index_Out_Rx %= U2_MAX_RX;
    }
    return dato;
}

//!< Funcion para Transmitir datos en crudo por UART2
int16_t UART2_Transmitir( const void *Mensaje, uint16_t Size ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){
        return BUFFER_VACIO;
    }

    if( Size >= STR_LEN( mensaje) ){
        return BUFFER_OVERSIZE;
    }

    if( Size >= U2_MAX_TX ){
        return BUFFER_OVERSIZE;
    }

    for( InxAux = 0; InxAux < Size; InxAux++){
        UART2_PushTx( mensaje[ InxAux ] );
    }

    return ENVIADO;
}

//!< Funcion para Transmitir un string UART_2
int16_t UART2_Transmitir_String( uint8_t *Mensaje ){

    uint32_t InxAux;

    if( mensaje[0] == (uint8_t) '\0' ){
        return BUFFER_VACIO;
    }

    for( InxAux = 0; mensaje[ InxAux ] != (uint8_t) '\0'; InxAux++){
        UART2_PushTx( mensaje[ InxAux ] );
    }
    UART2_PushTx( (uint8_t) '\0' );

    return ENVIADO;
}

//!< Funcion para recibir un string desde UART2
int16_t UART2_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max ){

    static uint32_t InxAux = 0;
    int16_t dato;

    dato = UART2_PopRx();

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    if( (uint8_t) dato != '\0' ){
        if( InxAux != Cant_Max - 1 ){
            Mensaje[InxAux] = (uint8_t) dato;
            InxAux ++;
            return RECIBIENDO_MENSAJE;
        }
        else{
            Mensaje[InxAux] = '\0';
            InxAux = 0;
            return MENSAJE_RECIBIDO_CORTADO;
        }
    }
    else{
        Mensaje[InxAux] = '\0';
        InxAux = 0;
        return MENSAJE_RECIBIDO_COMPLETO;
    }
}

//!< Funcion para Transmitir un mensaje como Char_Inicial + Mensaje + Char_Final por UART2
int16_t UART2_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){    // Si el mensaje está vacío
        return BUFFER_VACIO;
    }

    if( STR_LEN( mensaje ) > U2_MAX_TX - 2 ){   // Si mensaje es mas largo que UN_MAX_TX
        return BUFFER_OVERSIZE;
    }

    UART2_PushTx( Char_Inicial );   // Transmito Char_Inicial

    for( InxAux = 0; InxAux < STR_LEN( mensaje ); InxAux++){    // Transmito Mensaje
        UART2_PushTx( mensaje[ InxAux ] );
    }

    UART2_PushTx( Char_Final );     // Transmito Char_Final

    return ENVIADO;
}

//!< Funcion (Máquina de estados) para recibir un mensaje desde Char_Inicial hasta Char_Final desde UART2
int16_t UART2_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    static uint8_t Estado = ESPERANDO_INICIO, buf[ MAX_TRAMA_RECIBIDA ];
    static uint32_t InxAux = 0;
    int16_t dato;


    dato = UART2_PopRx( );

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    switch( Estado ){
        case ESPERANDO_INICIO:
            if( dato == Char_Inicial ){
                Estado = ESPERANDO_SIGUIENTE;
                return RECIBIENDO_MENSAJE;
            }
            else return RECIBI_BASURA;
            break;

        case ESPERANDO_SIGUIENTE:
            if( (uint8_t) dato != Char_Final ){
                if( InxAux != MAX_TRAMA_RECIBIDA - 1 ){
                    buf[InxAux] = (uint8_t) dato;
                    InxAux ++;
                    return RECIBIENDO_MENSAJE;
                }
                else{
                    buf[InxAux] = '\0';
                    Estado = ESPERANDO_INICIO;
                    InxAux = 0;
                    return MENSAJE_RECIBIDO_CORTADO;
                }
            }
            else{
                buf[InxAux] = '\0';
                Estado = ESPERANDO_INICIO;
                InxAux = 0;
                return MENSAJE_RECIBIDO_COMPLETO;
            }
            break;

        default:
            Estado = ESPERANDO_INICIO;
            return RECIBI_BASURA;
            break;
    }
}



/*  FUNCIONES PARA UART3    */

//!< Funcion para escribir un byte a UART3
void UART3_PushTx( uint8_t dato ){

    U3_Buf_TX[ U3_Index_In_Tx ] = dato;
    U3_Index_In_Tx ++;
    U3_Index_In_Tx %= U3_MAX_TX;

    if (U3_TxEnCurso == 0) {    // Si no había una TX en curso
        U3_TxEnCurso = 1;       // pongo una TX en curso y
        UART3_StartTx();        // fuerzo el inicio de la TX
    }
}

//!< Funcion para leer un byte desde UART3
int16_t UART3_PopRx( void ){

    int16_t dato = BUFFER_VACIO;

    if ( UART3_InxInRx != U3_Index_Out_Rx ){
        dato = UART3_Buf_Rx[ U3_Index_Out_Rx ];
        U3_Index_Out_Rx ++;
        U3_Index_Out_Rx %= U3_MAX_RX;
    }
    return dato;
}

//!< Funcion para Transmitir datos en crudo por UART3
int16_t UART3_Transmitir( const void *Mensaje, uint16_t Size ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){
        return BUFFER_VACIO;
    }

    if( Size >= STR_LEN( mensaje) ){
        return BUFFER_OVERSIZE;
    }

    if( Size >= U3_MAX_TX ){
        return BUFFER_OVERSIZE;
    }

    for( InxAux = 0; InxAux < Size; InxAux++){
        UART3_PushTx( mensaje[ InxAux ] );
    }

    return ENVIADO;
}

//!< Funcion para Transmitir un string UART_3
int16_t UART3_Transmitir_String( uint8_t *Mensaje ){

    uint32_t InxAux;

    if( mensaje[0] == (uint8_t) '\0' ){
        return BUFFER_VACIO;
    }

    for( InxAux = 0; mensaje[ InxAux ] != (uint8_t) '\0'; InxAux++){
        UART3_PushTx( mensaje[ InxAux ] );
    }
    UART3_PushTx( (uint8_t) '\0' );

    return ENVIADO;
}

//!< Funcion para recibir un string desde UART3
int16_t UART3_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max ){

    static uint32_t InxAux = 0;
    int16_t dato;

    dato = UART3_PopRx();

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    if( (uint8_t) dato != '\0' ){
        if( InxAux != Cant_Max - 1 ){
            Mensaje[InxAux] = (uint8_t) dato;
            InxAux ++;
            return RECIBIENDO_MENSAJE;
        }
        else{
            Mensaje[InxAux] = '\0';
            InxAux = 0;
            return MENSAJE_RECIBIDO_CORTADO;
        }
    }
    else{
        Mensaje[InxAux] = '\0';
        InxAux = 0;
        return MENSAJE_RECIBIDO_COMPLETO;
    }
}

//!< Funcion para Transmitir un mensaje como Char_Inicial + Mensaje + Char_Final por UART3
int16_t UART3_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    uint8_t *mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){    // Si el mensaje está vacío
        return BUFFER_VACIO;
    }

    if( STR_LEN( mensaje ) > U3_MAX_TX - 2 ){   // Si mensaje es mas largo que UN_MAX_TX
        return BUFFER_OVERSIZE;
    }

    UART3_PushTx( Char_Inicial );   // Transmito Char_Inicial

    for( InxAux = 0; InxAux < STR_LEN( mensaje ); InxAux++){    // Transmito Mensaje
        UART3_PushTx( mensaje[ InxAux ] );
    }

    UART3_PushTx( Char_Final );     // Transmito Char_Final

    return ENVIADO;
}

//!< Funcion (Máquina de estados) para recibir un mensaje desde Char_Inicial hasta Char_Final desde UART3
int16_t UART3_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    static uint8_t Estado = ESPERANDO_INICIO, buf[ MAX_TRAMA_RECIBIDA ];
    static uint32_t InxAux = 0;
    int16_t dato;


    dato = UART3_PopRx( );

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    switch( Estado ){
        case ESPERANDO_INICIO:
            if( dato == Char_Inicial ){
                Estado = ESPERANDO_SIGUIENTE;
                return RECIBIENDO_MENSAJE;
            }
            else return RECIBI_BASURA;
            break;

        case ESPERANDO_SIGUIENTE:
            if( (uint8_t) dato != Char_Final ){
                if( InxAux != MAX_TRAMA_RECIBIDA - 1 ){
                    buf[InxAux] = (uint8_t) dato;
                    InxAux ++;
                    return RECIBIENDO_MENSAJE;
                }
                else{
                    buf[InxAux] = '\0';
                    Estado = ESPERANDO_INICIO;
                    InxAux = 0;
                    return MENSAJE_RECIBIDO_CORTADO;
                }
            }
            else{
                buf[InxAux] = '\0';
                Estado = ESPERANDO_INICIO;
                InxAux = 0;
                return MENSAJE_RECIBIDO_COMPLETO;
            }
            break;

        default:
            Estado = ESPERANDO_INICIO;
            return RECIBI_BASURA;
            break;
    }
}




/* FUNCIONES GENERICAS PARA CUALQUIER UART */

int16_t PushTx( uint8_t UART_N, uint8_t dato ){

    switch( UART_N ){
        case UART0:
            UART0_PushTx( dato );
            break;

        case UART0:
            UART1_PushTx( dato );
            break;

        case UART0:
            UART2_PushTx( dato );
            break;

        case UART0:
            UART3_PushTx( dato );
            break;

        default:
            return UART_INEXISTENTE;
    }
    return ENVIADO;
}

//!< Funcion para recibir un byte en crudo de UART_N
int16_t PopRx( uint8_t UART_N ){

   int16_t dato;

   switch( UART_N ){
       case UART0:
           dato = UART0_PopRX( void );
           break;

       case UART1:
           dato = UART1_PopRX( void );
           break;

       case UART2:
           dato = UART2_PopRX( void );
           break;

       case UART3:
           dato = UART3_PopRX( void );
           break;

       default:
           return UART_INEXISTENTE;
   }
   return dato;
}

//!< Funcion para Transmitir datos en crudo por UART_N
int16_t Transmitir( uint8_t UART_N, const void *Mensaje, uint16_t Size ){

    uint8_t * mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){
        return BUFFER_VACIO;
    }

    if( Size >= STR_LEN( mensaje) ){
        return BUFFER_OVERSIZE;
    }

    switch( UART_N ){
        case UART0:
            if( STR_LEN( mensaje ) >= U0_MAX_TX ){
                return BUFFER_OVERSIZE;
            }
            UART0_Transmitir( Mensaje, Size );
            break;

        case UART1:
            if( STR_LEN( mensaje ) >= U1_MAX_TX ){
                return BUFFER_OVERSIZE;
            }
            UART1_Transmitir( Mensaje, Size );
            break;

        case UART2:
            if( STR_LEN( mensaje ) >= U2_MAX_TX ){
                return BUFFER_OVERSIZE;
            }
            UART2_Transmitir( Mensaje, Size );
            break;

        case UART3:
            if( STR_LEN( mensaje ) >= U3_MAX_TX ){
                return BUFFER_OVERSIZE;
            }
            UART3_Transmitir( Mensaje, Size );
            break;

        default:
            return UART_INEXISTENTE;
    }

    return ENVIADO;
}

//!< Funcion para Transmitir un string UART_N
int16_t Transmitir_String( uint8_t UART_N, const void *Mensaje ){

    uint8_t *mensaje = (uint8_t *) Mensaje;

    if( mensaje[0] == (uint8_t) '\0' ){
        return BUFFER_VACIO;
    }

    switch( UART_N ){
        case UART0:
            UART0_Transmitir_String( mensaje );
            break;

        case UART1:
            UART1_Transmitir_String( mensaje );
            break;

        case UART2:
            UART2_Transmitir_String( mensaje );
            break;

        case UART3:
            UART3_Transmitir_String( mensaje );
            break;

        default:
            return UART_INEXISTENTE;
    }

    return ENVIADO;
}

//!< Funcion para recibir un string desde UARTN
int16_t Recibir_String( uint8_t UART_N, uint8_t *Mensaje, uint32_t Cant_Max ){

    static uint32_t InxAux = 0;
    int16_t dato;


    dato = PopRx( UART_N );

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    if( dato == UART_INEXISTENTE ){
        return UART_INEXISTENTE;
    }

    if( (uint8_t) dato != '\0' ){
        if( InxAux != Cant_Max - 1 ){
            Mensaje[InxAux] = (uint8_t) dato;
            InxAux ++;
            return RECIBIENDO_MENSAJE;
        }
        else{
            Mensaje[InxAux] = '\0';
            InxAux = 0;
            return MENSAJE_RECIBIDO_CORTADO;
        }
    }
    else{
        Mensaje[InxAux] = '\0';
        InxAux = 0;
        return MENSAJE_RECIBIDO_COMPLETO;
    }
}

//!< Funcion para Transmitir un mensaje como Char_Inicial + Mensaje + Char_Final por UARTN
int16_t Transmitir_Mensaje( uint8_t UART_N, const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    uint8_t * mensaje = (uint8_t *) Mensaje;
    uint32_t InxAux = 0;

    if( mensaje[0] == '\0' ){
        return BUFFER_VACIO;
    }

    switch( UART_N ){
        case UART0:
            if( STR_LEN( mensaje ) > U0_MAX_TX - 2 ){
                return BUFFER_OVERSIZE;
            }
            UART0_Transmitir_Mensaje( Mensaje, Char_Inicial, Char_Final );
            break;

        case UART1:
            if( STR_LEN( mensaje ) > U1_MAX_TX - 2 ){
                return BUFFER_OVERSIZE;
            }
            UART1_Transmitir_Mensaje( Mensaje, Char_Inicial, Char_Final );
            break;

        case UART2:
            if( STR_LEN( mensaje ) > U2_MAX_TX - 2 ){
                return BUFFER_OVERSIZE;
            }
            UART2_Transmitir_Mensaje( Mensaje, Char_Inicial, Char_Final );
            break;

        case UART3:
            if( STR_LEN( mensaje ) > U3_MAX_TX - 2 ){
                return BUFFER_OVERSIZE;
            }
            UART3_Transmitir_Mensaje( Mensaje, Char_Inicial, Char_Final );
            break;

        default:
            return UART_INEXISTENTE;
    }

    return ENVIADO;
}

//!< Funcion (Máquina de estados) para recibir un mensaje desde Char_Inicial hasta Char_Final desde UARTN
int16_t Recibir_Mensaje( uint8_t UART_N, const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final ){

    static uint8_t Estado = ESPERANDO_INICIO, buf[ MAX_TRAMA_RECIBIDA ];
    static uint32_t InxAux = 0;
    int16_t dato;


    dato = PopRx( UART_N );

    if( dato == BUFFER_VACIO ){
        return BUFFER_VACIO;
    }

    if( dato == UART_INEXISTENTE ){
        return UART_INEXISTENTE;
    }

    switch( Estado ){
        case ESPERANDO_INICIO:
            if( dato == Char_Inicial ){
                Estado = ESPERANDO_SIGUIENTE;
                return RECIBIENDO_MENSAJE;
            }
            else return RECIBI_BASURA;
            break;

        case ESPERANDO_SIGUIENTE:
            if( (uint8_t) dato != Char_Final ){
                if( InxAux != MAX_TRAMA_RECIBIDA - 1 ){
                    buf[InxAux] = (uint8_t) dato;
                    InxAux ++;
                    return RECIBIENDO_MENSAJE;
                }
                else{
                    buf[InxAux] = '\0';
                    Estado = ESPERANDO_INICIO;
                    InxAux = 0;
                    return MENSAJE_RECIBIDO_CORTADO;
                }
            }
            else{
                buf[InxAux] = '\0';
                Estado = ESPERANDO_INICIO;
                InxAux = 0;
                return MENSAJE_RECIBIDO_COMPLETO;
            }
            break;

        default:
            Estado = ESPERANDO_INICIO;
            return RECIBI_BASURA;
            break;
    }
}


UART_INIT(  )








/********************************************************/
/********************************************************/
/********************************************************/

