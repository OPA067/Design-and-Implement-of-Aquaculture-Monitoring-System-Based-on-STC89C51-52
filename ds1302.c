#include"reg52.h"
#include"ds1302.h"
//---DS1302写入和读取时分秒的地址命令---//
//---秒分时日月周年 最低位读写位;-------//
uchar code READ_RTC_ADDR[7]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d}; 
uchar code WRITE_RTC_ADDR[7]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
//READ_RTC_ADDR[i]=WRITE_RTC_ADDR[i]+1;

//---存储顺序是秒分时日月周年,存储格式是用BCD码---////注意十进制表示

uchar NOWTIME[7]={0x00,0x00,0x00,0x00,0x00,0x00,0x00};	//12点34分56秒	 

void Ds1302Write(uchar addr,uchar dat)					//写入的地址，写入的数据
{
	uchar n;
	RST=0;
	_nop_();
	SCLK=0;				//先将SCLK置低电平。
	_nop_();
	RST=1;	 			//然后将RST(CE)置高电平。
	_nop_();
	for(n=0;n<8;n++)	//开始传送八位地址命令
	{
		DSIO=addr&0x01; //数据从低位开始传送
		addr>>=1;
		SCLK=1;			//数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK=0;
		_nop_();
	}
	for(n=0;n<8;n++)	//写入8位数据
	{
		DSIO=dat&0x01;
		dat>>=1;
		SCLK=1;			//数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK=0;
		_nop_();	
	}	 
	RST=0;				//传送数据结束
	_nop_();
}
uchar Ds1302Read(uchar addr)
{
	uchar n,dat,dat1;
	RST=0;
	_nop_();
	SCLK=0;				//先将SCLK置低电平。
	_nop_();
	RST=1;				//然后将RST(CE)置高电平。
	_nop_();
	for(n=0;n<8;n++)	//开始传送八位地址命令
	{
		DSIO=addr&0x01;	//数据从低位开始传送
		addr>>=1;
		SCLK=1;			//数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK=0;			//DS1302下降沿时，放置数据
		_nop_();
	}
	_nop_();
	for(n=0;n<8;n++)	//读取8位数据
	{
		dat1=DSIO;		//从最低位开始接收
		dat=(dat>>1)|(dat1<<7);
		SCLK=1;
		_nop_();
		SCLK=0;			//DS1302下降沿时，放置数据
		_nop_();
	}
	RST=0;
	_nop_();			//以下为DS1302复位的稳定时间,必须的。
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
	Ds1302Write(0x8E,0X00);		 	//禁止写保护，就是关闭写保护功能
	for(n=0;n<7;n++)			 	//写入7个字节的时钟信号：分秒时日月周年
	{
		Ds1302Write(WRITE_RTC_ADDR[n],NOWTIME[n]);	
	}
	Ds1302Write(0x8E,0x80);			//打开写保护功能
}
void Ds1302ReadTime()
{
	uchar n;
	for(n=0;n<7;n++)				//读取7个字节的时钟信号：分秒时日月周年
	{
		NOWTIME[n]=Ds1302Read(READ_RTC_ADDR[n]);
	}
		
}
