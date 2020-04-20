#include <proc/p32mx170f256b.h>
#include "spi.h"

unsigned char spi_io(unsigned char o){
    SPI1BUF = o;
    while(!SPI1STATbits.SPIRBF){;}
    
    return SPI1BUF;
}