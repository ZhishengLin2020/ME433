#include "imu.h"
#include "i2c_master_noint.h"
#include "ssd1306.h"

void imu_setup(){
    unsigned char who = 0;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    // read from IMU_WHOAMI
    who = readPin(0b11010110,0x0F);
    
    if (who != 0b01101001){
        while(1){
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < 24000000){}
            LATAbits.LATA4 = !LATAbits.LATA4;
        }
    }
    
    // init IMU_CTRL1_XL
    setPin(0b11010110,0x10,0b10000010);
    // init IMU_CTRL2_G
    setPin(0b11010110,0x11,0b10001000);
    // init IMU_CTRL3_C
    setPin(0b11010110,0x12,0b00000100);
}

void imu_read(unsigned char reg, signed short * data, int len){
    unsigned char raw[2*len];
    int i,j;
    // read multiple from the imu, each data takes 2 reads so you need len*2 chars
    i2c_master_read_multiple(0b11010110,reg,raw,2*len);
    // turn the chars into the shorts
    for(i=2*len-1,j=len-1;i>0;i=i-2,j--){
        data[j] = (raw[i]<<8)|raw[i-1];
    }
}

void bar_x(signed short x, int val){
    int i;
    x = 64*x/16383;
    if(x >= 0){
        for(i=0;i<x;i++){
            ssd1306_drawPixel(64+i,16,val);
        }
    }
    else{
        for(i=0;i>x;i--){
            ssd1306_drawPixel(64+i,16,val);
        }
    }
}

void bar_y(signed short y, int val){
    int i;
    y = 16*y/16383;
    if(y >= 0){
        for(i=0;i<y;i++){
            ssd1306_drawPixel(64,16+i,val);
        }
    }
    else{
        for(i=0;i>y;i--){
            ssd1306_drawPixel(64,16+i,val);
        }
    }
}