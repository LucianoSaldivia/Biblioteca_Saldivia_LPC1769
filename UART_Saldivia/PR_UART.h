/*******************************************************************************************************************************//**
 *
 * @file		PR_UART.h
 * @brief		Breve descripción del objetivo del Módulo
 * @date		28 de oct. de 2018
 * @author		Saldivia, Luciano
 *
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** MODULO
 **********************************************************************************************************************************/

#ifndef PR_UART_H_
#define PR_UART_H_

/***********************************************************************************************************************************
 *** INCLUDES GLOBALES
 **********************************************************************************************************************************/

#include "Regs_LPC176x.h"
#include "DR_tipos.h"
#include "DR_UART0.h"
#include "DR_UART1.h"
#include "DR_UART2.h"
#include "DR_UART3.h"


/***********************************************************************************************************************************
 *** DEFINES GLOBALES
 **********************************************************************************************************************************/

#define UART0                       0
#define UART1                       1
#define UART2                       2
#define UART3                       3

#define MENSAJE_RECIBIDO_COMPLETO   2
#define MENSAJE_RECIBIDO_CORTADO    1
#define ENVIADO                     0
#define RECIBIENDO_MENSAJE          0
#define RECIBI_BASURA               -1

#define BUFFER_VACIO                -1
#define BUFFER_OVERSIZE             -2
#define UART_INEXISTENTE            -5

#define MAX_TRAMA_RECIBIDA          50

/***********************************************************************************************************************************
 *** MACROS GLOBALES
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** TIPO DE DATOS GLOBALES
 **********************************************************************************************************************************/

/***********************************************************************************************************************************
 *** VARIABLES GLOBALES
 **********************************************************************************************************************************/
// extern tipo nombreVariable;

/***********************************************************************************************************************************
 *** PROTOTIPOS DE FUNCIONES GLOBALES
 **********************************************************************************************************************************/

/*  FUNCIONES PARA UART0    */

void UART0_PushTx( uint8_t dato );
    /* Descripción:
     *      Pasa Dato al buffer de transmisión, desde el cual se enviará "automáticamente"
     */
int16_t UART0_PopRx( void );
    /* Descripción:
     *      Retorna el primer byte que haya para leer
     *      En caso de error, retorna:
     *          BUFFER_VACIO  =>  No hay dato para leer
     */
int16_t UART0_Transmitir( const void *Mensaje, uint16_t Size );
    /* Descripción:
     *      Hace PushTx Size veces desde Mensaje[0]
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          BUFFER_OVERSIZE =>  Size es más grande que el tamaño del buffer de Transmisión
     *          BUFFER_OVERSIZE =>  Size es más grande que Mensaje
     *          ENVIADO         =>  Size bytes fueron enviados al buffer de transmisión
     */
int16_t UART0_Transmitir_String( uint8_t *Mensaje );
    /* Descripción:
     *      Hace PushTx hasta el primer '\0' en Mensaje
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          ENVIADO         =>  El string completo fue enviado al buffer de transmisión
     */
int16_t UART0_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace.
     *      No se realizarán mas de Cant_Max PopRx.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, pero no recibió un '\0'
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron Cant_Max bytes, y nunca se recibió un '\0'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un String Completo.
     */
int16_t UART0_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Transmite el Mensaje completo, es decir: 'Char_Inicial' + "Mensaje" + 'Char_Final'.
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  Mensaje está vacío
     *          BUFFER_OVERSIZE =>  El mensaje completo es más largo que el tamaño del buffer de transmisión
     *          ENVIADO         =>  El mensaje completo fue enviado al buffer de transmisión
     */
int16_t UART0_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace, excluyendo Char_Inicial y Char_Final.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBI_BASURA               =>  Se recibió algo antes de Char_Inicial
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, está armando el String
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron MAX_TRAMA_RECIBIDA bytes, y nunca se recibió un 'Char_Final'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un Mensaje completo y ya tenemos el String listo.
     */


/*  FUNCIONES PARA UART1    */

void UART1_PushTx( uint8_t dato );
    /* Descripción:
     *      Pasa Dato al buffer de transmisión, desde el cual se enviará "automáticamente"
     */
int16_t UART1_PopRx( void );
    /* Descripción:
     *      Retorna el primer byte que haya para leer
     *      En caso de error, retorna:
     *          BUFFER_VACIO  =>  No hay dato para leer
     */
int16_t UART1_Transmitir( const void *Mensaje, uint16_t Size );
    /* Descripción:
     *      Hace PushTx Size veces desde Mensaje[0]
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          BUFFER_OVERSIZE =>  Size es más grande que el tamaño del buffer de Transmisión
     *          BUFFER_OVERSIZE =>  Size es más grande que Mensaje
     *          ENVIADO         =>  Size bytes fueron enviados al buffer de transmisión
     */
int16_t UART1_Transmitir_String( uint8_t *Mensaje );
    /* Descripción:
     *      Hace PushTx hasta el primer '\0' en Mensaje
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          ENVIADO         =>  El string completo fue enviado al buffer de transmisión
     */
int16_t UART1_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace.
     *      No se realizarán mas de Cant_Max PopRx.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, pero no recibió un '\0'
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron Cant_Max bytes, y nunca se recibió un '\0'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un String Completo.
     */
int16_t UART1_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Transmite el Mensaje completo, es decir: 'Char_Inicial' + "Mensaje" + 'Char_Final'.
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  Mensaje está vacío
     *          BUFFER_OVERSIZE =>  El mensaje completo es más largo que el tamaño del buffer de transmisión
     *          ENVIADO         =>  El mensaje completo fue enviado al buffer de transmisión
     */
int16_t UART1_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace, excluyendo Char_Inicial y Char_Final.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBI_BASURA               =>  Se recibió algo antes de Char_Inicial
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, está armando el String
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron MAX_TRAMA_RECIBIDA bytes, y nunca se recibió un 'Char_Final'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un Mensaje completo y ya tenemos el String listo.
     */


/*  FUNCIONES PARA UART2    */

void UART2_PushTx( uint8_t dato );
    /* Descripción:
     *      Pasa Dato al buffer de transmisión, desde el cual se enviará "automáticamente"
     */
int16_t UART2_PopRx( void );
    /* Descripción:
     *      Retorna el primer byte que haya para leer
     *      En caso de error, retorna:
     *          BUFFER_VACIO  =>  No hay dato para leer
     */
int16_t UART2_Transmitir( const void *Mensaje, uint16_t Size );
    /* Descripción:
     *      Hace PushTx Size veces desde Mensaje[0]
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          BUFFER_OVERSIZE =>  Size es más grande que el tamaño del buffer de Transmisión
     *          BUFFER_OVERSIZE =>  Size es más grande que Mensaje
     *          ENVIADO         =>  Size bytes fueron enviados al buffer de transmisión
     */
int16_t UART2_Transmitir_String( uint8_t *Mensaje );
    /* Descripción:
     *      Hace PushTx hasta el primer '\0' en Mensaje
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          ENVIADO         =>  El string completo fue enviado al buffer de transmisión
     */
int16_t UART2_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace.
     *      No se realizarán mas de Cant_Max PopRx.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, pero no recibió un '\0'
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron Cant_Max bytes, y nunca se recibió un '\0'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un String Completo.
     */
int16_t UART2_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Transmite el Mensaje completo, es decir: 'Char_Inicial' + "Mensaje" + 'Char_Final'.
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  Mensaje está vacío
     *          BUFFER_OVERSIZE =>  El mensaje completo es más largo que el tamaño del buffer de transmisión
     *          ENVIADO         =>  El mensaje completo fue enviado al buffer de transmisión
     */
int16_t UART2_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace, excluyendo Char_Inicial y Char_Final.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBI_BASURA               =>  Se recibió algo antes de Char_Inicial
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, está armando el String
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron MAX_TRAMA_RECIBIDA bytes, y nunca se recibió un 'Char_Final'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un Mensaje completo y ya tenemos el String listo.
     */


/*  FUNCIONES PARA UART3    */

void UART3_PushTx( uint8_t dato );
    /* Descripción:
     *      Pasa Dato al buffer de transmisión, desde el cual se enviará "automáticamente"
     */
int16_t UART3_PopRx( void );
    /* Descripción:
     *      Retorna el primer byte que haya para leer
     *      En caso de error, retorna:
     *          BUFFER_VACIO  =>  No hay dato para leer
     */
int16_t UART3_Transmitir( const void *Mensaje, uint16_t Size );
    /* Descripción:
     *      Hace PushTx Size veces desde Mensaje[0]
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          BUFFER_OVERSIZE =>  Size es más grande que el tamaño del buffer de Transmisión
     *          BUFFER_OVERSIZE =>  Size es más grande que Mensaje
     *          ENVIADO         =>  Size bytes fueron enviados al buffer de transmisión
     */
int16_t UART3_Transmitir_String( uint8_t *Mensaje );
    /* Descripción:
     *      Hace PushTx hasta el primer '\0' en Mensaje
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  No hay dato para transmitir en Mensaje
     *          ENVIADO         =>  El string completo fue enviado al buffer de transmisión
     */
int16_t UART3_Recibir_String( uint8_t *Mensaje, uint32_t Cant_Max );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace.
     *      No se realizarán mas de Cant_Max PopRx.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, pero no recibió un '\0'
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron Cant_Max bytes, y nunca se recibió un '\0'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un String Completo.
     */
int16_t UART3_Transmitir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Transmite el Mensaje completo, es decir: 'Char_Inicial' + "Mensaje" + 'Char_Final'.
     *      Valores de retorno:
     *          BUFFER_VACIO    =>  Mensaje está vacío
     *          BUFFER_OVERSIZE =>  El mensaje completo es más largo que el tamaño del buffer de transmisión
     *          ENVIADO         =>  El mensaje completo fue enviado al buffer de transmisión
     */
int16_t UART3_Recibir_Mensaje( const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx que hace, excluyendo Char_Inicial y Char_Final.
     *      Valores de retorno:
     *          BUFFER_VACIO                =>  No hay dato para leer
     *          RECIBI_BASURA               =>  Se recibió algo antes de Char_Inicial
     *          RECIBIENDO_MENSAJE          =>  Hizo un PopRx exitoso, está armando el String
     *          MENSAJE_RECIBIDO_CORTADO    =>  Se recibieron MAX_TRAMA_RECIBIDA bytes, y nunca se recibió un 'Char_Final'
     *          MENSAJE_RECIBIDO_COMPLETO   =>  Se recibió un Mensaje completo y ya tenemos el String listo.
     */


/* FUNCIONES GENERICAS PARA CUALQUIER UART */

int16_t PushTx( uint8_t UART_N, uint8_t dato );
    /* Descripción:
     *      Hace UART_N_PushTx( dato )
     *      Valores de retorno:
     *          UART_INEXISTENTE    =>  La UART_N elegida no existe {0, 1, 2, 3}
     *          ENVIADO             =>  Dato enviado al buffer UART_N de transmision
     */
int16_t PopRx( uint8_t UART_N );
    /* Descripción:
     *      Retorna el primer byte que haya para leer en UART_N
     *      En caso de error, retorna lo mismo que UART_N_PopRx o :
     *          UART_INEXISTENTE    =>  La UART_N elegida no existe {0, 1, 2, 3}
     */
int16_t Transmitir( uint8_t UART_N, const void *Mensaje, uint16_t Size );
    /* Descripción:
     *      Hace UART_N_Transmitir( Mensaje, Size )
     *      Valores de retorno de UART_N_Transmitir o :
     *          UART_INEXISTENTE    =>  La UART_N elegida no existe {0, 1, 2, 3}
     */
int16_t Transmitir_String( uint8_t UART_N, const void *Mensaje );
    /* Descripción:
     *      Hace UART_N_Transmitir_String( (uint8_t *) Mensaje )
     *      Valores de retorno de UART_N_Transmitir_String o :
     *          UART_INEXISTENTE    =>  La UART_N elegida no existe {0, 1, 2, 3}
     */
int16_t Recibir_String( uint8_t UART_N, uint8_t *Mensaje, uint32_t Cant_Max );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx( UART_N ) que hace.
     *      No se realizarán mas de Cant_Max PopRx.
     *      Valores de retorno de UART_N_Recibir_String o :
     *          UART_INEXISTENTE    =>  La UART_N elegida no existe {0, 1, 2, 3}
     */
int16_t Transmitir_Mensaje( uint8_t UART_N, const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Hace UART_N_Transmitir_Mensaje( Mensaje, Char_Inicial, Char_Final )
     *      Valores de retorno de UART_N_Transmitir_Mensaje o :
     *          UART_INEXISTENTE    =>  La UART_N elegida no existe {0, 1, 2, 3}
     */
int16_t Recibir_Mensaje( uint8_t UART_N, const void *Mensaje, uint8_t Char_Inicial, uint8_t Char_Final );
    /* Descripción:
     *      Arma un String que guarda en Mensaje, con los PopRx( UART_N ) que hace, excluyendo Char_Inicial y Char_Final.
     *      Valores de retorno de UART_N_Recibir_Mensaje o :
     *          UART_INEXISTENTE    =>  La UART_N elegida no existe {0, 1, 2, 3}
     */

#endif /* PRIMITIVAS_INC_PR_UART_H_ */
