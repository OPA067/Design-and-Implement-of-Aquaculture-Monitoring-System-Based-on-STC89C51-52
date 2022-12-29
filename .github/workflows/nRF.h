#ifndef _NRF_H_
#define _NRF_H_

#include"reg52.h"

#define uchar unsigned char
#define uint  unsigned int

#define TX_ADR_WIDTH   10 											//10�ֽڿ�ȵķ���/���յ�ַ
#define TX_PLOAD_WIDTH 10 											//����ͨ����Ч���ݿ��
uchar code TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01};  	//����һ����̬���͵�ַ
uchar RX_BUF[TX_PLOAD_WIDTH];	 									//�ӻ���������
uchar TX_BUF[TX_PLOAD_WIDTH];										//������������

sbit CE   =P1^2;  //оƬʹ�������ź�
sbit CSN  =P1^3;  //�Ӷ�ѡ������
sbit IRQ  =P1^4;  //�ж��ź�
sbit MISO =P1^6;  //����ӳ�
sbit MOSI =P1^1;  //��������
sbit SCK  =P1^7;  //ʱ���ź�

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
	CE  =0;        //����
	CSN =1;        //SPI��ֹ
	SCK =0;        //SPIʱ���õ�
	IRQ =1;        //�жϸ�λ
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
   	for(i=0;i<8;i++)          	//ѭ��8��
   	{
   		MOSI=(byte&0x80);   	//byte���λ�����MOSI
   		byte<<=1;             	//��һλ��λ�����λ
   		SCK=1;                	//����SCK��nRF24L01��MOSI����1λ���ݣ�ͬʱ��MISO���1λ����
   		byte|=MISO;       		//��MISO��byte���λ
   		SCK=0;            		//SCK�õ�
   	}
    return(byte);           	//���ض�����һ�ֽ�
}
uchar SPI_RW_Reg(uchar reg,uchar value)
{
	uchar status;
  	CSN=0;                   	//CSN�õͣ���ʼ��������
  	status=SPI_RW(reg);      	//ѡ��Ĵ�����ͬʱ����״̬��
  	SPI_RW(value);              //Ȼ��д���ݵ��üĴ���
  	CSN=1;                   	//CSN���ߣ��������ݴ���
  	return(status);            	//����״̬�Ĵ���
}

uchar SPI_Read(uchar reg)
{
	uchar reg_val;
  	CSN=0;                    //CSN�õͣ���ʼ��������
  	SPI_RW(reg);                //ѡ��Ĵ���
  	reg_val = SPI_RW(0);        //Ȼ��ӸüĴ���������
  	CSN=1;                    //CSN���ߣ��������ݴ���
  	return(reg_val);            //���ؼĴ�������
}
uchar SPI_Read_Buf(uchar reg,uchar * pBuf,uchar bytes)
{
	uchar status,i;
  	CSN=0;                   	//CSN�õͣ���ʼ��������
  	status=SPI_RW(reg);       	//ѡ��Ĵ�����ͬʱ����״̬��
  	for(i=0;i<bytes;i++)
    	pBuf[i]=SPI_RW(0);    	//����ֽڴ�nRF24L01����
  	CSN=1;                    	//CSN���ߣ��������ݴ���
  	return(status);             //����״̬�Ĵ���
}

uchar SPI_Write_Buf(uchar reg,uchar * pBuf,uchar bytes)
{
	uchar status, i;
  	CSN=0;                    	//CSN�õͣ���ʼ��������
  	status=SPI_RW(reg);       	//ѡ��Ĵ�����ͬʱ����״̬��
  	for(i=0;i<bytes;i++)
    	SPI_RW(pBuf[i]);      	//����ֽ�д��nRF24L01
  	CSN=1;                    	//CSN���ߣ��������ݴ���
  	return(status);           	//����״̬�Ĵ���
}
void RX_Mode(void)
{
	CE=0;
  	SPI_Write_Buf(WRITE_REG+RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH);  	//�����豸����ͨ��0ʹ�úͷ����豸��ͬ�ķ��͵�ַ
  	SPI_RW_Reg(WRITE_REG+EN_AA,0x01);               				//ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG+EN_RXADDR,0x01);           				//ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG+RF_CH,40);                 				//ѡ����Ƶͨ��0x40
  	SPI_RW_Reg(WRITE_REG+RX_PW_P0,TX_PLOAD_WIDTH);  				//����ͨ��0ѡ��ͷ���ͨ����ͬ��Ч���ݿ��
  	SPI_RW_Reg(WRITE_REG+RF_SETUP,0x07);            				//���ݴ�����1Mbps�����书��0dBm���������Ŵ�������
  	SPI_RW_Reg(WRITE_REG+CONFIG,0x0f);              				//CRCʹ�ܣ�16λCRCУ�飬�ϵ磬����ģʽ
  	CE=1;                                         					//����CE���������豸
}
void TX_Mode(uchar *BUF)
{
	CE=0;
  	SPI_Write_Buf(WRITE_REG+TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);     	//д�뷢�͵�ַ
  	SPI_Write_Buf(WRITE_REG+RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH);  	//Ϊ��Ӧ������豸������ͨ��0��ַ�ͷ��͵�ַ��ͬ
  	SPI_Write_Buf(WR_TX_PLOAD,BUF,TX_PLOAD_WIDTH);                  //д���ݰ���TX FIFO
  	SPI_RW_Reg(WRITE_REG+EN_AA,0x01);       						//ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG+EN_RXADDR,0x01);   						//ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG+SETUP_RETR,0x0a);  						//�Զ��ط���ʱ�ȴ�250us+86us���Զ��ط�10��
  	SPI_RW_Reg(WRITE_REG+RF_CH,40);        			 				//ѡ����Ƶͨ��0x40
  	SPI_RW_Reg(WRITE_REG+RF_SETUP,0x07);    						//���ݴ�����1Mbps�����书��0dBm���������Ŵ�������
  	SPI_RW_Reg(WRITE_REG+CONFIG,0x0e);      						//CRCʹ�ܣ�16λCRCУ�飬�ϵ�
	CE=1;
}