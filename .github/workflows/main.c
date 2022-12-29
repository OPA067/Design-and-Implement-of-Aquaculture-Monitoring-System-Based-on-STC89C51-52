/***����ͷ�ļ�***/
#include"reg52.h"
#include"nRF.h"
#include"temp.h"
#include"O2.h"
/***��ر���˵��***/
typedef unsigned int u16;	  	//���������ͽ�����������
typedef unsigned char u8;	  	//���������ͽ�����������
u8 flag=0;					  	//�������ݽ��ձ�ʶ�ַ�
sbit k1=P3^1;  					//���������������ʹӻ��Ĺؼ�
u8 ChooseZC=0;					//�����ʹӻ����ñ�־
double NRFtemp;					//��¼�ӻ��¶����ݺ���������
/***��ʱ����1s***/
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

/***���NRF24L01ͨ���Ƿ�����***/
u8 bdata sta;	  			  	//��������ͨ�ŵı�ʶ�ַ�
sbit RX_DR	=sta^6;
sbit TX_DS	=sta^5;
sbit MAX_RT	=sta^4;
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

/***�¶����ݴ����� ���ܣ���ȡ�¶����ݵĸ���λ����t1t2.t3t4.#C;***/
void DataDealTemp(int temp) 	 
{
   	double tp;
	int t1,t2,t3,t4;  
	tp=temp;					//��Ϊ���ݴ�����С�������Խ��¶ȸ���һ�������ͱ���
	temp=tp*0.0625*100+0.5;	    //+0.5����������
	t1=temp%10000/1000;			//�¶�ʮλ
	TX_BUF[0]=t1+'0';	
	t2=temp%1000/100;		   	//�¶ȸ�λ
	TX_BUF[1]=t2+'0';	
	TX_BUF[2]='.';
	t3=temp%100/10;			  	//�¶�ʮ��λ
	TX_BUF[3]=t3+'0';
	t4=temp%10/1;				//�¶Ȱٷ�λ
	TX_BUF[4]=t4+'0';			//���ϵȼ�==memset(TX_BUF,t);
	NRFtemp=t1*10+t2+t3*0.1+t4*0.01;		//��¼�¶�����
}
/***�������ݴ����� ���ܣ���ȡ�������ݵĸ���λ����t1t2.t3t4.MG;***/
void DataDealVO2() 	 
{
	int vo2=TempToVO2(NRFtemp)*100;
	TX_BUF[5]=vo2/1000+'0';	   //��������ʮλ
	TX_BUF[6]=vo2/100%10+'0';  //�������ݸ�λ
	TX_BUF[7]='.';			   //С����
	TX_BUF[8]=vo2/10%10+'0';   //��������ʮ��λ
	TX_BUF[9]=vo2%10+'0';	   //�������ݰٷ�λ
}
/***�����������û�ȷ�������ʹӻ�***/
void CheckButtons()
{	
	if(k1==0)									//�û�����K1������˵����ǰ�����µĵ�Ƭ��Ϊ���ݷ��Ͷˣ�����һ��Ϊ���ݽ��ն�	            
	{
		delay_ms(20);							//��ʱ
		if(k1==0)			    
		{
			DataDealTemp(Ds18b20ReadTemp()); 	//�¶����ݴ�����
			DataDealVO2();						//�������ݴ�����
			TX_Mode(TX_BUF);				 	//��nRF24L01����Ϊ����ģʽ����������
			Check_ACK(1);               	 	//�ȴ�������ϣ����TX_FIFO
			RX_Mode();			        	 	//����Ϊ����ģʽ
			while(!k1);							//����
			ChooseZC=1;							//���ñ�ʶ
		}	
	}
	if(ChooseZC==1)
	{
		Delay1000ms();							//��ʱ2000ms�ٽ������ݴ��䣬���ʵ����ӵ�һ����
		DataDealTemp(Ds18b20ReadTemp()); 		//�¶����ݴ�����
		DataDealVO2();							//�������ݴ�����
		TX_Mode(TX_BUF);						//��nRF24L01����Ϊ����ģʽ����������
		Check_ACK(1);               			//�ȴ�������ϣ����TX_FIFO
		RX_Mode();			        			//����Ϊ����ģʽ		
	}	
}
/***���������֣��������ݿ��ƴ���***/
void main(void)
{
	init_io();		              							//��ʼ��IO
	RX_Mode();		             	 						//����Ϊ����ģʽ
	while(1)
	{
		CheckButtons();           							//����ɨ��
		sta=SPI_Read(STATUS);	  							//��״̬�Ĵ���
	    if(RX_DR)				  							//�ж��Ƿ���ܵ�����
		{
			SPI_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);//��RX FIFO��������
			flag=1;
		}
		SPI_RW_Reg(WRITE_REG+STATUS,sta);  					//���RX_DS�жϱ�־
	}
}

