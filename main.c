/***导入头文件***/
#include"reg52.h"			 
#include"config.h"
#include"string.h" 
#include"lcd12864.h"
#include"math.h"
#include"ds1302.h"
#include"temp.h"
#include"XPT2046.h"
#include"O2.h"
#include"intrins.h"
#include"nRF.h"
/***红外通信***/
u8 HWValue[4];	 		//数据格式数组
u8 Time;				//红外数据参数	
u8 Value;				//定义获取的数据，十进制表示
/***DS1302记录时间	定时器***/
u8 s,min,hour;			//时间：时-分-秒
u16 sumTime;			//总时间，单位s
u8 Timechar[8];			//显示时间数组
/***水体数据显示***/
double tempD;			//记录十进制表示的温度
double VO2D;			//记录十进制含氧量
double StempD;			//系统工作温度	
u16 Light;				//记录光照强度
u16 ST;					//记录系统工作温度
u8 tempDchar[5]; 	   	//显示温度数组
u8 VO2Dchar[5];	   		//显示氧气含量数组
u8 Lightchar[5];	   	//显示光照强度数组
u8 SystemTemp[6];	   	//显示系统工作温度数组
u8 flag=0;				//定义数据接收标识字符
u8 ChooseZC=0;			//主机和从机设置标志
/***检测NRF24L01通信是否正常***/
u8 bdata sta;	  		//定义无线通信的标识字符
sbit RX_DR	=sta^6;
sbit TX_DS	=sta^5;
sbit MAX_RT	=sta^4;
/***初始化系统***/
void InitSystem()
{
	motoO2	 =0;  		//关闭氧气电机
	motoTemp =0;		//关闭温度电机
	motoFeed =0;		//关闭投料电机
	motoLight=0;		//关闭光照电机
	beep=0;				//关闭蜂鸣器报警		
}
/***初始化外部中断0***/
void Init0Int()
{
 	IT0=1;				//下降沿触发
	EX0=1;				//打开中断0允许
	EA=1;				//打开总中断
	HWin=1;				//初始化端口
}
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
/***温度处理函数***/
void DataDealTemp(int temp) 	 
{
   	double tp;
	u8 t1,t2,t3,t4;  
	tp=temp;					//因为数据处理有小数点所以将温度赋给一个浮点型变量
	temp=tp*0.0625*100+0.5;	    //+0.5是四舍五入
	t1=temp%10000/1000;			//温度十位
	tempDchar[0]=t1+'0';  	
	t2=temp%1000/100;		   	//温度个位
	tempDchar[1]=t2+'0';  
	tempDchar[2]='.';	
	t3=temp%100/10;			  	//温度十分位
	tempDchar[3]=t3+'0';
	t4=temp%10/1;				//温度百分位
	tempDchar[4]=t4+'0'; 
	tp=t1*10+t2+t3*0.1+t4*0.01;
	tempD=tp;					//返回十进制温度全局变量，用于实时处理
}
/***氧气处理函数***/
void DataDealVO2(double vo2)	
{
	u8 v1,v2,v3,v4;
	int Intvo2=(int)(vo2*100);  
	v1=Intvo2/1000;
	VO2Dchar[0]=v1+'0';	       	//氧气十位			  
	v2=Intvo2/100%10;
	VO2Dchar[1]=v2+'0';			//氧气个位
	VO2Dchar[2]='.';			//氧气数据小数点
	v3=Intvo2%100/10;
	VO2Dchar[3]=v3+'0';			//氧气十分位
	v4=Intvo2%10;
	VO2Dchar[4]=v4+'0';			//氧气百分位
	VO2D=v1*10+v2
		 +v3*0.1+v4*0.01;		//返回十进制氧气全局变量，用于实时处理
}
/***按键操作，用户确定主机和从机***/
void CheckButtons()
{	
	if(k1==0)									//用户按下K1按键，说明当前被按下的单片机为数据发送端，另外一段为数据接收端	            
	{
		delay(20);								//延时
		if(k1==0)			    
		{
			DataDealTemp(Ds18b20ReadTemp()); 	//温度数据处理函数
			DataDealVO2(TempToVO2(tempD));		//氧气数据处理函数
			TX_Mode(TX_BUF);				 	//把nRF24L01设置为发送模式并发送数据
			Check_ACK(1);               	 	//等待发送完毕，清除TX_FIFO
			RX_Mode();			        	 	//设置为接收模式
			while(!k1);							//消抖
			ChooseZC=1;							//设置标识
		}	
	}
	if(ChooseZC==1)
	{
		Delay2000ms();							//延时2000ms再进行数据传输，可适当增加到一分钟
		DataDealTemp(Ds18b20ReadTemp()); 		//温度数据处理函数
		DataDealVO2(TempToVO2(tempD));			//氧气数据处理函数
		TX_Mode(TX_BUF);						//把nRF24L01设置为发送模式并发送数据
		Check_ACK(1);               			//等待发送完毕，清除TX_FIFO
		RX_Mode();			        			//设置为接收模式		
	}	
}
/***光照处理函数***/
void ADDealLight()
{	
	Light=Read_AD_Data(0xa4);					//AIN2光敏电阻
	Light=Read_AD_Data(0xa4);					//AIN2光敏电阻
	Light=Read_AD_Data(0xa4);					//AIN2光敏电阻
	Lightchar[0]=Light/1000+'0';				//数据千位
	Lightchar[1]=Light%1000/100+'0';			//数据百位
	Lightchar[2]=Light%1000%100/10+'0';			//数据十位
	Lightchar[3]=Light%1000%100%10+'0';			//数据个位			
}
/***系统温度处理函数***/
void ADDealSystemTemp()
{	
	ST=Read_AD_Data(0xd4);						//AIN1热敏电阻	
	ST=Read_AD_Data(0xd4);						//AIN1热敏电阻
	ST=Read_AD_Data(0xd4);						//AIN1热敏电阻	
	StempD=ST/100+ST%1000/100;	
	SystemTemp[0]=ST/1000+'0';					//数据千位
	SystemTemp[1]=ST%1000/100+'0';				//数据百位
	SystemTemp[2]='.';						   	//显示小数点
	SystemTemp[3]=ST%1000%100/10+'0';			//数据十位
	SystemTemp[4]=ST%1000%100%10+'0';			//数据个位		
}
/***通用LCD12864输出，传入u8类型的字符数组***/
void LCD12864Dis(u8 LCD12864String[])
{
	u8 i=0;
	while(LCD12864String[i]!='\0')
	{
		 LCD12864_WriteData(LCD12864String[i]);
		 i++;
	}
}
/***红外中断处理函数***/
void ReadIr() interrupt 0 						//定义外部中断0
{
	u8 j,k;
	u16 err;
	Time=0;					 			   	
	delay(700);									//延时7ms
	if(HWin==0)									//确认是否真的接收到正确的信号
	{	 	
		err=1000;					
		while((HWin==0)&&(err>0))				//等待前面9ms的低电平过去  		
		{			
			delay(1);
			err--;
		} 
		if(HWin==1)								//如果正确等到9ms低电平
		{
			err=500;
			while((HWin==1)&&(err>0))			//等待4.5ms的起始高电平过去
			{
				delay(1);
				err--;
			}
			for(k=0;k<4;k++)					//共有4组数据，数据格式获取
			{				
				for(j=0;j<8;j++)				//接收一组数据
				{

					err=60;		
					while((HWin==0)&&(err>0))	//等待信号前面的560us低电平过去
					{
						delay(1);
						err--;
					}
					err=500;
					while((HWin==1)&&(err>0))	//计算高电平的时间长度。
					{
						delay(10);	 			//延时0.1ms
						Time++;
						err--;
						if(Time>30)
						{
							return;
						}
					}
					HWValue[k]>>=1;	 			//k表示第几组数据
					if(Time>=8)					//如果高电平出现大于565us，那么是1
					{
						HWValue[k]|=0x80;
					}
					Time=0;						//用完时间要重新赋值							
				}
			}
		}
		if(HWValue[2]!=~HWValue[3])				//检验数据码和数据反码是否相同
		{
			return;
		}
	}			
}
/***红外按键处理函数HWWorkButton_xx()***/
void HWWorkButton_00()							//显示开机界面，显示初始界面，显示当前时间
{	 	
	LCD12864_WriteCmd(0x01);
	Delay100ms(); 	
	LCD12864_SetWindow(0,0);  					//LCD初始显示位置左上角(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//打印开机初始化信息
	LCD12864_SetWindow(1,0);					//LCD初始显示位置(x,y)==(1,0)
	LCD12864Dis(LCD12864Hello);					//打印开机初始化信息
	Delay1000ms();								//系统等待2s
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864Doing);					//显示用户提示信息
	Delay1000ms();
	LCD12864_SetWindow(2,0);					//显示用户提示信息
	LCD12864Dis(LCD12864Key);
	delayLong(1);								//长延时Delaylong-10												
}
void HWWorkButton_01()							//系统时钟测试
{
	Ds1302Init(); 								//初始化Ds302时钟，从00:00:00开始计时
	LCD12864_WriteCmd(0x01);   					//清除LCD12864的显示内容
	Delay100ms();
	s=0;min=0;hour=0;							//初始化系统计时器
	while(1)
	{  			
		LCD12864_SetWindow(0, 0);  
		LCD12864Dis(LCD12864Init);
		Delay100ms();
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864Time);
		Ds1302ReadTime();						//获取实时时间数据	
		Timechar[0]=(int)(NOWTIME[2]/16)+'0'; 	Timechar[1]=(int)(NOWTIME[2]&0x0f)+'0'; 	Timechar[2]=':';
		Timechar[3]=(int)(NOWTIME[1]/16)+'0'; 	Timechar[4]=(int)(NOWTIME[1]&0x0f)+'0';  	Timechar[5]=':';
		Timechar[6]=(int)(NOWTIME[0]/16)+'0';	Timechar[7]=(int)(NOWTIME[0]&0x0f)+'0';
		LCD12864Dis(Timechar);
		motoO2	 =1;
		motoTemp =1;
		motoFeed =1;
		motoLight=1;
		Delay100ms();
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864motoONALL);
		Delay100ms();	
		LCD12864_SetWindow(3,0);
		LCD12864Dis(LCD12864Set);								
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)
		{
			memset(NOWTIME,0,sizeof(NOWTIME));	//清空时间数据
			motoO2	 =0;
			motoTemp =0;
			motoFeed =0;
			motoLight=0;
			break;								//实际对应的按键值是4		
		}	
	}
	HWWorkButton_04();							//系统索引界面
}
void HWWorkButton_02()	 						//蜂鸣器测试模块
{
	LCD12864_WriteCmd(0x01);   					//清除LCD12864的显示内容
	Delay1000ms();
   	LCD12864_SetWindow(0,0);  
	LCD12864Dis(LCD12864Init);
	LCD12864_SetWindow(1,0);
	Delay1000ms();  
	LCD12864Dis(LCD12864BeepStatus);
	if(beep==0)
	{
		LCD12864Dis("-ON");	    				//显示报警模块打开
	}
	else
	{
	 	LCD12864Dis("OFF");						//显示报警模块关闭
	}
	Delay2000ms();	
	LCD12864_SetWindow(2,0);
	LCD12864Dis(LCD12864BeepON);
	LCD12864_SetWindow(3,0);
	LCD12864Dis(LCD12863Line);	 	
	BeepWarning();								//按复位键退出 
}
void HWWorkButton_03()							//显示各项电机工作状态
{
	LCD12864_WriteCmd(0x01);   					//清除LCD12864的显示内容
	Delay100ms();
	LCD12864_SetWindow(0,0);  					//LCD初始显示位置左上角(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//打印开机初始化信息
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864motoStatus);
	LCD12864_SetWindow(2,0);   	
	LCD12864Dis(LCD12864motoWorkingFindStatus);
	Delay1000ms();
	while(1)
	{	
		LCD12864_WriteCmd(0x01);   				//清除LCD12864的显示内容
		LCD12864_SetWindow(0,0);
		LCD12864Dis(LCD12864motoO2);
		if(motoO2==1)	LCD12864Dis("已开启-ON");	
		else	LCD12864Dis("已关闭OFF");
		Delay100ms();
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864motoTemp);
		if(motoTemp==1)	LCD12864Dis("已开启-ON");
		else   LCD12864Dis("已关闭OFF");
		Delay100ms();
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864motoFeed);
		if(motoFeed==1)LCD12864Dis("已开启-ON");
		else LCD12864Dis("已关闭OFF");
		Delay100ms();
		LCD12864_SetWindow(3,0); 
		LCD12864Dis(LCD12864motoLight);	
		if(motoLight==1)  LCD12864Dis("已开启-ON");
		else LCD12864Dis("已关闭OFF");	
		Delay2000ms();
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;					//实际对应的按键值是4			
	}
	HWWorkButton_04();
}
void HWWorkButton_04()							//显示返回信息，菜单信息===系统索引界面  
{
	LCD12864_WriteCmd(0x01);   					//清除LCD12864的显示内容 
	Delay100ms();
	LCD12864_WriteCmd(0x01);
	LCD12864_SetWindow(0,0);  					//LCD初始显示位置左上角(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//打印开机初始化信息
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864MenuHint);
   	LCD12864_SetWindow(2,0);
	LCD12864Dis(LCD12864Menu);	 
	LCD12864_SetWindow(3,0);
	LCD12864Dis(LCD12863Line); 
	Delay2000ms();								//延时接受新的红外数据信号
	return;
}	
void HWWorkButton_05()							//显示NRF24L01无线传输的信息.
{
	motoO2=0;motoTemp=0;motoFeed=0;motoLight=0;	//公共接口复位，开始NRF24L01工作										
	init_io();									//初始化NRF24L01端口
	RX_Mode();									//将本机设置为接收数据模式
	LCD12864_WriteCmd(0x01);   					//清除LCD12864的显示内容
	Delay100ms();
	LCD12864_SetWindow(0,0);  					//LCD初始显示位置左上角(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//打印开机初始化信息
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864NRF24L01);
	while(1)
	{	
		CheckButtons();           				//按键扫描
		sta=SPI_Read(STATUS);	  				//读状态寄存器
	    if(RX_DR)				  				//判断是否接受到数据
		{										//从RX_FIFO读出数据
			SPI_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);
			flag=1;
		}
		SPI_RW_Reg(WRITE_REG+STATUS,sta);  		//清除RX_DS中断标志
		if(flag)		           				//接受完成
		{
			u8 i;
			flag=0;		       					//清标志  
			LCD12864_SetWindow(1,0);			//LCD显示位置(1,0)
			LCD12864Dis(LCD12864NRF24L01);
			LCD12864_SetWindow(2,0);			//LCD显示位置(2,0)
			LCD12864Dis(LCD12864TempNRF24L01);
			for(i=0;i<5;i++)
				LCD12864_WriteData(RX_BUF[i]);	//打印接收到的温度数据
			LCD12864_WriteData('#');			//打印单位
			LCD12864_WriteData('C');			//打印单位
			LCD12864_SetWindow(3,0);			//LCD显示位置(3,0)
			LCD12864Dis(LCD12864Vo2NRF24L01);
			for(i=5;i<10;i++)
				LCD12864_WriteData(RX_BUF[i]);	//打印接收到的氧气数据
			LCD12864_WriteData('M');			//打印单位
			LCD12864_WriteData('G');			//打印单位
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;					//实际对应的按键值是4			
	}
	HWWorkButton_04();							//系统索引界面				
}				
void HWWorkButton_06(){return;}					//按键功能扩展代码
void HWWorkButton_07(){return;}					//按键功能扩展代码
void HWWorkButton_08(){return;}					//按键功能扩展代码
void HWWorkButton_09()							//实时显示各项数据--温度-含氧量-光照强度-系统工作温度-电机状态--时钟状况
{ 	
	LCD12864_WriteCmd(0x01);   				 
	Delay100ms();
	while(1)
	{			
		u8 t=10;	
		LCD12864_WriteCmd(0x01);
		LCD12864_SetWindow(0,0); 
		LCD12864Dis(LCD12864Init);	
		while(t!=0)
		{
		  	LCD12864_SetWindow(1,0);
			LCD12864Dis(LCD12864Temp);		
			DataDealTemp(Ds18b20ReadTemp()); 	//温度数据处理函数	  
			LCD12864Dis(tempDchar);
			LCD12864Dis("#C");
			Delay500ms();
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864VO2);
			DataDealTemp(Ds18b20ReadTemp()); 	//温度数据处理函数
			DataDealVO2(TempToVO2(tempD));	 	//氧气数据处理函数
			LCD12864Dis(VO2Dchar);
			LCD12864Dis("MG");
 			Delay500ms();
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12864Light);	
			ADDealLight();						//光照数据处理函数
			LCD12864Dis(Lightchar);
			LCD12864Dis("LUX");		 
			t--;
		}
		LCD12864_WriteCmd(0x01);
		LCD12864_SetWindow(0,0); 
		LCD12864Dis(LCD12864Init);
		t=10;
		while(t!=0)
		{
			LCD12864_SetWindow(1,0);
			LCD12864Dis(LCD12864SystemTemp);
			ADDealSystemTemp();					//获取系统工作温度		
			LCD12864Dis(SystemTemp);
			LCD12864Dis("#C");
			Delay500ms();
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864motoWorkingStatus);
			if(motoO2==1||motoTemp==1||motoFeed==1||motoLight==1)
			{
				LCD12864Dis("-ON");				 //查询电机工作状态
			}
			else
			{
				LCD12864Dis("OFF");
			}
			Delay500ms();
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12864TimeWorkingStatus);
			LCD12864Dis("-ON");	
			t--;
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;					//实际对应的按键值是4		
	}
	HWWorkButton_04();			
}
void HWWorkButton_10(){return;}					//按键功能扩展代码
void HWWorkButton_11(){return;}					//按键功能扩展代码
void HWWorkButton_12()							//显示温度大小，实时处理温度
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring);
	while(1)
	{
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864Temp);		
		DataDealTemp(Ds18b20ReadTemp()); 		//温度数据处理函数	  
		LCD12864Dis(tempDchar);
		LCD12864Dis("#C");
		if(tempD>=TempH) 				 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningTemp);
			Delay2000ms();				 		//适当延时	
			BeepWarning();				 		//转入蜂鸣器报警
		}		
		else if(tempD<=TempL)			 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WorkingTemp);
			motoTemp=1;			  				//转入增温电机工作，提升温度
		}								 
		else
		{
		  	LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD1268HintMsgTemp);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("TempLH:20#C-30#C");	//正常显示数据
			Delay1000ms();
			motoTemp=0;		  					//关闭电机
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;					//实际对应的按键值是4		
	}
	HWWorkButton_04();		
}
void HWWorkButton_13()							//显示氧气含量，实时处理温度
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring);
	while(1)
	{	
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864VO2);
		DataDealTemp(Ds18b20ReadTemp()); 		//温度数据处理函数
		DataDealVO2(TempToVO2(tempD));	 		//氧气数据处理函数
		LCD12864Dis(VO2Dchar);
		LCD12864Dis("MG");
		if(VO2D<=O2L)					 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningVO2);
			motoO2=1;				 			//转入电机工作,提升氧气
		}
		else if(VO2D>=O2H)					 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WorkingFeed);
			motoFeed=1;					 		//建议转入投喂机工作	
		}
		else
		{
		   	LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD1268HintMsgVO2);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("O2LH:4.5MG-5.5MG");	  //正常显示数据
			motoO2=0;		  					//关闭升氧电机 	
			motoFeed=0;			  				//关闭投料电机
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;					//实际对应的按键值是4		
	}
	HWWorkButton_04();
}
void HWWorkButton_14()							//显示光照强度，实时处理光照
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();	
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring); 			
	while(1)
	{
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864Light);	
		ADDealLight();							//光照数据处理函数
	   	ADDealLight();							//光照数据处理函数
		ADDealLight();							//光照数据处理函数
		LCD12864Dis(Lightchar);
		LCD12864Dis("LUX");	
		if(Light>=LightH)
		{	
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864Lightmsg1);		//提示光照强
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
		}
		else if(Light<=LightL)
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864Lightmsg2);		//提示光照弱小
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
			motoLight=1;	 					//开启电机
		}
		else 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864LightRang);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("光照:100LX-200LX");
			motoLight=0;						//关闭电机
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;					//实际对应的按键值是4		
	}
	HWWorkButton_04();	
}
void HWWorkButton_15()							//显示系统工作温度
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();				 
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring);
	while(1)
	{
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864SystemTemp);
		ADDealSystemTemp();						//获取系统工作温度
		ADDealSystemTemp();						//获取系统工作温度
		ADDealSystemTemp();						//获取系统工作温度	
		LCD12864Dis(SystemTemp);
		LCD12864Dis("#C");
		if(StempD>=STempH)
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningStempH);	//系统温度过高
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
			BeepWarning();						//蜂鸣器报警
		}
		else if(StempD<=STempL)
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningStempL);	//系统温度过低
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
			BeepWarning();						//蜂鸣器报警
		}
		else
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD1268HintMsgSystem);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("TempLH:15#C-25#C");	//系统正常工作
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;					//实际对应的按键值是4		
	}
	HWWorkButton_04();
}
/***非时电机工作状态设计***/
void HWWorkButton_16()							//定时增氧
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoO2=1;  									//启动增氧机
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init); 		
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864WorkingVO2);    
	Ds1302Init();
	while(1)
	{
		Ds1302ReadTime();
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864WorkingVO2Time);
		hour=NOWTIME[2]/16*10+NOWTIME[2]&0x0f;
		min=NOWTIME[1]/16*10+NOWTIME[1]&0x0f;
		s=NOWTIME[0]/16*10+NOWTIME[0]&0x0f;
		sumTime=hour*60*60+min*60+s;
		Timechar[0]=(int)(NOWTIME[2]/16)+'0'; 	Timechar[1]=(int)(NOWTIME[2]&0x0f)+'0'; 	Timechar[2]=':';
		Timechar[3]=(int)(NOWTIME[1]/16)+'0'; 	Timechar[4]=(int)(NOWTIME[1]&0x0f)+'0';  	Timechar[5]=':';
		Timechar[6]=(int)(NOWTIME[0]/16)+'0';	Timechar[7]=(int)(NOWTIME[0]&0x0f)+'0';
		LCD12864Dis(Timechar);
		if(sumTime==UpO2Time)  					//增氧机的工作时间
		{
			motoO2=0;
			return;	
		}
		LCD12864_SetWindow(3,0);
		LCD12864Dis(LCD12864WorkingVO2TimeOut);	
		Value=HWValue[2]/16*10+HWValue[2]%16;
		if(Value==40)
		{
			memset(NOWTIME,0,sizeof(NOWTIME));	 //清空时间数据
			motoO2=0;
		   	break;	
		}				
	}
	HWWorkButton_04();
}
void HWWorkButton_17()							//定时增料
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();						
	motoFeed=1;									//开启喂养机
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);	 	
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864WorkingFeed); 	
	Ds1302Init();						  		//初始化Ds302时钟，从00:00:00开始计时
	while(1)
	{
		Ds1302ReadTime();	
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864WorkingFeedTime);
		hour=NOWTIME[2]/16*10+NOWTIME[2]&0x0f;
		min=NOWTIME[1]/16*10+NOWTIME[1]&0x0f;
		s=NOWTIME[0]/16*10+NOWTIME[0]&0x0f;
		sumTime=hour*60*60+min*60+s;   			//计算时间总和
		Timechar[0]=(int)(NOWTIME[2]/16)+'0'; 	Timechar[1]=(int)(NOWTIME[2]&0x0f)+'0'; 	Timechar[2]=':';
		Timechar[3]=(int)(NOWTIME[1]/16)+'0'; 	Timechar[4]=(int)(NOWTIME[1]&0x0f)+'0';  	Timechar[5]=':';
		Timechar[6]=(int)(NOWTIME[0]/16)+'0';	Timechar[7]=(int)(NOWTIME[0]&0x0f)+'0';
		LCD12864Dis(Timechar);					//实时显示时间
		if(sumTime==UpFeedTime)  				//投料机器的工作时间
		{
			motoFeed=0;							//关闭喂养机
			return;	
		}
		LCD12864_SetWindow(3,0);
		LCD12864Dis(LCD12864WorkingFeedTimeOut);//显示提示信息
		Value=HWValue[2]/16*10+HWValue[2]%16;
		if(Value==40)
		{
			memset(NOWTIME,0,sizeof(NOWTIME));	 //清空时间数据
			motoFeed=0;
			break;	
		}
	}
	HWWorkButton_04();
}
/***非定时电机工作状态设计***/
void HWWorkButton_18()							//增氧电机工作
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoO2=1;
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);	
	while(1)
	{	
		LCD12864_SetWindow(0,0);  				//LCD初始显示位置左上角(x,y)==(0,0)
		LCD12864Dis(LCD12864Init);				//打印开机初始化信息		
		LCD12864_SetWindow(1,0);		
		LCD12864Dis(LCD12864WorkingVO2);
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864VO2);
		DataDealTemp(Ds18b20ReadTemp()); 		//温度数据处理函数
		DataDealVO2(TempToVO2(tempD));	 		//氧气数据处理函数
		LCD12864Dis(VO2Dchar);
		LCD12864Dis("MG"); 		
		LCD12864_SetWindow(3,0);
		if(VO2D>=5.5)	motoO2=0;	
		LCD12864Dis("氧达到5.00MG关闭");
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;		
	}
	HWWorkButton_04();
}
void HWWorkButton_19()							//增温电机工作	  
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoTemp=1;
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);
	while(1)
	{
		LCD12864_SetWindow(0, 0);  				//LCD初始显示位置左上角(x,y)==(0,0)
		LCD12864Dis(LCD12864Init);				//打印开机初始化信息
		LCD12864_SetWindow(1,0);		
		LCD12864Dis(LCD12864WorkingTemp);
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864Temp);		
		DataDealTemp(Ds18b20ReadTemp()); 		//温度数据处理函数	  
		LCD12864Dis(tempDchar);
		LCD12864Dis("#C");
		LCD12864_SetWindow(3,0);
		if(tempD>=20.0)		motoTemp=0;	
		LCD12864Dis("温度达到20#C关闭");
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40)	break;		
	}
	HWWorkButton_04();
}
void HWWorkButton_20()							//增料电机工作
{	
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoFeed=1;
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);
	while(1)
	{
		LCD12864_SetWindow(0, 0);  				//LCD初始显示位置左上角(x,y)==(0,0)
		LCD12864Dis(LCD12864Init);				//打印开机初始化信息
		LCD12864_SetWindow(1,0);		
		LCD12864Dis(LCD12864WorkingFeed);
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12863Line);
		LCD12864_SetWindow(3,0);
		LCD12864Dis(LCD12863Line);
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理
		if(Value==40) break;	
	}
	HWWorkButton_04();		
}
/***负责红外通信数据采集，总功能模块跳转main()代码***/							  
void main(void)
{
	InitSystem();	 							//初始化系统和外部中断系统
	Init0Int();									//初始化外部中断系统0
	LCD12864_Init();							//初始化LCD12864
	LCD12864_SetWindow(0,0);  					//LCD初始显示位置左上角(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//打印开机初始化信息
	LCD12864_SetWindow(1,0);  					//LCD初始显示位置左上角(x,y)==(1,0)
	LCD12864Dis(LCD12864User);	
	LCD12864_SetWindow(2,0);
	LCD12864Dis(LCD12863Line);	
	LCD12864_SetWindow(3,0);
	LCD12864Dis(LCD12863Line);
	Delay100ms();		 						//延时等待
	HWWorkButton_00();	 				 		//进入开机界面
//	HWWorkButton_0x();							//测试代码
	while(1)   									//红外控制界面
	{	
		u8 i; 
		Value=HWValue[2]/16*10+HWValue[2]%16;	//获取数据码高4位和低4位,转化为对应的十进制，方便处理			
		for(i=0;i<21;i++)			 			//共计21个按键
		{
			if(Value==HWsmgduan[i])				//转化为按键码
			{	
				LCD12864_WriteCmd(0x01);   		//清除LCD12864的显示内容
				switch(i)
				{
					case 0:	HWWorkButton_00();break; //开机界面显示
					case 1:	HWWorkButton_01();break; //时钟测试按键
					case 2:	HWWorkButton_02();break; //蜂鸣器测试
					case 3:	HWWorkButton_03();break; //显示各个电机工作状态
					case 4:	HWWorkButton_04();break; //系统索引界面，不写在主函数中.
					case 5:	HWWorkButton_05();break; //NRF24L01无线通信显示界面
					case 6:	HWWorkButton_06();break; //待扩展
					case 7:	HWWorkButton_07();break; //待扩展
					case 8:	HWWorkButton_08();break; //待扩展
					case 9:	HWWorkButton_09();break; //显示各项水体温度数据
					case 10:HWWorkButton_10();break; //待扩展
					case 11:HWWorkButton_11();break; //待扩展
					case 12:HWWorkButton_12();break; //显示温度
					case 13:HWWorkButton_13();break; //显示氧气
					case 14:HWWorkButton_14();break; //显示光照
					case 15:HWWorkButton_15();break; //显示系统工作实际温度
					case 16:HWWorkButton_16();break; //定时增氧
					case 17:HWWorkButton_17();break; //定时增料
					case 18:HWWorkButton_18();break; //增氧电机工作
					case 19:HWWorkButton_19();break; //增温电机工作
					case 20:HWWorkButton_20();break; //增料电机工作
				} 							
			}							   
		}	
	}		
}
