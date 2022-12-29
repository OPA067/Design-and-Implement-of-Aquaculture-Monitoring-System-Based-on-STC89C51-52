#ifndef __LCD12864_H
#define __LCD12864_H

#include<reg52.h>

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint 
#define uint unsigned int
#endif

#define LCD12864_DATAPORT P0	  	 //����IO��

sbit LCD12864_RS  =P2^6;             //��������Ĵ���ѡ������ 
sbit LCD12864_RW  =P2^5;             //Һ����/д����
sbit LCD12864_EN  =P2^7;             //Һ��ʹ�ܿ���
sbit LCD12864_PSB =P3^2;             //��/����ʽ����
sbit LCD12864_RST =P3^3;			 //��λ��

void LCD12864_Delay1ms(uint c);
uchar LCD12864_Busy(void);
void LCD12864_WriteCmd(uchar cmd);
void LCD12864_WriteData(uchar dat);
void LCD12864_Init();
void LCD12864_ClearScreen(void);  	 //LCD12864_WriteCmd(0x01);
void LCD12864_SetWindow(uchar x, uchar y);

#endif