typedef unsigned char u8;
typedef unsigned int u16;
/***导入配置文件头文件***/
#include"reg52.h" 
#include"intrins.h"
/***调用函数的次序不知，需要提前声明红外按键函数***/
void HWWorkButton_00();
void HWWorkButton_01();
void HWWorkButton_02();
void HWWorkButton_03();
void HWWorkButton_04();
void HWWorkButton_05();
void HWWorkButton_06();
void HWWorkButton_07();
void HWWorkButton_08();
void HWWorkButton_09();
void HWWorkButton_10();
void HWWorkButton_11();
void HWWorkButton_12();
void HWWorkButton_13();
void HWWorkButton_14();
void HWWorkButton_15();
void HWWorkButton_16();
void HWWorkButton_17();
void HWWorkButton_18();
void HWWorkButton_19();
void HWWorkButton_20();
/***五线四项步进电机模块***/
sbit motoO2		=P1^0;		//氧气电机口，低电平有效输出 即motoO2   =0,OUT1=1;
sbit motoTemp	=P1^1;		//温度电机口，低电平有效输出 即motoTemp =0,OUT2=1;
sbit motoFeed	=P1^2;		//投喂电机口，低电平有效输出 即motoFeed =0,OUT3=1;
sbit motoLight	=P1^3;		//灯光电机口，低电平有效输出 即motoLight=0,OUT4=1;
sbit motoEx		=P1^4;		//待扩展接口
sbit beep		=P1^5;	 	//控制蜂鸣器接口，低电平有效输出
/***红外通信，外部中断0***/
sbit HWin	=P3^2;			//红外通信输入口
sbit k1		=P3^1;  		//按键，设置主机和从机的关键
/***红外显示数字数组表格***/					 
u8 code HWsmgduan[30]={	45,46,47,  		//0  1  2
						44,40,43,		//3  4  5
						 7,15, 9,		//6  7  8	    
					    16,19,14,		//9  10 11
						12,18,64,		//12 13 14
						 8,22,60,		//15 16 17 
						42,52,50};		//18 19 20 
/***LCD12864显示信息***/
u8 code LCD12864Init[] 		  ="水产养殖监控系统";
u8 code LCD12864Set[]  	  	  ="复位按键寻航系统";
u8 code LCD12864User[]		  ="即将进入开机界面";		
u8 code LCD12864Hello[]		  ="系统启动中......";
u8 code LCD12864Doing[]		  ="系统启动成功....";
u8 code LCD12864Key[]		  ="按任意键继续....";	
u8 code LCD12864Temp[]  	  ="当前温度:";
u8 code LCD12864VO2[]         ="当前氧气:"; 
u8 code LCD12864Light[]       ="当前光照:";
u8 code LCD12864SystemTemp[]  ="系统温度:";
u8 code LCD12864Time[]        ="电机计时";
u8 code LCD12864TDanWei[]     ="-- -- --";
u8 code LCD12864SportWaring[] ="实时监控报警工作";
u8 code LCD12864WarningTemp[]  			="警告当前水温高!";
u8 code LCD12864WarningVO2[]   			="警告当前含氧低!";
u8 code LCD12864WarningLight[] 			="警告光照强度";
u8 code LCD12864WarningStempH[]			="警告环境温度高!";
u8 code LCD12864WarningStempL[]			="警告环境温度低!";
u8 code LCD1268HintMsgTemp[]   			="当前水温适宜养殖";
u8 code LCD1268HintMsgVO2[]    			="当前含氧适宜养殖";
u8 code LCD1268HintMsgLight[]  			="当前光照适宜养殖";
u8 code LCD1268HintMsgSystem[] 			="当前系统温度适合";
u8 code LCD12864motoWorkingStatus[]		="电机工作状态:";
u8 code LCD12864motoWorkingFindStatus[]	="电机状态查询中..";
u8 code LCD12864TimeWorkingStatus[]		="当前时钟状态:";
u8 code LCD12864BeepStatus[]			="报警工作状态:";
u8 code LCD12864BeepON[]				="报警电路已打开";
u8 code LCD12864motoStatus[]	 		="水产养殖电机状态";
u8 code LCD12864motoONALL[]				="工作电机全部启动";
u8 code LCD12864motoO2[]				="增氧机";
u8 code LCD12864motoTemp[]				="增温机";
u8 code LCD12864motoFeed[]				="投料机";
u8 code LCD12864motoLight[]				="光照机";
u8 code LCD12864WorkingVO2[]       		="增氧机工作中..";
u8 code LCD12864WorkingVO2Time[]   		="工作时间";
u8 code LCD12864WorkingVO2TimeOut[]		="工作到一小时停止";
u8 code LCD12864WorkingTemp[]       	="增温机工作中..";
u8 code LCD12864WorkingTempTime[]   	="工作时间";
u8 code LCD12864WorkingTempTimeOut[]	="工作到一小时停止";
u8 code LCD12864WorkingFeed[]       	="投料机工作中..";
u8 code LCD12864WorkingFeedTime[]   	="工作时间";
u8 code LCD12864WorkingFeedTimeOut[]	="工作到一小时停止";
u8 code LCD12864WorkingLight[]       	="增光机工作中..";
u8 code LCD12864WorkingLightTime[]   	="工作时间";
u8 code LCD12864LightRang[]				="参考投喂光照范围";
u8 code LCD12864WorkingLightTimeOut[]	="工作到一小时停止";
u8 code LCD12864Lightmsg1[]				="光照适宜适宜投喂";
u8 code LCD12864Lightmsg2[]				="进入夜晚注意防盗";
u8 code LCD12864Menu[]					="--用户按键操作--";
u8 code LCD12864MenuHint[]	  			="系统进入巡航模式";
u8 code LCD12864NRF24L01[]				="来自从机温度信息";
u8 code LCD12864TempNRF24L01[]			="水温:";
u8 code LCD12864Vo2NRF24L01[]			="氧气:";
u8 code LCD12863Line[]		  			="----------------";
/***用户根据环境要求自定义数据上下限***/
#define TempH 	30.00	  	//温度上限
#define TempL 	20.00		//温度下限
#define O2H   	8.00		//氧气含量高，适合喂食
#define O2L   	2.00		//氧气含量低，适合增氧
#define LightH 	500			//光照强度上限
#define LightL 	50			//光照强度下限
#define STempH 	25.00	  	//系统温度上限
#define STempL 	15.00		//系统温度下限
#define UpTime 		5		//一般定时定时s-自由定时
#define UpTempTime 	60*60 	//定时一个小时s-升温机
#define UpO2Time 	60*60 	//定时一个小时s-升氧机
#define UpLightTime 60*60 	//定时一个小时s-增光机
#define UpFeedTime  60*60 	//定时一个小时s-投料机
void delay(u16 t)			//延时函数 
{
	while(t--);
}
void delayLong(u16 t)		//长延时函数
{
	u8 i,j,k;
	for(i=0;i<t;i++)
		for(j=0;j<t;j++)
			for(k=0;k<t;k++);
}
void Delay100ms()		//@11.0592MHz
{
	unsigned char i,j;
	i=180;j=73;
	do
	{
		while(--j);
	}while(--i);
}
void Delay500ms()		//@11.0592MHz
{
	unsigned char i,j,k;
	_nop_();
	i=4;j=129;k=119;
	do
	{
		do
		{
			while(--k);
		}while(--j);
	}while(--i);
}
void Delay1000ms()		//@11.0592MHz
{
	unsigned char i, j, k;
	_nop_();
	i=8;j=1;k=243;
	do{
		do{
			while(--k);
		}while(--j);
	}while(--i);
}
void Delay2000ms()		//@11.0592MHz
{
	unsigned char i,j,k;
	_nop_();
	i=15;j=2;k=235;
	do{
		do{
			while(--k);
		}while(--j);
	}while(--i);
}
/***蜂鸣器报警***/
void BeepWarning()
{
	while(1)
	{
		beep=~beep;
	 	delay(50);
	}
}





