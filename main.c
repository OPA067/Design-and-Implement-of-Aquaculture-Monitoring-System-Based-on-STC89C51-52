/***����ͷ�ļ�***/
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
/***����ͨ��***/
u8 HWValue[4];	 		//���ݸ�ʽ����
u8 Time;				//�������ݲ���	
u8 Value;				//�����ȡ�����ݣ�ʮ���Ʊ�ʾ
/***DS1302��¼ʱ��	��ʱ��***/
u8 s,min,hour;			//ʱ�䣺ʱ-��-��
u16 sumTime;			//��ʱ�䣬��λs
u8 Timechar[8];			//��ʾʱ������
/***ˮ��������ʾ***/
double tempD;			//��¼ʮ���Ʊ�ʾ���¶�
double VO2D;			//��¼ʮ���ƺ�����
double StempD;			//ϵͳ�����¶�	
u16 Light;				//��¼����ǿ��
u16 ST;					//��¼ϵͳ�����¶�
u8 tempDchar[5]; 	   	//��ʾ�¶�����
u8 VO2Dchar[5];	   		//��ʾ������������
u8 Lightchar[5];	   	//��ʾ����ǿ������
u8 SystemTemp[6];	   	//��ʾϵͳ�����¶�����
u8 flag=0;				//�������ݽ��ձ�ʶ�ַ�
u8 ChooseZC=0;			//�����ʹӻ����ñ�־
/***���NRF24L01ͨ���Ƿ�����***/
u8 bdata sta;	  		//��������ͨ�ŵı�ʶ�ַ�
sbit RX_DR	=sta^6;
sbit TX_DS	=sta^5;
sbit MAX_RT	=sta^4;
/***��ʼ��ϵͳ***/
void InitSystem()
{
	motoO2	 =0;  		//�ر��������
	motoTemp =0;		//�ر��¶ȵ��
	motoFeed =0;		//�ر�Ͷ�ϵ��
	motoLight=0;		//�رչ��յ��
	beep=0;				//�رշ���������		
}
/***��ʼ���ⲿ�ж�0***/
void Init0Int()
{
 	IT0=1;				//�½��ش���
	EX0=1;				//���ж�0����
	EA=1;				//�����ж�
	HWin=1;				//��ʼ���˿�
}
u8 Check_ACK(bit clear)
{
	while(IRQ);
	sta=SPI_RW(NOP);                    	//����״̬�Ĵ���
	if(MAX_RT)
	{									  	
	   	if(clear)                         	//û������ڸ�λMAX_RT�жϱ�־���ط�
		{
			SPI_RW(FLUSH_TX);
		}		
	}
	SPI_RW_Reg(WRITE_REG+STATUS,sta);  		//���TX_DS��MAX_RT�жϱ�־
	IRQ=1;
	if(TX_DS)	return(0x00);
	else	return(0xff);
}
/***�¶ȴ�����***/
void DataDealTemp(int temp) 	 
{
   	double tp;
	u8 t1,t2,t3,t4;  
	tp=temp;					//��Ϊ���ݴ�����С�������Խ��¶ȸ���һ�������ͱ���
	temp=tp*0.0625*100+0.5;	    //+0.5����������
	t1=temp%10000/1000;			//�¶�ʮλ
	tempDchar[0]=t1+'0';  	
	t2=temp%1000/100;		   	//�¶ȸ�λ
	tempDchar[1]=t2+'0';  
	tempDchar[2]='.';	
	t3=temp%100/10;			  	//�¶�ʮ��λ
	tempDchar[3]=t3+'0';
	t4=temp%10/1;				//�¶Ȱٷ�λ
	tempDchar[4]=t4+'0'; 
	tp=t1*10+t2+t3*0.1+t4*0.01;
	tempD=tp;					//����ʮ�����¶�ȫ�ֱ���������ʵʱ����
}
/***����������***/
void DataDealVO2(double vo2)	
{
	u8 v1,v2,v3,v4;
	int Intvo2=(int)(vo2*100);  
	v1=Intvo2/1000;
	VO2Dchar[0]=v1+'0';	       	//����ʮλ			  
	v2=Intvo2/100%10;
	VO2Dchar[1]=v2+'0';			//������λ
	VO2Dchar[2]='.';			//��������С����
	v3=Intvo2%100/10;
	VO2Dchar[3]=v3+'0';			//����ʮ��λ
	v4=Intvo2%10;
	VO2Dchar[4]=v4+'0';			//�����ٷ�λ
	VO2D=v1*10+v2
		 +v3*0.1+v4*0.01;		//����ʮ��������ȫ�ֱ���������ʵʱ����
}
/***�����������û�ȷ�������ʹӻ�***/
void CheckButtons()
{	
	if(k1==0)									//�û�����K1������˵����ǰ�����µĵ�Ƭ��Ϊ���ݷ��Ͷˣ�����һ��Ϊ���ݽ��ն�	            
	{
		delay(20);								//��ʱ
		if(k1==0)			    
		{
			DataDealTemp(Ds18b20ReadTemp()); 	//�¶����ݴ�����
			DataDealVO2(TempToVO2(tempD));		//�������ݴ�����
			TX_Mode(TX_BUF);				 	//��nRF24L01����Ϊ����ģʽ����������
			Check_ACK(1);               	 	//�ȴ�������ϣ����TX_FIFO
			RX_Mode();			        	 	//����Ϊ����ģʽ
			while(!k1);							//����
			ChooseZC=1;							//���ñ�ʶ
		}	
	}
	if(ChooseZC==1)
	{
		Delay2000ms();							//��ʱ2000ms�ٽ������ݴ��䣬���ʵ����ӵ�һ����
		DataDealTemp(Ds18b20ReadTemp()); 		//�¶����ݴ�����
		DataDealVO2(TempToVO2(tempD));			//�������ݴ�����
		TX_Mode(TX_BUF);						//��nRF24L01����Ϊ����ģʽ����������
		Check_ACK(1);               			//�ȴ�������ϣ����TX_FIFO
		RX_Mode();			        			//����Ϊ����ģʽ		
	}	
}
/***���մ�����***/
void ADDealLight()
{	
	Light=Read_AD_Data(0xa4);					//AIN2��������
	Light=Read_AD_Data(0xa4);					//AIN2��������
	Light=Read_AD_Data(0xa4);					//AIN2��������
	Lightchar[0]=Light/1000+'0';				//����ǧλ
	Lightchar[1]=Light%1000/100+'0';			//���ݰ�λ
	Lightchar[2]=Light%1000%100/10+'0';			//����ʮλ
	Lightchar[3]=Light%1000%100%10+'0';			//���ݸ�λ			
}
/***ϵͳ�¶ȴ�����***/
void ADDealSystemTemp()
{	
	ST=Read_AD_Data(0xd4);						//AIN1��������	
	ST=Read_AD_Data(0xd4);						//AIN1��������
	ST=Read_AD_Data(0xd4);						//AIN1��������	
	StempD=ST/100+ST%1000/100;	
	SystemTemp[0]=ST/1000+'0';					//����ǧλ
	SystemTemp[1]=ST%1000/100+'0';				//���ݰ�λ
	SystemTemp[2]='.';						   	//��ʾС����
	SystemTemp[3]=ST%1000%100/10+'0';			//����ʮλ
	SystemTemp[4]=ST%1000%100%10+'0';			//���ݸ�λ		
}
/***ͨ��LCD12864���������u8���͵��ַ�����***/
void LCD12864Dis(u8 LCD12864String[])
{
	u8 i=0;
	while(LCD12864String[i]!='\0')
	{
		 LCD12864_WriteData(LCD12864String[i]);
		 i++;
	}
}
/***�����жϴ�����***/
void ReadIr() interrupt 0 						//�����ⲿ�ж�0
{
	u8 j,k;
	u16 err;
	Time=0;					 			   	
	delay(700);									//��ʱ7ms
	if(HWin==0)									//ȷ���Ƿ���Ľ��յ���ȷ���ź�
	{	 	
		err=1000;					
		while((HWin==0)&&(err>0))				//�ȴ�ǰ��9ms�ĵ͵�ƽ��ȥ  		
		{			
			delay(1);
			err--;
		} 
		if(HWin==1)								//�����ȷ�ȵ�9ms�͵�ƽ
		{
			err=500;
			while((HWin==1)&&(err>0))			//�ȴ�4.5ms����ʼ�ߵ�ƽ��ȥ
			{
				delay(1);
				err--;
			}
			for(k=0;k<4;k++)					//����4�����ݣ����ݸ�ʽ��ȡ
			{				
				for(j=0;j<8;j++)				//����һ������
				{

					err=60;		
					while((HWin==0)&&(err>0))	//�ȴ��ź�ǰ���560us�͵�ƽ��ȥ
					{
						delay(1);
						err--;
					}
					err=500;
					while((HWin==1)&&(err>0))	//����ߵ�ƽ��ʱ�䳤�ȡ�
					{
						delay(10);	 			//��ʱ0.1ms
						Time++;
						err--;
						if(Time>30)
						{
							return;
						}
					}
					HWValue[k]>>=1;	 			//k��ʾ�ڼ�������
					if(Time>=8)					//����ߵ�ƽ���ִ���565us����ô��1
					{
						HWValue[k]|=0x80;
					}
					Time=0;						//����ʱ��Ҫ���¸�ֵ							
				}
			}
		}
		if(HWValue[2]!=~HWValue[3])				//��������������ݷ����Ƿ���ͬ
		{
			return;
		}
	}			
}
/***���ⰴ��������HWWorkButton_xx()***/
void HWWorkButton_00()							//��ʾ�������棬��ʾ��ʼ���棬��ʾ��ǰʱ��
{	 	
	LCD12864_WriteCmd(0x01);
	Delay100ms(); 	
	LCD12864_SetWindow(0,0);  					//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//��ӡ������ʼ����Ϣ
	LCD12864_SetWindow(1,0);					//LCD��ʼ��ʾλ��(x,y)==(1,0)
	LCD12864Dis(LCD12864Hello);					//��ӡ������ʼ����Ϣ
	Delay1000ms();								//ϵͳ�ȴ�2s
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864Doing);					//��ʾ�û���ʾ��Ϣ
	Delay1000ms();
	LCD12864_SetWindow(2,0);					//��ʾ�û���ʾ��Ϣ
	LCD12864Dis(LCD12864Key);
	delayLong(1);								//����ʱDelaylong-10												
}
void HWWorkButton_01()							//ϵͳʱ�Ӳ���
{
	Ds1302Init(); 								//��ʼ��Ds302ʱ�ӣ���00:00:00��ʼ��ʱ
	LCD12864_WriteCmd(0x01);   					//���LCD12864����ʾ����
	Delay100ms();
	s=0;min=0;hour=0;							//��ʼ��ϵͳ��ʱ��
	while(1)
	{  			
		LCD12864_SetWindow(0, 0);  
		LCD12864Dis(LCD12864Init);
		Delay100ms();
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864Time);
		Ds1302ReadTime();						//��ȡʵʱʱ������	
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
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)
		{
			memset(NOWTIME,0,sizeof(NOWTIME));	//���ʱ������
			motoO2	 =0;
			motoTemp =0;
			motoFeed =0;
			motoLight=0;
			break;								//ʵ�ʶ�Ӧ�İ���ֵ��4		
		}	
	}
	HWWorkButton_04();							//ϵͳ��������
}
void HWWorkButton_02()	 						//����������ģ��
{
	LCD12864_WriteCmd(0x01);   					//���LCD12864����ʾ����
	Delay1000ms();
   	LCD12864_SetWindow(0,0);  
	LCD12864Dis(LCD12864Init);
	LCD12864_SetWindow(1,0);
	Delay1000ms();  
	LCD12864Dis(LCD12864BeepStatus);
	if(beep==0)
	{
		LCD12864Dis("-ON");	    				//��ʾ����ģ���
	}
	else
	{
	 	LCD12864Dis("OFF");						//��ʾ����ģ��ر�
	}
	Delay2000ms();	
	LCD12864_SetWindow(2,0);
	LCD12864Dis(LCD12864BeepON);
	LCD12864_SetWindow(3,0);
	LCD12864Dis(LCD12863Line);	 	
	BeepWarning();								//����λ���˳� 
}
void HWWorkButton_03()							//��ʾ����������״̬
{
	LCD12864_WriteCmd(0x01);   					//���LCD12864����ʾ����
	Delay100ms();
	LCD12864_SetWindow(0,0);  					//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//��ӡ������ʼ����Ϣ
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864motoStatus);
	LCD12864_SetWindow(2,0);   	
	LCD12864Dis(LCD12864motoWorkingFindStatus);
	Delay1000ms();
	while(1)
	{	
		LCD12864_WriteCmd(0x01);   				//���LCD12864����ʾ����
		LCD12864_SetWindow(0,0);
		LCD12864Dis(LCD12864motoO2);
		if(motoO2==1)	LCD12864Dis("�ѿ���-ON");	
		else	LCD12864Dis("�ѹر�OFF");
		Delay100ms();
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864motoTemp);
		if(motoTemp==1)	LCD12864Dis("�ѿ���-ON");
		else   LCD12864Dis("�ѹر�OFF");
		Delay100ms();
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864motoFeed);
		if(motoFeed==1)LCD12864Dis("�ѿ���-ON");
		else LCD12864Dis("�ѹر�OFF");
		Delay100ms();
		LCD12864_SetWindow(3,0); 
		LCD12864Dis(LCD12864motoLight);	
		if(motoLight==1)  LCD12864Dis("�ѿ���-ON");
		else LCD12864Dis("�ѹر�OFF");	
		Delay2000ms();
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;					//ʵ�ʶ�Ӧ�İ���ֵ��4			
	}
	HWWorkButton_04();
}
void HWWorkButton_04()							//��ʾ������Ϣ���˵���Ϣ===ϵͳ��������  
{
	LCD12864_WriteCmd(0x01);   					//���LCD12864����ʾ���� 
	Delay100ms();
	LCD12864_WriteCmd(0x01);
	LCD12864_SetWindow(0,0);  					//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//��ӡ������ʼ����Ϣ
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864MenuHint);
   	LCD12864_SetWindow(2,0);
	LCD12864Dis(LCD12864Menu);	 
	LCD12864_SetWindow(3,0);
	LCD12864Dis(LCD12863Line); 
	Delay2000ms();								//��ʱ�����µĺ��������ź�
	return;
}	
void HWWorkButton_05()							//��ʾNRF24L01���ߴ������Ϣ.
{
	motoO2=0;motoTemp=0;motoFeed=0;motoLight=0;	//�����ӿڸ�λ����ʼNRF24L01����										
	init_io();									//��ʼ��NRF24L01�˿�
	RX_Mode();									//����������Ϊ��������ģʽ
	LCD12864_WriteCmd(0x01);   					//���LCD12864����ʾ����
	Delay100ms();
	LCD12864_SetWindow(0,0);  					//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//��ӡ������ʼ����Ϣ
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864NRF24L01);
	while(1)
	{	
		CheckButtons();           				//����ɨ��
		sta=SPI_Read(STATUS);	  				//��״̬�Ĵ���
	    if(RX_DR)				  				//�ж��Ƿ���ܵ�����
		{										//��RX_FIFO��������
			SPI_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);
			flag=1;
		}
		SPI_RW_Reg(WRITE_REG+STATUS,sta);  		//���RX_DS�жϱ�־
		if(flag)		           				//�������
		{
			u8 i;
			flag=0;		       					//���־  
			LCD12864_SetWindow(1,0);			//LCD��ʾλ��(1,0)
			LCD12864Dis(LCD12864NRF24L01);
			LCD12864_SetWindow(2,0);			//LCD��ʾλ��(2,0)
			LCD12864Dis(LCD12864TempNRF24L01);
			for(i=0;i<5;i++)
				LCD12864_WriteData(RX_BUF[i]);	//��ӡ���յ����¶�����
			LCD12864_WriteData('#');			//��ӡ��λ
			LCD12864_WriteData('C');			//��ӡ��λ
			LCD12864_SetWindow(3,0);			//LCD��ʾλ��(3,0)
			LCD12864Dis(LCD12864Vo2NRF24L01);
			for(i=5;i<10;i++)
				LCD12864_WriteData(RX_BUF[i]);	//��ӡ���յ�����������
			LCD12864_WriteData('M');			//��ӡ��λ
			LCD12864_WriteData('G');			//��ӡ��λ
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;					//ʵ�ʶ�Ӧ�İ���ֵ��4			
	}
	HWWorkButton_04();							//ϵͳ��������				
}				
void HWWorkButton_06(){return;}					//����������չ����
void HWWorkButton_07(){return;}					//����������չ����
void HWWorkButton_08(){return;}					//����������չ����
void HWWorkButton_09()							//ʵʱ��ʾ��������--�¶�-������-����ǿ��-ϵͳ�����¶�-���״̬--ʱ��״��
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
			DataDealTemp(Ds18b20ReadTemp()); 	//�¶����ݴ�����	  
			LCD12864Dis(tempDchar);
			LCD12864Dis("#C");
			Delay500ms();
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864VO2);
			DataDealTemp(Ds18b20ReadTemp()); 	//�¶����ݴ�����
			DataDealVO2(TempToVO2(tempD));	 	//�������ݴ�����
			LCD12864Dis(VO2Dchar);
			LCD12864Dis("MG");
 			Delay500ms();
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12864Light);	
			ADDealLight();						//�������ݴ�����
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
			ADDealSystemTemp();					//��ȡϵͳ�����¶�		
			LCD12864Dis(SystemTemp);
			LCD12864Dis("#C");
			Delay500ms();
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864motoWorkingStatus);
			if(motoO2==1||motoTemp==1||motoFeed==1||motoLight==1)
			{
				LCD12864Dis("-ON");				 //��ѯ�������״̬
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
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;					//ʵ�ʶ�Ӧ�İ���ֵ��4		
	}
	HWWorkButton_04();			
}
void HWWorkButton_10(){return;}					//����������չ����
void HWWorkButton_11(){return;}					//����������չ����
void HWWorkButton_12()							//��ʾ�¶ȴ�С��ʵʱ�����¶�
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring);
	while(1)
	{
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864Temp);		
		DataDealTemp(Ds18b20ReadTemp()); 		//�¶����ݴ�����	  
		LCD12864Dis(tempDchar);
		LCD12864Dis("#C");
		if(tempD>=TempH) 				 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningTemp);
			Delay2000ms();				 		//�ʵ���ʱ	
			BeepWarning();				 		//ת�����������
		}		
		else if(tempD<=TempL)			 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WorkingTemp);
			motoTemp=1;			  				//ת�����µ�������������¶�
		}								 
		else
		{
		  	LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD1268HintMsgTemp);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("TempLH:20#C-30#C");	//������ʾ����
			Delay1000ms();
			motoTemp=0;		  					//�رյ��
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;					//ʵ�ʶ�Ӧ�İ���ֵ��4		
	}
	HWWorkButton_04();		
}
void HWWorkButton_13()							//��ʾ����������ʵʱ�����¶�
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring);
	while(1)
	{	
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864VO2);
		DataDealTemp(Ds18b20ReadTemp()); 		//�¶����ݴ�����
		DataDealVO2(TempToVO2(tempD));	 		//�������ݴ�����
		LCD12864Dis(VO2Dchar);
		LCD12864Dis("MG");
		if(VO2D<=O2L)					 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningVO2);
			motoO2=1;				 			//ת��������,��������
		}
		else if(VO2D>=O2H)					 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WorkingFeed);
			motoFeed=1;					 		//����ת��Ͷι������	
		}
		else
		{
		   	LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD1268HintMsgVO2);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("O2LH:4.5MG-5.5MG");	  //������ʾ����
			motoO2=0;		  					//�ر�������� 	
			motoFeed=0;			  				//�ر�Ͷ�ϵ��
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;					//ʵ�ʶ�Ӧ�İ���ֵ��4		
	}
	HWWorkButton_04();
}
void HWWorkButton_14()							//��ʾ����ǿ�ȣ�ʵʱ�������
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();	
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring); 			
	while(1)
	{
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864Light);	
		ADDealLight();							//�������ݴ�����
	   	ADDealLight();							//�������ݴ�����
		ADDealLight();							//�������ݴ�����
		LCD12864Dis(Lightchar);
		LCD12864Dis("LUX");	
		if(Light>=LightH)
		{	
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864Lightmsg1);		//��ʾ����ǿ
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
		}
		else if(Light<=LightL)
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864Lightmsg2);		//��ʾ������С
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
			motoLight=1;	 					//�������
		}
		else 
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864LightRang);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("����:100LX-200LX");
			motoLight=0;						//�رյ��
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;					//ʵ�ʶ�Ӧ�İ���ֵ��4		
	}
	HWWorkButton_04();	
}
void HWWorkButton_15()							//��ʾϵͳ�����¶�
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();				 
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864SportWaring);
	while(1)
	{
		LCD12864_SetWindow(1,0);
		LCD12864Dis(LCD12864SystemTemp);
		ADDealSystemTemp();						//��ȡϵͳ�����¶�
		ADDealSystemTemp();						//��ȡϵͳ�����¶�
		ADDealSystemTemp();						//��ȡϵͳ�����¶�	
		LCD12864Dis(SystemTemp);
		LCD12864Dis("#C");
		if(StempD>=STempH)
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningStempH);	//ϵͳ�¶ȹ���
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
			BeepWarning();						//����������
		}
		else if(StempD<=STempL)
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD12864WarningStempL);	//ϵͳ�¶ȹ���
			LCD12864_SetWindow(3,0);
			LCD12864Dis(LCD12863Line);
			BeepWarning();						//����������
		}
		else
		{
			LCD12864_SetWindow(2,0);
			LCD12864Dis(LCD1268HintMsgSystem);
			LCD12864_SetWindow(3,0);
			LCD12864Dis("TempLH:15#C-25#C");	//ϵͳ��������
		}
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;					//ʵ�ʶ�Ӧ�İ���ֵ��4		
	}
	HWWorkButton_04();
}
/***��ʱ�������״̬���***/
void HWWorkButton_16()							//��ʱ����
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoO2=1;  									//����������
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
		if(sumTime==UpO2Time)  					//�������Ĺ���ʱ��
		{
			motoO2=0;
			return;	
		}
		LCD12864_SetWindow(3,0);
		LCD12864Dis(LCD12864WorkingVO2TimeOut);	
		Value=HWValue[2]/16*10+HWValue[2]%16;
		if(Value==40)
		{
			memset(NOWTIME,0,sizeof(NOWTIME));	 //���ʱ������
			motoO2=0;
		   	break;	
		}				
	}
	HWWorkButton_04();
}
void HWWorkButton_17()							//��ʱ����
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();						
	motoFeed=1;									//����ι����
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);	 	
	LCD12864_SetWindow(1,0);
	LCD12864Dis(LCD12864WorkingFeed); 	
	Ds1302Init();						  		//��ʼ��Ds302ʱ�ӣ���00:00:00��ʼ��ʱ
	while(1)
	{
		Ds1302ReadTime();	
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864WorkingFeedTime);
		hour=NOWTIME[2]/16*10+NOWTIME[2]&0x0f;
		min=NOWTIME[1]/16*10+NOWTIME[1]&0x0f;
		s=NOWTIME[0]/16*10+NOWTIME[0]&0x0f;
		sumTime=hour*60*60+min*60+s;   			//����ʱ���ܺ�
		Timechar[0]=(int)(NOWTIME[2]/16)+'0'; 	Timechar[1]=(int)(NOWTIME[2]&0x0f)+'0'; 	Timechar[2]=':';
		Timechar[3]=(int)(NOWTIME[1]/16)+'0'; 	Timechar[4]=(int)(NOWTIME[1]&0x0f)+'0';  	Timechar[5]=':';
		Timechar[6]=(int)(NOWTIME[0]/16)+'0';	Timechar[7]=(int)(NOWTIME[0]&0x0f)+'0';
		LCD12864Dis(Timechar);					//ʵʱ��ʾʱ��
		if(sumTime==UpFeedTime)  				//Ͷ�ϻ����Ĺ���ʱ��
		{
			motoFeed=0;							//�ر�ι����
			return;	
		}
		LCD12864_SetWindow(3,0);
		LCD12864Dis(LCD12864WorkingFeedTimeOut);//��ʾ��ʾ��Ϣ
		Value=HWValue[2]/16*10+HWValue[2]%16;
		if(Value==40)
		{
			memset(NOWTIME,0,sizeof(NOWTIME));	 //���ʱ������
			motoFeed=0;
			break;	
		}
	}
	HWWorkButton_04();
}
/***�Ƕ�ʱ�������״̬���***/
void HWWorkButton_18()							//�����������
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoO2=1;
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);	
	while(1)
	{	
		LCD12864_SetWindow(0,0);  				//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
		LCD12864Dis(LCD12864Init);				//��ӡ������ʼ����Ϣ		
		LCD12864_SetWindow(1,0);		
		LCD12864Dis(LCD12864WorkingVO2);
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864VO2);
		DataDealTemp(Ds18b20ReadTemp()); 		//�¶����ݴ�����
		DataDealVO2(TempToVO2(tempD));	 		//�������ݴ�����
		LCD12864Dis(VO2Dchar);
		LCD12864Dis("MG"); 		
		LCD12864_SetWindow(3,0);
		if(VO2D>=5.5)	motoO2=0;	
		LCD12864Dis("���ﵽ5.00MG�ر�");
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;		
	}
	HWWorkButton_04();
}
void HWWorkButton_19()							//���µ������	  
{
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoTemp=1;
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);
	while(1)
	{
		LCD12864_SetWindow(0, 0);  				//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
		LCD12864Dis(LCD12864Init);				//��ӡ������ʼ����Ϣ
		LCD12864_SetWindow(1,0);		
		LCD12864Dis(LCD12864WorkingTemp);
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12864Temp);		
		DataDealTemp(Ds18b20ReadTemp()); 		//�¶����ݴ�����	  
		LCD12864Dis(tempDchar);
		LCD12864Dis("#C");
		LCD12864_SetWindow(3,0);
		if(tempD>=20.0)		motoTemp=0;	
		LCD12864Dis("�¶ȴﵽ20#C�ر�");
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40)	break;		
	}
	HWWorkButton_04();
}
void HWWorkButton_20()							//���ϵ������
{	
	LCD12864_WriteCmd(0x01);
	Delay100ms();
	motoFeed=1;
	LCD12864_SetWindow(0,0); 
	LCD12864Dis(LCD12864Init);
	while(1)
	{
		LCD12864_SetWindow(0, 0);  				//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
		LCD12864Dis(LCD12864Init);				//��ӡ������ʼ����Ϣ
		LCD12864_SetWindow(1,0);		
		LCD12864Dis(LCD12864WorkingFeed);
		LCD12864_SetWindow(2,0);
		LCD12864Dis(LCD12863Line);
		LCD12864_SetWindow(3,0);
		LCD12864Dis(LCD12863Line);
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��
		if(Value==40) break;	
	}
	HWWorkButton_04();		
}
/***�������ͨ�����ݲɼ����ܹ���ģ����תmain()����***/							  
void main(void)
{
	InitSystem();	 							//��ʼ��ϵͳ���ⲿ�ж�ϵͳ
	Init0Int();									//��ʼ���ⲿ�ж�ϵͳ0
	LCD12864_Init();							//��ʼ��LCD12864
	LCD12864_SetWindow(0,0);  					//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(0,0)
	LCD12864Dis(LCD12864Init);					//��ӡ������ʼ����Ϣ
	LCD12864_SetWindow(1,0);  					//LCD��ʼ��ʾλ�����Ͻ�(x,y)==(1,0)
	LCD12864Dis(LCD12864User);	
	LCD12864_SetWindow(2,0);
	LCD12864Dis(LCD12863Line);	
	LCD12864_SetWindow(3,0);
	LCD12864Dis(LCD12863Line);
	Delay100ms();		 						//��ʱ�ȴ�
	HWWorkButton_00();	 				 		//���뿪������
//	HWWorkButton_0x();							//���Դ���
	while(1)   									//������ƽ���
	{	
		u8 i; 
		Value=HWValue[2]/16*10+HWValue[2]%16;	//��ȡ�������4λ�͵�4λ,ת��Ϊ��Ӧ��ʮ���ƣ����㴦��			
		for(i=0;i<21;i++)			 			//����21������
		{
			if(Value==HWsmgduan[i])				//ת��Ϊ������
			{	
				LCD12864_WriteCmd(0x01);   		//���LCD12864����ʾ����
				switch(i)
				{
					case 0:	HWWorkButton_00();break; //����������ʾ
					case 1:	HWWorkButton_01();break; //ʱ�Ӳ��԰���
					case 2:	HWWorkButton_02();break; //����������
					case 3:	HWWorkButton_03();break; //��ʾ�����������״̬
					case 4:	HWWorkButton_04();break; //ϵͳ�������棬��д����������.
					case 5:	HWWorkButton_05();break; //NRF24L01����ͨ����ʾ����
					case 6:	HWWorkButton_06();break; //����չ
					case 7:	HWWorkButton_07();break; //����չ
					case 8:	HWWorkButton_08();break; //����չ
					case 9:	HWWorkButton_09();break; //��ʾ����ˮ���¶�����
					case 10:HWWorkButton_10();break; //����չ
					case 11:HWWorkButton_11();break; //����չ
					case 12:HWWorkButton_12();break; //��ʾ�¶�
					case 13:HWWorkButton_13();break; //��ʾ����
					case 14:HWWorkButton_14();break; //��ʾ����
					case 15:HWWorkButton_15();break; //��ʾϵͳ����ʵ���¶�
					case 16:HWWorkButton_16();break; //��ʱ����
					case 17:HWWorkButton_17();break; //��ʱ����
					case 18:HWWorkButton_18();break; //�����������
					case 19:HWWorkButton_19();break; //���µ������
					case 20:HWWorkButton_20();break; //���ϵ������
				} 							
			}							   
		}	
	}		
}
