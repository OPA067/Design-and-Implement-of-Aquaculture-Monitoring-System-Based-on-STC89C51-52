typedef unsigned char u8;
typedef unsigned int u16;
/***���������ļ�ͷ�ļ�***/
#include"reg52.h" 
#include"intrins.h"
/***���ú����Ĵ���֪����Ҫ��ǰ�������ⰴ������***/
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
/***������������ģ��***/
sbit motoO2		=P1^0;		//��������ڣ��͵�ƽ��Ч��� ��motoO2   =0,OUT1=1;
sbit motoTemp	=P1^1;		//�¶ȵ���ڣ��͵�ƽ��Ч��� ��motoTemp =0,OUT2=1;
sbit motoFeed	=P1^2;		//Ͷι����ڣ��͵�ƽ��Ч��� ��motoFeed =0,OUT3=1;
sbit motoLight	=P1^3;		//�ƹ����ڣ��͵�ƽ��Ч��� ��motoLight=0,OUT4=1;
sbit motoEx		=P1^4;		//����չ�ӿ�
sbit beep		=P1^5;	 	//���Ʒ������ӿڣ��͵�ƽ��Ч���
/***����ͨ�ţ��ⲿ�ж�0***/
sbit HWin	=P3^2;			//����ͨ�������
sbit k1		=P3^1;  		//���������������ʹӻ��Ĺؼ�
/***������ʾ����������***/					 
u8 code HWsmgduan[30]={	45,46,47,  		//0  1  2
						44,40,43,		//3  4  5
						 7,15, 9,		//6  7  8	    
					    16,19,14,		//9  10 11
						12,18,64,		//12 13 14
						 8,22,60,		//15 16 17 
						42,52,50};		//18 19 20 
/***LCD12864��ʾ��Ϣ***/
u8 code LCD12864Init[] 		  ="ˮ����ֳ���ϵͳ";
u8 code LCD12864Set[]  	  	  ="��λ����Ѱ��ϵͳ";
u8 code LCD12864User[]		  ="�������뿪������";		
u8 code LCD12864Hello[]		  ="ϵͳ������......";
u8 code LCD12864Doing[]		  ="ϵͳ�����ɹ�....";
u8 code LCD12864Key[]		  ="�����������....";	
u8 code LCD12864Temp[]  	  ="��ǰ�¶�:";
u8 code LCD12864VO2[]         ="��ǰ����:"; 
u8 code LCD12864Light[]       ="��ǰ����:";
u8 code LCD12864SystemTemp[]  ="ϵͳ�¶�:";
u8 code LCD12864Time[]        ="�����ʱ";
u8 code LCD12864TDanWei[]     ="-- -- --";
u8 code LCD12864SportWaring[] ="ʵʱ��ر�������";
u8 code LCD12864WarningTemp[]  			="���浱ǰˮ�¸�!";
u8 code LCD12864WarningVO2[]   			="���浱ǰ������!";
u8 code LCD12864WarningLight[] 			="�������ǿ��";
u8 code LCD12864WarningStempH[]			="���滷���¶ȸ�!";
u8 code LCD12864WarningStempL[]			="���滷���¶ȵ�!";
u8 code LCD1268HintMsgTemp[]   			="��ǰˮ��������ֳ";
u8 code LCD1268HintMsgVO2[]    			="��ǰ����������ֳ";
u8 code LCD1268HintMsgLight[]  			="��ǰ����������ֳ";
u8 code LCD1268HintMsgSystem[] 			="��ǰϵͳ�¶��ʺ�";
u8 code LCD12864motoWorkingStatus[]		="�������״̬:";
u8 code LCD12864motoWorkingFindStatus[]	="���״̬��ѯ��..";
u8 code LCD12864TimeWorkingStatus[]		="��ǰʱ��״̬:";
u8 code LCD12864BeepStatus[]			="��������״̬:";
u8 code LCD12864BeepON[]				="������·�Ѵ�";
u8 code LCD12864motoStatus[]	 		="ˮ����ֳ���״̬";
u8 code LCD12864motoONALL[]				="�������ȫ������";
u8 code LCD12864motoO2[]				="������";
u8 code LCD12864motoTemp[]				="���»�";
u8 code LCD12864motoFeed[]				="Ͷ�ϻ�";
u8 code LCD12864motoLight[]				="���ջ�";
u8 code LCD12864WorkingVO2[]       		="������������..";
u8 code LCD12864WorkingVO2Time[]   		="����ʱ��";
u8 code LCD12864WorkingVO2TimeOut[]		="������һСʱֹͣ";
u8 code LCD12864WorkingTemp[]       	="���»�������..";
u8 code LCD12864WorkingTempTime[]   	="����ʱ��";
u8 code LCD12864WorkingTempTimeOut[]	="������һСʱֹͣ";
u8 code LCD12864WorkingFeed[]       	="Ͷ�ϻ�������..";
u8 code LCD12864WorkingFeedTime[]   	="����ʱ��";
u8 code LCD12864WorkingFeedTimeOut[]	="������һСʱֹͣ";
u8 code LCD12864WorkingLight[]       	="�����������..";
u8 code LCD12864WorkingLightTime[]   	="����ʱ��";
u8 code LCD12864LightRang[]				="�ο�Ͷι���շ�Χ";
u8 code LCD12864WorkingLightTimeOut[]	="������һСʱֹͣ";
u8 code LCD12864Lightmsg1[]				="������������Ͷι";
u8 code LCD12864Lightmsg2[]				="����ҹ��ע�����";
u8 code LCD12864Menu[]					="--�û���������--";
u8 code LCD12864MenuHint[]	  			="ϵͳ����Ѳ��ģʽ";
u8 code LCD12864NRF24L01[]				="���Դӻ��¶���Ϣ";
u8 code LCD12864TempNRF24L01[]			="ˮ��:";
u8 code LCD12864Vo2NRF24L01[]			="����:";
u8 code LCD12863Line[]		  			="----------------";
/***�û����ݻ���Ҫ���Զ�������������***/
#define TempH 	30.00	  	//�¶�����
#define TempL 	20.00		//�¶�����
#define O2H   	8.00		//���������ߣ��ʺ�ιʳ
#define O2L   	2.00		//���������ͣ��ʺ�����
#define LightH 	500			//����ǿ������
#define LightL 	50			//����ǿ������
#define STempH 	25.00	  	//ϵͳ�¶�����
#define STempL 	15.00		//ϵͳ�¶�����
#define UpTime 		5		//һ�㶨ʱ��ʱs-���ɶ�ʱ
#define UpTempTime 	60*60 	//��ʱһ��Сʱs-���»�
#define UpO2Time 	60*60 	//��ʱһ��Сʱs-������
#define UpLightTime 60*60 	//��ʱһ��Сʱs-�����
#define UpFeedTime  60*60 	//��ʱһ��Сʱs-Ͷ�ϻ�
void delay(u16 t)			//��ʱ���� 
{
	while(t--);
}
void delayLong(u16 t)		//����ʱ����
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
/***����������***/
void BeepWarning()
{
	while(1)
	{
		beep=~beep;
	 	delay(50);
	}
}





