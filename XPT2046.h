#ifndef	  __XPT2046_H_
#define   __XPT2046_H_

#include<reg52.h>
#include<intrins.h>

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint  unsigned int
#endif

#ifndef ulong
#define ulong  unsigned long
#endif

sbit DOUT=P3^7;	  //����ӿ�
sbit CLK =P3^6;	  //ʱ�ӽӿ�
sbit DIN =P3^4;	  //����ӿ�
sbit CS  =P3^5;	  //Ƭѡ�ӿ�

uint Read_AD_Data(uchar cmd);
uint SPI_AD_Read(void);
void SPI_AD_Write(uchar dat);

#endif





