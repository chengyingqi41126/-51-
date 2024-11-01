#include <REGX52.H>
#include "public.h"
#include "lcd1602.h"
#include <I2C.H>
#include <stdio.h>
#define  PCF8591 0x90    //PCF8591 ��ַ
sbit fmq=P1^0;
sbit led=P1^1;
sbit Sevro_moto_pwm=P3^7;	   	// ����ź��ߣ���ɫ��
sbit hongw=P3^0;
// ��������
unsigned char AD_CHANNEL;
unsigned long xdata  LedOut[8];
unsigned int  D[32];

u8 v_buf[5];
u8 a=0;
bit DACconversion(unsigned char sla,unsigned char c,  unsigned char Val);
bit ISendByte(unsigned char sla,unsigned char c);
unsigned char IRcvByte(unsigned char sla);

unsigned char pwm_val  = 0;//��������
unsigned char push_val = 14;//������У�����Լ��1.5MS �ź�
				
/**********************************************************************************************
**                              TIMER1�жϷ����Ӻ�������PWM�ź�
**********************************************************************************************/

void time1()interrupt 3   using 2
{	
	TH1=(65536-100)/256;	  //100US��ʱ
	TL1=(65536-100)%256;
	pwm_val++;
	if(pwm_val<=push_val)	  
		Sevro_moto_pwm=1;   //PWM�źŸߵ�ƽʱ��
	else 
		Sevro_moto_pwm=0;   //PWM�źŸߵ�ƽʱ��
	if(pwm_val>=100)
		pwm_val=0;
}
/*******************************************************************
DAC �任, ת������               
*******************************************************************/
bit DACconversion(unsigned char sla,unsigned char c,  unsigned char Val)
{
   Start_I2c();              //��������
   SendByte(sla);            //����������ַ
   if(ack==0)return(0);
   SendByte(c);              //���Ϳ����ֽ�
   if(ack==0)return(0);
   SendByte(Val);            //����DAC����ֵ  
   if(ack==0)return(0);
   Stop_I2c();               //��������
   return(1);
}

/*******************************************************************
ADC�����ֽ�[����]���ݺ���               
*******************************************************************/
bit ISendByte(unsigned char sla,unsigned char c)
{
   Start_I2c();              //��������
   SendByte(sla);            //����������ַ
   if(ack==0)return(0);
   SendByte(c);              //��������
   if(ack==0)return(0);
   Stop_I2c();               //��������
   return(1);
}

/*******************************************************************
ADC���ֽ����ݺ���               
*******************************************************************/
unsigned char IRcvByte(unsigned char sla)
{  unsigned char c;

   Start_I2c();          //��������
   SendByte(sla+1);      //����������ַ
   if(ack==0)return(0);
   c=RcvByte();          //��ȡ����0

   Ack_I2c(1);           //���ͷǾʹ�λ
   Stop_I2c();           //��������
   return(c);
}

/*******************************************************************************
* �� �� ��       : main
* ��������		 : ������
* ��    ��       : ��
* ��    ��    	 : ��
*******************************************************************************/
void main()
{	
  u16 v=0;
	lcd1602_init();//LCD1602��ʼ��
	lcd1602_show_string(0,0,"======LJT======");//��һ����ʾ
	lcd1602_show_string(0,1,"  loading...");//�ڶ�����ʾ
	delay_ms(1000);
	delay_ms(1000);
	lcd1602_show_string(0,0,"                ");//��һ����ʾ
	lcd1602_show_string(0,1,"                ");//�ڶ�����ʾ
	lcd1602_show_string(0,0,"100ppm");//��һ����ʾ
	lcd1602_show_string(0,1,"MQ-2:");//�ڶ�����ʾ
	TMOD=0X10;
	TH1=(65536-100)/256;	  //100US��ʱ
	TL1=(65536-100)%256;
	TR1= 1;
	ET1= 1;
	EA = 1;	
	push_val=13;	  //������У�����ִ��ָ�������ڣ�����PWM�ź������
	delay_ms(1000);   //��ʱ1S�ö��ת����λ�ã�ͣ��һ��
	while(1)
	{
		/********����AD-DA����*************/  
		switch(AD_CHANNEL)
		{
		case 0: ISendByte(PCF8591,0x41);
		D[0]=IRcvByte(PCF8591);  //ADC0 ģ��ת��1      ��������
		break;  

		case 1: ISendByte(PCF8591,0x42);
		D[1]=IRcvByte(PCF8591);  //ADC1  ģ��ת��2	  ��������
		break;  

		case 2: ISendByte(PCF8591,0x43);
		D[2]=IRcvByte(PCF8591);  //ADC2	ģ��ת��3	   ����
		break;  

		case 3: ISendByte(PCF8591,0x40);
		D[3]=IRcvByte(PCF8591);  //ADC3   ģ��ת��4	   �ɵ�0-5v
		break;  

		case 4: DACconversion(PCF8591,0x40, D[4]); //DAC	  ��ģת��
		break;

		}
	//	D[4]=D[0];  //��ģ������������ź� ͨ����ģת�����
		if(++AD_CHANNEL>4) AD_CHANNEL=0;
////////////////////////////////////////////////
		v=D[0];
		v_buf[0]=v/100+0x30;
		v_buf[1]=v%100/10+0x30;
		v_buf[2]=v%10%10+0x30;
		v_buf[3]='\0';
	  lcd1602_show_string(5,1,v_buf);
		if(v>100)
		{
			fmq=0;
			led=0;
			delay_ms(200);
			fmq=1;
			led=1;
			delay_ms(200);
			
		}
		else
		{
			fmq=1;
			led=1;	
		}
		
		if(hongw==0)
		{
			push_val=4;	  //�������תԼ90�ȣ�����ִ��ָ�������ڣ�����PWM�ź������
			delay_ms(500); //��ʱ500MS�ö��ת����λ��
		}
		else if(hongw==1)
		{
			push_val=22;	  //�����תԼ90�ȣ�����ִ��ָ�������ڣ�����PWM�ź������
			delay_ms(500);; //��ʱ500MS�ö��ת����λ��
		}

	}
}

