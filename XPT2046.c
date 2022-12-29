#include"XPT2046.h"
void SPI_AD_Write(uchar dat)
{
	uchar i;
	CLK=0;
	for(i=0;i<8;i++)
	{
		DIN=dat>>7;  		//放置最高位
		dat<<=1;
		CLK=0;				//上升沿放置数据
		CLK=1;
	}
}
uint SPI_AD_Read(void)
{
	uint i,dat=0;
	CLK=0;
	for(i=0;i<12;i++)		//接收12位数据
	{
		dat<<=1;
		CLK=1;
		CLK=0;
		dat|=DOUT;
	}
	return dat;	
}
uint Read_AD_Data(uchar cmd)
{
	uchar i;
	uint AD_Value;
	CLK=0;
	CS=0;
	SPI_AD_Write(cmd);
	for(i=6;i>0;i--); 		//延时等待转换结果
	CLK=1;	  				//发送一个时钟周期，清除BUSY
	_nop_();
	_nop_();
	CLK=0;
	_nop_();
	_nop_();
	AD_Value=SPI_AD_Read();
	CS=1;
	return AD_Value;	
}
