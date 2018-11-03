TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    DR_UART0.c \
    PR_UART.c \
    DR_UART1.c \
    DR_UART2.c \
    DR_UART3.c

HEADERS += \
    DR_UART0.h \
    PR_UART.h \
    DR_UART1.h \
    DR_UART3.h \
    DR_UART2.h \
    DR__UARTS.h
