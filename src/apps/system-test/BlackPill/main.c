/*!
 * \file      main.c
 *
 * \brief     Ping-Pong implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <string.h>
#include "board.h"
#include "gpio.h"
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "uart.h"
#include "timer.h"
#include "rtc-board.h"

#include "stm32f401xe.h"

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;

extern Uart_t Uart2;

Gpio_t interrupt;

static void PB8_handler( void* context ) {
    GpioToggle( &Led2 );
}

static void UartHartbeat(void){

    static char serial_character = 'A';

    UartPutChar( &Uart2, serial_character);
    if (serial_character < 'z') {
        serial_character++;
    } else {
        serial_character = 'A';
    }

    if (serial_character == 'Z') {
        UartPutChar(&Uart2, '\n');
        UartPutChar(&Uart2, '\r');
    }

}

static uint8_t HalfByteBin2Ascii(uint8_t hb) {
    uint8_t ch;
    switch(hb) {
        case 0: ch = '0'; break;
        case 1: ch = '1'; break;
        case 2: ch = '2'; break;
        case 3: ch = '3'; break;
        case 4: ch = '4'; break;
        case 5: ch = '5'; break;
        case 6: ch = '6'; break;
        case 7: ch = '7'; break;
        case 8: ch = '8'; break;
        case 9: ch = '9'; break;
        case 10: ch = 'A'; break;
        case 11: ch = 'B'; break;
        case 12: ch = 'C'; break;
        case 13: ch = 'D'; break;
        case 14: ch = 'E'; break;
        case 15: ch = 'F'; break;
        default: ch = 'Z'; break;
    }
    return ch;
}

static void WordBin2Ascii(uint32_t word, uint8_t* buff) {
    buff[7] = HalfByteBin2Ascii(word & 0x0000000F);
    buff[6] = HalfByteBin2Ascii((word & 0x000000F0) >> 4);
    buff[5] = HalfByteBin2Ascii((word & 0x00000F00) >> 8);
    buff[4] = HalfByteBin2Ascii((word & 0x0000F000) >> 12);
    buff[3] = HalfByteBin2Ascii((word & 0x000F0000) >> 16);
    buff[2] = HalfByteBin2Ascii((word & 0x00F00000) >> 20);
    buff[1] = HalfByteBin2Ascii((word & 0x0F000000) >> 24);
    buff[0] = HalfByteBin2Ascii((word & 0xF0000000) >> 28);
}

TimerEvent_t te;

static uint32_t testArr[16] = { 0xABCDABC0, 0xABCDABC1, 0xABCDABC2, 0x3, 0x4,
		0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };

static dumpRegisters(uint32_t pointer, uint8_t number) {
    uint8_t arr[10];

    for (uint32_t i = 0; i < number; i++) {
        // dump address
        WordBin2Ascii( 
            (uint32_t)((uint8_t*)pointer + (sizeof(uint32_t) * i))
            , arr 
        );        
    	// add line end
        arr[sizeof(arr)-2] = '*';
    	arr[sizeof(arr)-1] = '*';
        // send
    	UartPutBuffer(&Uart2, arr, sizeof(arr));
    	// clear send buffer
        for (uint8_t j = 0; j < sizeof(arr); j++) {
            arr[j] = 0;
        }
        // dump content
        WordBin2Ascii( 
            *((uint32_t*)((uint8_t*)pointer + (sizeof(uint32_t) * i)))
            , arr 
        );
    	// add line end
        arr[sizeof(arr)-2] = '\n';
    	arr[sizeof(arr)-1] = '\r';
        // send
    	UartPutBuffer(&Uart2, arr, sizeof(arr));
    	// clear send buffer
        for (uint8_t j = 0; j < sizeof(arr); j++) {
            arr[j] = 0;
        }
    }
    // generate line of dashes
    for (uint8_t j = 0; j < (sizeof(arr)-2); j++) {
            arr[j] = '-';
    }
    // add line end
    arr[sizeof(arr)-2] = '\n';
    arr[sizeof(arr)-1] = '\r';
    // send
    UartPutBuffer(&Uart2, arr, sizeof(arr));
}

static void function_1(void) 
{

    DelayMs(500);
    // RtcDelayMs(500);

    GpioToggle( &Led1 );
    
    // UartHartbeat();

}

static void function_2(void) {
    
    GpioToggle( &Led1 );
    BoardLowPowerHandler( );

}

static void function_3(void* context) 
{

    GpioToggle( &Led2 );

    TimerStart(&te);
}

/**
 * Main application entry point.
 */
int main( void )
{
    GpioInit( &interrupt, PB_8, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioSetInterrupt( &interrupt, IRQ_RISING_FALLING_EDGE, IRQ_HIGH_PRIORITY, &PB8_handler );

    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    TimerInit(&te, &function_3);
    TimerSetValue(&te, 100);
    TimerStart(&te);
    
    dumpRegisters(RCC, 32);
    
    while( 1 )
    {	
        function_1();
    }
}
