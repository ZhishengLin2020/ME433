#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "rtcc.h"
#include "font.h"
#include "i2c_master_noint.h"
#include "ssd1306.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = ON // disable secondary oscillator
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
    
    __builtin_enable_interrupts();
    
    i2c_master_setup(); 
    ssd1306_setup();
    
    unsigned long time = 0x21271000;
    unsigned long date = 0x20060202;
    rtcc_setup(time, date);
    
    rtccTime t;
    char msg[50];
    char day[50];
    int i = 0;

    while (1) { 
        ssd1306_clear();
        t = readRTCC();
        dayOfTheWeek(t.wk, day);
        
        sprintf(msg, "Hi! %d", i);
        drawarray(0,0,msg);
        sprintf(msg, "%d%d:%d%d:%d%d", t.hr10,t.hr01,t.min10,t.min01,t.sec10,t.sec01);
        drawarray(8,16,msg);
        sprintf(msg, "%s, %d%d/%d%d/20%d%d", day, t.mn10,t.mn01,t.dy10,t.dy01,t.yr10,t.yr01);
        drawarray(8,24,msg);
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 24000000/2){;}
        ssd1306_update();
        i++;
    }
}