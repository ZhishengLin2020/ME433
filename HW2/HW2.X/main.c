#include "spi.h"
#include <math.h>
#include <stdio.h>
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

// define a constant
#define PI 3.1415926

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB4 = 1;
    LATAbits.LATA4 = 0;
    
    initSPI();
    
    __builtin_enable_interrupts();
    
    // define some parameters
    unsigned short vol;
    int sinnum = 150;
    int trinum = 300;
    int sinwave[sinnum];
    int triwave[trinum];
    int i,j;
    float ysin = 0;
    float ytri = 0;
    
    // generate an array of sin wave
    for(i=0;i<sinnum;i++){
        ysin = (4096/2)*sin(4*PI*((1.0/trinum)*i))+(4096/2);
        sinwave[i] = (int)ysin;
    }
    
    // generate an array of triangle wave
    for(j=0;j<trinum;j++){
        if(j < sinnum){
            ytri = ytri+((3.3*4096)/(3.3*sinnum));
        }
        else{
            ytri = ytri-((3.3*4096)/(3.3*sinnum));
        }
        triwave[j] = (int)ytri;
    }
    
    // reset index i and j
    i = 0;
    j = 0;
    
    // loop arrays and DAC
    while (1) {
        // sin wave
        vol = ch_vol(0b0,sinwave[i]); // calculate voltage
        LATAbits.LATA0 = 0; // bring cs to low
        spi_io(vol>>8); // send the most significant bits of voltage
        spi_io(vol); // send the least significant bits of voltage
        LATAbits.LATA0 = 1; // bring cs to high
        
        // triangle wave
        vol = ch_vol(0b1,triwave[j]); // calculate voltage
        LATAbits.LATA0 = 0; // bring cs to low
        spi_io(vol>>8); // send the most significant bits of voltage
        spi_io(vol); // send the least significant bits of voltage
        LATAbits.LATA0 = 1; // bring cs to high
        
        // loop wave arrays
        i++;
        j++;
        
        // go back to first index of wave arrays
        if(i == sinnum){
            i = 0;
        }
        if(j == trinum){
            j = 0;
        }
        
        // delay a dt
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000/(2.0*trinum)){;}
    }
}