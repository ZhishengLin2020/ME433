#include <proc/p32mx170f256b.h>
#include "spi.h"

unsigned short ch_vol(unsigned char c, unsigned short v){
    unsigned short vol = (c<<15);
    vol = vol|(0b111<<12)|v;
    
    return vol;
}