#ifndef __DS1302_H_
#define __DS1302_H_

#include<reg52.h>
#include<intrins.h>	 //_nop_();


#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint 
#define uint unsigned int
#endif

sbit DSIO =P3^4;
sbit RST  =P3^5;
sbit SCLK =P3^6;


void Ds1302Write(uchar addr, uchar dat);
uchar Ds1302Read(uchar addr);
void Ds1302Init();
void Ds1302ReadTime();

//---加入全局变量--//
extern uchar NOWTIME[7];	//时钟测试工作时间
#endif