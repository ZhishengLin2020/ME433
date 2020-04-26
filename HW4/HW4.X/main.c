#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
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

void drawletter(int x, int y, char letter);
void drawarray(int x, int y, char* arr);

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
    
    i2c_master_setup();
    ssd1306_setup();
    
    __builtin_enable_interrupts();
    
    char arr1[50];
    char arr2[50];
    sprintf(arr1,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    sprintf(arr2,"01234567890123456789012345");
    drawarray(0,0,arr1);
    ssd1306_update();
    drawarray(0,8,arr2);
    ssd1306_update();
    
    int i = 0;
    float time;
    char arr3[50];
    char arr4[50];
    
    while (1) {
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 24000000) {;}
        LATAbits.LATA4 = !LATAbits.LATA4;
        
        sprintf(arr3,"Hello World! i = %d",i);
        drawarray(5,16,arr3);
        ssd1306_update();
        i++;
        
        _CP0_SET_COUNT(0);
        ssd1306_update();
        time = _CP0_GET_COUNT();
        time = 24000000/time;
        sprintf(arr4,"FPS = %.2f",time);
        drawarray(50,24,arr4);
        ssd1306_update();
    }
}


void drawletter(int x, int y, char letter){
    int i,j;
    for(i=0;i<5;i++){
        for(j=0;j<8;j++){
            if(((ASCII[letter-0x20][i] >> j) & 1) == 1){
                ssd1306_drawPixel(x+i, y+j, 1);
            }
            else{
                ssd1306_drawPixel(x+i, y+j, 0);
            }
        }
    }
}

void drawarray(int x, int y, char* arr){
    int i = 0;
    while(arr[i] != 0){
        drawletter(x+5*i,y,arr[i]);
        i++;
    }
}