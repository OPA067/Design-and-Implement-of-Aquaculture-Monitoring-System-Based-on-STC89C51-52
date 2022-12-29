#include"reg52.h"
#include"ds1302.h"
//---DS1302д��Ͷ�ȡʱ����ĵ�ַ����---//
//---���ʱ�������� ���λ��дλ;-------//
uchar code READ_RTC_ADDR[7]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d}; 
uchar code WRITE_RTC_ADDR[7]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
//READ_RTC_ADDR[i]=WRITE_RTC_ADDR[i]+1;

//---�洢˳�������ʱ��������,�洢��ʽ����BCD��---////ע��ʮ���Ʊ�ʾ

uchar NOWTIME[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};	//12��34��56��	 

void Ds1302Write(uchar addr,uchar dat)					//д��ĵ�ַ��д�������
{
	uchar n;
	RST=0;
	_nop_();
	SCLK=0;				//�Ƚ�SCLK�õ͵�ƽ��
	_nop_();
	RST=1;	 			//Ȼ��RST(CE)�øߵ�ƽ��
	_nop_();
	for(n=0;n<8;n++)	//��ʼ���Ͱ�λ��ַ����
	{
		DSIO=addr&0x01; //���ݴӵ�λ��ʼ����
		addr>>=1;
		SCLK=1;			//������������ʱ��DS1302��ȡ����
		_nop_();
		SCLK=0;
		_nop_();
	}
	for(n=0;n<8;n++)	//д��8λ����
	{
		DSIO=dat&0x01;
		dat>>=1;
		SCLK=1;			//������������ʱ��DS1302��ȡ����
		_nop_();
		SCLK=0;
		_nop_();	
	}	 
	RST=0;				//�������ݽ���
	_nop_();
}
uchar Ds1302Read(uchar addr)
{
	uchar n,dat,dat1;
	RST=0;
	_nop_();
	SCLK=0;				//�Ƚ�SCLK�õ͵�ƽ��
	_nop_();
	RST=1;				//Ȼ��RST(CE)�øߵ�ƽ��
	_nop_();
	for(n=0;n<8;n++)	//��ʼ���Ͱ�λ��ַ����
	{
		DSIO=addr&0x01;	//���ݴӵ�λ��ʼ����
		addr>>=1;
		SCLK=1;			//������������ʱ��DS1302��ȡ����
		_nop_();
		SCLK=0;			//DS1302�½���ʱ����������
		_nop_();
	}
	_nop_();
	for(n=0;n<8;n++)	//��ȡ8λ����
	{
		dat1=DSIO;		//�����λ��ʼ����
		dat=(dat>>1)|(dat1<<7);
		SCLK=1;
		_nop_();
		SCLK=0;			//DS1302�½���ʱ����������
		_nop_();
	}
	RST=0;
	_nop_();			//����ΪDS1302��λ���ȶ�ʱ��,����ġ�
	SCLK=1;
	_nop_();
	DSIO=0;
	_nop_();
	DSIO=1;
	_nop_();
	return dat;	
}

void Ds1302Init()
{
	uchar n;
	Ds1302Write(0x8E,0X00);		 	//��ֹд���������ǹر�д��������
	for(n=0;n<7;n++)			 	//д��7���ֽڵ�ʱ���źţ�����ʱ��������
	{
		Ds1302Write(WRITE_RTC_ADDR[n],NOWTIME[n]);	
	}
	Ds1302Write(0x8E,0x80);			//��д��������
}
void Ds1302ReadTime()
{
	uchar n;
	for(n=0;n<7;n++)				//��ȡ7���ֽڵ�ʱ���źţ�����ʱ��������
	{
		NOWTIME[n]=Ds1302Read(READ_RTC_ADDR[n]);
	}
		
}
