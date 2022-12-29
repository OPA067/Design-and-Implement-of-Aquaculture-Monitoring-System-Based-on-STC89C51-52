#include"lcd12864.h"
#include"reg52.h"
void LCD12864_Delay1ms(uint c)
{
    uchar a,b;
	for(;c>0;c--)
	    for(b=199;b>0;b--)
	        for(a=1;a>0;a--);
}
uchar LCD12864_Busy(void)
{
	uchar i=0;
	LCD12864_RS=0;   						//ѡ������
	LCD12864_RW=1;							//ѡ���ȡ
	LCD12864_EN=1;
	LCD12864_Delay1ms(1);
	while((LCD12864_DATAPORT&0x80)==0x80)	//����ȡ����ֵ
	{
		i++;
		if(i>100)
		{
			LCD12864_EN=0;
			return 0;	   					//�����ȴ�ʱ�䷵��0��ʾʧ��
		}
	}
	LCD12864_EN=0;
	return 1;
}
void LCD12864_WriteCmd(uchar cmd)
{
	uchar i;
	i=0;
	while(LCD12864_Busy()==0)
	{
		LCD12864_Delay1ms(1);
		i++;
		if(i>100)
		{
			return;	   						//�����ȴ��˳�
		}	
	}
	LCD12864_RS=0;     						//ѡ������
	LCD12864_RW=0;     						//ѡ��д��
	LCD12864_EN=0;     						//��ʼ��ʹ�ܶ�
	LCD12864_DATAPORT=cmd;   				//��������
	LCD12864_EN=1;		   					//дʱ��
	LCD12864_Delay1ms(5);
	LCD12864_EN=0;    					
}
void LCD12864_WriteData(uchar dat)
{
	uchar i;
	i=0;
	while(LCD12864_Busy()==0)
	{
		LCD12864_Delay1ms(1);
		i++;
		if(i>100)
		{
			return;	   						//�����ȴ��˳�
		}	
	}
	LCD12864_RS=1;     						//ѡ������
	LCD12864_RW=0;     						//ѡ��д��
	LCD12864_EN=0;     						//��ʼ��ʹ�ܶ�
	LCD12864_DATAPORT=dat;   				//��������
	LCD12864_EN=1;		  					//дʱ��
	LCD12864_Delay1ms(5);
	LCD12864_EN=0;    								
}
#ifdef LCD12864_PICTURE
uchar LCD12864_ReadData(void)
{
	uchar i,readValue;
	i=0;
	while(LCD12864_Busy()==0)
	{
		LCD12864_Delay1ms(1);
		i++;
		if(i>100)
		{
			return 0;	   					//�����ȴ��˳�
		}	
	}
	LCD12864_RS=1;       					//ѡ������
	LCD12864_RW=1;
	LCD12864_EN=0;
	LCD12864_Delay1ms(1);  					//�ȴ�
	LCD12864_EN=1;
	LCD12864_Delay1ms(1);
	readValue=LCD12864_DATAPORT;
	LCD12864_EN=0;
	return readValue;
}
#endif
void LCD12864_Init()
{
	LCD12864_PSB=1;	  		   				//ѡ��������
	LCD12864_RST=1;	  		   				//��λ
	LCD12864_WriteCmd(0x30);   				//ѡ�����ָ�����
	LCD12864_WriteCmd(0x0c);   				//��ʾ�����ع��
	LCD12864_WriteCmd(0x01);   				//���LCD12864����ʾ����
}
#ifdef LCD12864_PICTURE
void LCD12864_ClearScreen(void)
{
	uchar i,j;
	LCD12864_WriteCmd(0x34);	 			//������չָ�

	for(i=0;i<32;i++)			 			//��ΪLCD��������32������д��ʮ����
	{
		LCD12864_WriteCmd(0x80+i);			//��д��������Y��ֵ
		LCD12864_WriteCmd(0x80);		  	//��д�������X��ֵ
		for(j=0;j<32;j++)		  			//��������16λ��ÿλд�������ֽڵĵ����ݣ�Ҳ
		{						  			//��д��32����Ϊ��д�������ֽ�֮����������
			LCD12864_WriteData(0xFF);	  	//����1�����ԾͲ����ٴ�д���ַ�ˡ�
		}
	}
	LCD12864_WriteCmd(0x36);	 			//0x36��չָ������򿪻�ͼ��ʾ
	LCD12864_WriteCmd(0x30);	 			//�ָ�����ָ�
}

#endif
void LCD12864_SetWindow(uchar x, uchar y)
{
	uchar pos;
	if(x==0)	   	//��һ�еĵ�ַ��80H
	{
		x=0x80;
	}
		
	else if(x==1)  	//�ڶ��еĵ�ַ��90H
	{
		x=0x90;	
	}
	
	else if(x==2)  	//�����еĵ�ַ��88H
	{
		x=0x88;
	}
	else if(x==3) 	//�����еĵ�ַ��98H
	{
		x=0x98;
	}
	pos=x+y;
	LCD12864_WriteCmd(pos);
}