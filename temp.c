#include"temp.h"
void Delay1ms(uint y)			//��ʱ1ms
{
	uint x;
	for(;y>0;y--)
		for(x=110;x>0;x--);
}

uchar Ds18b20Init()
{
	uchar i;
	DSPORT=0;					//����������480us~960us
	i=70;	
	while(i--);					//��ʱ642us
	DSPORT=1;					//Ȼ���������ߣ����DS18B20������Ӧ�Ὣ��15us~60us����������
	i=0;
	while(DSPORT)				//�ȴ�DS18B20��������
	{
		Delay1ms(1);
		i++;
		if(i>5)					//�ȴ�>5MS
		{
			return 0;			//��ʼ��ʧ��
		}
	
	}
	return 1;					//��ʼ���ɹ�
}
void Ds18b20WriteByte(uchar dat)
{
	uint i,j;
	for(j=0;j<8;j++)
	{
		DSPORT=0;	    		//ÿд��һλ����֮ǰ�Ȱ���������1us
		i++;
		DSPORT=dat&0x01;		//Ȼ��д��һ�����ݣ������λ��ʼ
		i=6;
		while(i--); 			//��ʱ68us������ʱ������60us
		DSPORT=1;				//Ȼ���ͷ����ߣ�����1us�����߻ָ�ʱ����ܽ���д��ڶ�����ֵ
		dat>>=1;
	}
}
uchar Ds18b20ReadByte()
{
	uchar byte,bi;
	uint i,j;	
	for(j=8;j>0;j--)
	{
		DSPORT=0;				//�Ƚ���������1us
		i++;
		DSPORT=1;				//Ȼ���ͷ�����
		i++;
		i++;					//��ʱ6us�ȴ������ȶ�
		bi=DSPORT;	 			//��ȡ���ݣ������λ��ʼ��ȡ
		byte=(byte>>1)|(bi<<7);						  
		i=4;					//��ȡ��֮��ȴ�48us�ٽ��Ŷ�ȡ��һ����
		while(i--);
	}				
	return byte;
}
void  Ds18b20ChangTemp()
{
	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);		//����ROM��������		 
	Ds18b20WriteByte(0x44);	    //�¶�ת������
}
void  Ds18b20ReadTempCom()
{	
				  
	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);	 	//����ROM��������
	Ds18b20WriteByte(0xbe);	 	//���Ͷ�ȡ�¶�����
}
int Ds18b20ReadTemp()
{
	int temp=0;
	uchar tmh, tml;
	Ds18b20ChangTemp();			//��д��ת������
	Ds18b20ReadTempCom();		//Ȼ��ȴ�ת������Ͷ�ȡ�¶�����
	tml=Ds18b20ReadByte();		//��ȡ�¶�ֵ��16λ���ȶ����ֽ�
	tmh=Ds18b20ReadByte();		//�ٶ����ֽ�
	temp=tmh;
	temp<<=8;
	temp|=tml;
	return temp;
}