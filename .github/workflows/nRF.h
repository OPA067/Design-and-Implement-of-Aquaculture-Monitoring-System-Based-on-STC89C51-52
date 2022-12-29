#ifndef _NRF_H_
#define _NRF_H_

#include"reg52.h"

#define uchar unsigned char
#define uint  unsigned int

#define TX_ADR_WIDTH   10 											//10字节宽度的发送/接收地址
#define TX_PLOAD_WIDTH 10 											//数据通道有效数据宽度
uchar code TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01};  	//定义一个静态发送地址
uchar RX_BUF[TX_PLOAD_WIDTH];	 									//从机发送数据
uchar TX_BUF[TX_PLOAD_WIDTH];										//主机接收数据

sbit CE   =P1^2;  //芯片使能引脚信号
sbit CSN  =P1^3;  //从端选择引脚
sbit IRQ  =P1^4;  //中断信号
sbit MISO =P1^6;  //主入从出
sbit MOSI =P1^1;  //主出从入
sbit SCK  =P1^7;  //时钟信号

#define READ_REG    0x00  //Define read command to register
#define WRITE_REG   0x20  //Define write command to register
#define RD_RX_PLOAD 0x61  //Define RX payload register address
#define WR_TX_PLOAD 0xA0  //Define TX payload register address
#define FLUSH_TX    0xE1  //Define flush TX register command
#define NOP         0xFF  //Define No Operation, might be used to read status register

#define CONFIG      0x00  //'Config' register address
#define EN_AA       0x01  //'Enable Auto Acknowledgment' register address
#define EN_RXADDR   0x02  //'Enabled RX addresses' register address
#define SETUP_RETR  0x04  //'Setup Auto. Retrans' register address
#define RF_CH       0x05  //'RF channel' register address
#define RF_SETUP    0x06  //'RF setup' register address
#define STATUS      0x07  //'Status' register address
#define RX_ADDR_P0  0x0A  //'RX address pipe0' register address
#define TX_ADDR     0x10  //'TX address' register address
#define RX_PW_P0    0x11  //'RX payload width, pipe0' register address
#endif  
 
void init_io(void)
{
	CE  =0;        //待机
	CSN =1;        //SPI禁止
	SCK =0;        //SPI时钟置低
	IRQ =1;        //中断复位
}

void delay_ms(uchar x)
{
    uchar i,j;
    i=0;
    for(i=0;i<x;i++)
    {
       j=250;
       while(--j);
	   j=250;
       while(--j);
    }
}

uchar SPI_RW(uchar byte)
{
	uchar i;
   	for(i=0;i<8;i++)          	//循环8次
   	{
   		MOSI=(byte&0x80);   	//byte最高位输出到MOSI
   		byte<<=1;             	//低一位移位到最高位
   		SCK=1;                	//拉高SCK，nRF24L01从MOSI读入1位数据，同时从MISO输出1位数据
   		byte|=MISO;       		//读MISO到byte最低位
   		SCK=0;            		//SCK置低
   	}
    return(byte);           	//返回读出的一字节
}
uchar SPI_RW_Reg(uchar reg,uchar value)
{
	uchar status;
  	CSN=0;                   	//CSN置低，开始传输数据
  	status=SPI_RW(reg);      	//选择寄存器，同时返回状态字
  	SPI_RW(value);              //然后写数据到该寄存器
  	CSN=1;                   	//CSN拉高，结束数据传输
  	return(status);            	//返回状态寄存器
}

uchar SPI_Read(uchar reg)
{
	uchar reg_val;
  	CSN=0;                    //CSN置低，开始传输数据
  	SPI_RW(reg);                //选择寄存器
  	reg_val = SPI_RW(0);        //然后从该寄存器读数据
  	CSN=1;                    //CSN拉高，结束数据传输
  	return(reg_val);            //返回寄存器数据
}
uchar SPI_Read_Buf(uchar reg,uchar * pBuf,uchar bytes)
{
	uchar status,i;
  	CSN=0;                   	//CSN置低，开始传输数据
  	status=SPI_RW(reg);       	//选择寄存器，同时返回状态字
  	for(i=0;i<bytes;i++)
    	pBuf[i]=SPI_RW(0);    	//逐个字节从nRF24L01读出
  	CSN=1;                    	//CSN拉高，结束数据传输
  	return(status);             //返回状态寄存器
}

uchar SPI_Write_Buf(uchar reg,uchar * pBuf,uchar bytes)
{
	uchar status, i;
  	CSN=0;                    	//CSN置低，开始传输数据
  	status=SPI_RW(reg);       	//选择寄存器，同时返回状态字
  	for(i=0;i<bytes;i++)
    	SPI_RW(pBuf[i]);      	//逐个字节写入nRF24L01
  	CSN=1;                    	//CSN拉高，结束数据传输
  	return(status);           	//返回状态寄存器
}
void RX_Mode(void)
{
	CE=0;
  	SPI_Write_Buf(WRITE_REG+RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH);  	//接收设备接收通道0使用和发送设备相同的发送地址
  	SPI_RW_Reg(WRITE_REG+EN_AA,0x01);               				//使能接收通道0自动应答
  	SPI_RW_Reg(WRITE_REG+EN_RXADDR,0x01);           				//使能接收通道0
  	SPI_RW_Reg(WRITE_REG+RF_CH,40);                 				//选择射频通道0x40
  	SPI_RW_Reg(WRITE_REG+RX_PW_P0,TX_PLOAD_WIDTH);  				//接收通道0选择和发送通道相同有效数据宽度
  	SPI_RW_Reg(WRITE_REG+RF_SETUP,0x07);            				//数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
  	SPI_RW_Reg(WRITE_REG+CONFIG,0x0f);              				//CRC使能，16位CRC校验，上电，接收模式
  	CE=1;                                         					//拉高CE启动接收设备
}
void TX_Mode(uchar *BUF)
{
	CE=0;
  	SPI_Write_Buf(WRITE_REG+TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);     	//写入发送地址
  	SPI_Write_Buf(WRITE_REG+RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH);  	//为了应答接收设备，接收通道0地址和发送地址相同
  	SPI_Write_Buf(WR_TX_PLOAD,BUF,TX_PLOAD_WIDTH);                  //写数据包到TX FIFO
  	SPI_RW_Reg(WRITE_REG+EN_AA,0x01);       						//使能接收通道0自动应答
  	SPI_RW_Reg(WRITE_REG+EN_RXADDR,0x01);   						//使能接收通道0
  	SPI_RW_Reg(WRITE_REG+SETUP_RETR,0x0a);  						//自动重发延时等待250us+86us，自动重发10次
  	SPI_RW_Reg(WRITE_REG+RF_CH,40);        			 				//选择射频通道0x40
  	SPI_RW_Reg(WRITE_REG+RF_SETUP,0x07);    						//数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
  	SPI_RW_Reg(WRITE_REG+CONFIG,0x0e);      						//CRC使能，16位CRC校验，上电
	CE=1;
}