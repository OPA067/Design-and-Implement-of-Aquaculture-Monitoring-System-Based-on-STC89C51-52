#include"O2.h"
/*��Ϻ���*/
/*
	0.00137359*x^2-0.16400919*x+8.21849879	R^2=0.9985
*/
double TempToVO2(double temp)
{
//	double VO2=-0.0927*temp+7.4023;					//��С���˷����
//	double VO2=-2.1713*log10(temp)+11.911;			//�����������
	double VO2=0.00137359*temp*temp-0.16400919*temp+8.21849879;	//����ʽ���
	return VO2;
}