#ifndef __TEMP_H_
#define __TEMP_H_

#include<reg52.h>

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint 
#define uint unsigned int
#endif

sbit DSPORT=P3^7;

void Delay1ms(uint );
uchar Ds18b20Init();
void Ds18b20WriteByte(uchar com);
uchar Ds18b20ReadByte();
void  Ds18b20ChangTemp();
void  Ds18b20ReadTempCom();
int Ds18b20ReadTemp();

#endif
