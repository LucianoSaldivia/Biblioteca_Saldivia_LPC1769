TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    DR_UART0.c \
    PR_UART.c

HEADERS += \
    DR_UART0.h \
    PR_UART.h
