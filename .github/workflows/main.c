/***导入头文件***/
#include"reg52.h"
#include"nRF.h"
#include"temp.h"
#include"O2.h"
/***相关变量说明***/
typedef unsigned int u16;	  	//对数据类型进行声明定义
typedef unsigned char u8;	  	//对数据类型进行声明定义
u8 flag=0;					  	//定义数据接收标识字符
sbit k1=P3^1;  					//按键，设置主机和从机的关键
u8 ChooseZC=0;					//主机和从机设置标志
double NRFtemp;					//记录从机温度数据和氧气数据
/***延时函数1s***/
void Delay1000ms()		//@11.0592MHz
{
	unsigned char i,j,k;
	i=8;j=1;k=243;
	do
	{
		do
		{
			while(--k);
		}while(--j);
	}while(--i);
}

/***检测NRF24L01通信是否正常***/
u8 bdata sta;	  			  	//定义无线通信的标识字符
sbit RX_DR	=sta^6;
sbit TX_DS	=sta^5;
sbit MAX_RT	=sta^4;
u8 Check_ACK(bit clear)
{
	while(IRQ);
	sta=SPI_RW(NOP);                    	//返回状态寄存器
	if(MAX_RT)
	{									  	
	   	if(clear)                         	//没有清除在复位MAX_RT中断标志后重发
		{
			SPI_RW(FLUSH_TX);
		}		
	}
	SPI_RW_Reg(WRITE_REG+STATUS,sta);  		//清除TX_DS或MAX_RT中断标志
	IRQ=1;
	if(TX_DS)	return(0x00);
	else	return(0xff);
}

/***温度数据处理函数 功能：提取温度数据的各个位数：t1t2.t3t4.#C;***/
void DataDealTemp(int temp) 	 
{
   	double tp;
	int t1,t2,t3,t4;  
	tp=temp;					//因为数据处理有小数点所以将温度赋给一个浮点型变量
	temp=tp*0.0625*100+0.5;	    //+0.5是四舍五入
	t1=temp%10000/1000;			//温度十位
	TX_BUF[0]=t1+'0';	
	t2=temp%1000/100;		   	//温度个位
	TX_BUF[1]=t2+'0';	
	TX_BUF[2]='.';
	t3=temp%100/10;			  	//温度十分位
	TX_BUF[3]=t3+'0';
	t4=temp%10/1;				//温度百分位
	TX_BUF[4]=t4+'0';			//以上等价==memset(TX_BUF,t);
	NRFtemp=t1*10+t2+t3*0.1+t4*0.01;		//记录温度数据
}
/***氧气数据处理函数 功能：提取氧气数据的各个位数：t1t2.t3t4.MG;***/
void DataDealVO2() 	 
{
	int vo2=TempToVO2(NRFtemp)*100;
	TX_BUF[5]=vo2/1000+'0';	   //氧气数据十位
	TX_BUF[6]=vo2/100%10+'0';  //氧气数据个位
	TX_BUF[7]='.';			   //小数点
	TX_BUF[8]=vo2/10%10+'0';   //氧气数据十分位
	TX_BUF[9]=vo2%10+'0';	   //氧气数据百分位
}
/***按键操作，用户确定主机和从机***/
void CheckButtons()
{	
	if(k1==0)									//用户按下K1按键，说明当前被按下的单片机为数据发送端，另外一段为数据接收端	            
	{
		delay_ms(20);							//延时
		if(k1==0)			    
		{
			DataDealTemp(Ds18b20ReadTemp()); 	//温度数据处理函数
			DataDealVO2();						//氧气数据处理函数
			TX_Mode(TX_BUF);				 	//把nRF24L01设置为发送模式并发送数据
			Check_ACK(1);               	 	//等待发送完毕，清除TX_FIFO
			RX_Mode();			        	 	//设置为接收模式
			while(!k1);							//消抖
			ChooseZC=1;							//设置标识
		}	
	}
	if(ChooseZC==1)
	{
		Delay1000ms();							//延时2000ms再进行数据传输，可适当增加到一分钟
		DataDealTemp(Ds18b20ReadTemp()); 		//温度数据处理函数
		DataDealVO2();							//氧气数据处理函数
		TX_Mode(TX_BUF);						//把nRF24L01设置为发送模式并发送数据
		Check_ACK(1);               			//等待发送完毕，清除TX_FIFO
		RX_Mode();			        			//设置为接收模式		
	}	
}
/***主函数部分，发送数据控制代码***/
void main(void)
{
	init_io();		              							//初始化IO
	RX_Mode();		             	 						//设置为接收模式
	while(1)
	{
		CheckButtons();           							//按键扫描
		sta=SPI_Read(STATUS);	  							//读状态寄存器
	    if(RX_DR)				  							//判断是否接受到数据
		{
			SPI_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);//从RX FIFO读出数据
			flag=1;
		}
		SPI_RW_Reg(WRITE_REG+STATUS,sta);  					//清除RX_DS中断标志
	}
}

