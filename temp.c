#include"temp.h"
void Delay1ms(uint y)			//延时1ms
{
	uint x;
	for(;y>0;y--)
		for(x=110;x>0;x--);
}

uchar Ds18b20Init()
{
	uchar i;
	DSPORT=0;					//将总线拉低480us~960us
	i=70;	
	while(i--);					//延时642us
	DSPORT=1;					//然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线拉低
	i=0;
	while(DSPORT)				//等待DS18B20拉低总线
	{
		Delay1ms(1);
		i++;
		if(i>5)					//等待>5MS
		{
			return 0;			//初始化失败
		}
	
	}
	return 1;					//初始化成功
}
void Ds18b20WriteByte(uchar dat)
{
	uint i,j;
	for(j=0;j<8;j++)
	{
		DSPORT=0;	    		//每写入一位数据之前先把总线拉低1us
		i++;
		DSPORT=dat&0x01;		//然后写入一个数据，从最低位开始
		i=6;
		while(i--); 			//延时68us，持续时间最少60us
		DSPORT=1;				//然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
		dat>>=1;
	}
}
uchar Ds18b20ReadByte()
{
	uchar byte,bi;
	uint i,j;	
	for(j=8;j>0;j--)
	{
		DSPORT=0;				//先将总线拉低1us
		i++;
		DSPORT=1;				//然后释放总线
		i++;
		i++;					//延时6us等待数据稳定
		bi=DSPORT;	 			//读取数据，从最低位开始读取
		byte=(byte>>1)|(bi<<7);						  
		i=4;					//读取完之后等待48us再接着读取下一个数
		while(i--);
	}				
	return byte;
}
void  Ds18b20ChangTemp()
{
	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);		//跳过ROM操作命令		 
	Ds18b20WriteByte(0x44);	    //温度转换命令
}
void  Ds18b20ReadTempCom()
{	
				  
	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);	 	//跳过ROM操作命令
	Ds18b20WriteByte(0xbe);	 	//发送读取温度命令
}
int Ds18b20ReadTemp()
{
	int temp=0;
	uchar tmh, tml;
	Ds18b20ChangTemp();			//先写入转换命令
	Ds18b20ReadTempCom();		//然后等待转换完后发送读取温度命令
	tml=Ds18b20ReadByte();		//读取温度值共16位，先读低字节
	tmh=Ds18b20ReadByte();		//再读高字节
	temp=tmh;
	temp<<=8;
	temp|=tml;
	return temp;
}