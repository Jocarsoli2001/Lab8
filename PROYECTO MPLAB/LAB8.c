/*
 * File:   LAB8.c
 * Author: José Santizo
 *
 * Creado el 4 de octubre de 2021
 * 
 * Descripción: Contadores que aumentan por medio de una entrada analógica (pots)
 */

//---------------------Bits de configuración-------------------------------
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


//-----------------Librerías utilizadas en código-------------------- 
#include <xc.h>
#include <stdint.h>
#include <stdio.h>

//-----------------Definición de frecuencia de cristal---------------
#define _XTAL_FREQ 4000000

//-----------------------Constantes----------------------------------
#define  valor_tmr0 237              // valor_tmr0 = 61
#define  prueba 123

//-----------------------Variables------------------------------------
int cont2 = 0;
int cont_vol = 0;
int digi = 0;
int uni = 0;
int dece = 0;
int cen = 0;
uint8_t  disp_selector = 0b001;
int dig[3];

//------------Funciones sin retorno de variables----------------------
void setup(void);                   // Función de setup
void divisor(void);
void tmr0(void);                    // Función para reiniciar TMR0
void displays(void);

//-------------Funciones que retornan variables-----------------------
int  tabla(int a);

//----------------------Interrupciones--------------------------------
void __interrupt() isr(void){
    if(PIR1bits.ADIF){
        if(ADCON0bits.CHS == 1){
            cont2 = ADRESH;
            cont_vol = cont2*2;
        }
        else{
            PORTC = ADRESH;
        }
        PIR1bits.ADIF = 0;
        divisor();
    }
    if(T0IF){
        tmr0();
        displays();
    }
}

//----------------------Main Loop--------------------------------
void main(void) {
    setup();                        // Subrutina de setup
    ADCON0bits.GO = 1;
    while(1){
        if(ADCON0bits.GO == 0){
            if(ADCON0bits.CHS == 1){
                ADCON0bits.CHS = 0;
            }
            else{
                ADCON0bits.CHS = 1;
            }
            __delay_us(50);
            ADCON0bits.GO = 1;
        } 
    }
}

//----------------------Subrutinas--------------------------------
void setup(void){
    
    //Configuración de entradas y salidas
    ANSEL = 0b00000011;                     // Pines 2 y 3 como analógicos
    ANSELH = 0;
    
    TRISA = 0b00000011;                     // PORTA como salida
    TRISC = 0;                              // PORTC como salida
    TRISD = 0;
    TRISE = 0;
    
    PORTA = 0;
    PORTD = 0;
    PORTC = 0;
    PORTE = 0;
    
    //Configuración de oscilador
    OSCCONbits.IRCF = 0b0110;       // Oscilador a 4 MHz = 110
    OSCCONbits.SCS = 1;
    
    //Configuración de TMR0
    OPTION_REGbits.T0CS = 0;        // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
    OPTION_REGbits.T0SE = 0;        // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
    OPTION_REGbits.PSA = 0;         // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the Timer0
    OPTION_REGbits.PS2 = 1;         // bits 2-0  PS2:PS0: Prescaler Rate Select bits
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    TMR0 = valor_tmr0;              // preset for timer register
    
    //Configuración del ADC
    ADCON1bits.ADFM = 0;
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    
    ADCON0bits.ADCS = 0b01;
    ADCON0bits.CHS = 0;
    ADCON0bits.ADON = 1;
    __delay_us(50);
    
    //Configuración de interrupciones
    INTCONbits.T0IF = 0;            // Habilitada la bandera de TIMER 0      
    INTCONbits.T0IE = 1;            // Habilitar las interrupciones de TIMER 0
    INTCONbits.GIE = 1;             // Habilitar interrupciones globales
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
    INTCONbits.PEIE = 1;
    
    return;
}

void tmr0(void){
    INTCONbits.T0IF = 0;            // Limpiar bandera de TIMER 0
    TMR0 = valor_tmr0;              // TMR0 = 61
    return;
}

void divisor(void){
    PORTD = cont_vol;
    for(int i = 2; i >= 4; i--){
        dig[ i ] = cont_vol % 10;
        cont_vol /= 10;
    }
    dig[0] = uni;
    dig[1] = dece;
    dig[2] = cen;
}

void displays(void){
    PORTE = disp_selector;
    if(disp_selector == 0b001){
        PORTD = tabla(uni);
        disp_selector = 0b010;
    }
    else if(disp_selector == 0b010){
        PORTD = tabla(dece);
        disp_selector = 0b100;
    }
    else if(disp_selector == 0b100){
        PORTD = tabla(cen);
        disp_selector = 0b001;
    }
}
 
int tabla(int a){
    switch (a){
        case 0:
            return 0b00111111;
            break;
        case 1:
            return 0b00000110;
            break;
        case 2:
            return 0b01011011;
            break;
        case 3:
            return 0b01001111;
            break;
        case 4:
            return 0b01100110;
            break;
        case 5:
            return 0b01101101;
            break;
        case 6:
            return 0b01111101;
            break;
        case 7:
            return 0b00000111;
            break;
        case 8:
            return 0b01111111;
            break;
        case 9:
            return 0b01101111;
            break;
        case 10:
            return 0b01111011;
        default:
            break;
            
    }
}