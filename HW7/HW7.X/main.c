#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "adc.h"
#include "font.h"
#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "ws2812b.h"
#include <stdio.h>
#include <math.h>

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

    // setup
    adc_setup();
    ctmu_setup();
    i2c_master_setup();
    ssd1306_setup();
    ws2812b_setup();
    
    // some variables
    int read1[10];
    int read1_sum = 0;
    int read2[10];
    int read2_sum = 0;
    int i;
    char array1[50];
    char array2[50];
    char array3[50];
    
    // color
    wsColor color[4];
    color[3] = HSBtoRGB(0,0,0);
    
    
    while(1){
        // left capacitive touch
        for(i=0;i<10;i++){
            read1[i] = ctmu_read(4,50);
            read1_sum = read1_sum+read1[i];
        }
        sprintf(array1,"%d",read1_sum);
        drawarray(0,0,array1);
        
        // right capacitive touch
        for(i=0;i<10;i++){
            read2[i] = ctmu_read(5,50);
            read2_sum = read2_sum+read2[i];
        }
        sprintf(array2,"%d",read2_sum);
        drawarray(0,8,array2);
        
        // calculate position
        float read1_base = 1000;
        float read2_base = 1000;
        float delta1 = read1_base - (float)(0.1*read1_sum);
        float delta2 = read2_base - (float)(0.1*read2_sum);
        float pos1 = (delta1*100)/(delta1 + delta2);
        float pos2 = ((1-delta2)*100)/(delta1 + delta2);
        float pos = ((pos1 + pos2)/2);
        sprintf(array3,"%f",pos);
        drawarray(0,16,array3);
        
        // display
        ssd1306_update();
        ssd1306_clear();
        
        // decide 
        if(read1_sum > 7000){
            color[0] = HSBtoRGB(120,0,0);
        }
        else{
            color[0] = HSBtoRGB(120,1,1);
        }
            
        if(read2_sum > 7000){
            color[1] = HSBtoRGB(120,0,0);
        }
        else{
            color[1] = HSBtoRGB(120,1,1);
        }
            
        if(read1_sum+read2_sum > 12500){
            color[2] = HSBtoRGB(0,0,0);
        }
        else{
            color[2] = HSBtoRGB(270,1,pos/100.0);
        }

        
        ws2812b_setColor(color,4);
        
        // set back to 0
        read1_sum = 0;
        read2_sum = 0;
        
        //delay
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000000/100) {}
        
    }
}