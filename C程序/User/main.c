#include <REGX52.H>
#include "public.h"
#include "lcd1602.h"
#include <I2C.H>
#include <stdio.h>
#define  PCF8591 0x90    //PCF8591 地址
sbit fmq=P1^0;
sbit led=P1^1;
sbit Sevro_moto_pwm=P3^7;	   	// 舵机信号线（橙色）
sbit hongw=P3^0;
// 变量定义
unsigned char AD_CHANNEL;
unsigned long xdata  LedOut[8];
unsigned int  D[32];

u8 v_buf[5];
u8 a=0;
bit DACconversion(unsigned char sla,unsigned char c,  unsigned char Val);
bit ISendByte(unsigned char sla,unsigned char c);
unsigned char IRcvByte(unsigned char sla);

unsigned char pwm_val  = 0;//变量定义
unsigned char push_val = 14;//舵机归中，产生约，1.5MS 信号
				
/**********************************************************************************************
**                              TIMER1中断服务子函数产生PWM信号
**********************************************************************************************/

void time1()interrupt 3   using 2
{	
	TH1=(65536-100)/256;	  //100US定时
	TL1=(65536-100)%256;
	pwm_val++;
	if(pwm_val<=push_val)	  
		Sevro_moto_pwm=1;   //PWM信号高电平时间
	else 
		Sevro_moto_pwm=0;   //PWM信号高电平时间
	if(pwm_val>=100)
		pwm_val=0;
}
/*******************************************************************
DAC 变换, 转化函数               
*******************************************************************/
bit DACconversion(unsigned char sla,unsigned char c,  unsigned char Val)
{
   Start_I2c();              //启动总线
   SendByte(sla);            //发送器件地址
   if(ack==0)return(0);
   SendByte(c);              //发送控制字节
   if(ack==0)return(0);
   SendByte(Val);            //发送DAC的数值  
   if(ack==0)return(0);
   Stop_I2c();               //结束总线
   return(1);
}

/*******************************************************************
ADC发送字节[命令]数据函数               
*******************************************************************/
bit ISendByte(unsigned char sla,unsigned char c)
{
   Start_I2c();              //启动总线
   SendByte(sla);            //发送器件地址
   if(ack==0)return(0);
   SendByte(c);              //发送数据
   if(ack==0)return(0);
   Stop_I2c();               //结束总线
   return(1);
}

/*******************************************************************
ADC读字节数据函数               
*******************************************************************/
unsigned char IRcvByte(unsigned char sla)
{  unsigned char c;

   Start_I2c();          //启动总线
   SendByte(sla+1);      //发送器件地址
   if(ack==0)return(0);
   c=RcvByte();          //读取数据0

   Ack_I2c(1);           //发送非就答位
   Stop_I2c();           //结束总线
   return(c);
}

/*******************************************************************************
* 函 数 名       : main
* 函数功能		 : 主函数
* 输    入       : 无
* 输    出    	 : 无
*******************************************************************************/
void main()
{	
  u16 v=0;
	lcd1602_init();//LCD1602初始化
	lcd1602_show_string(0,0,"======LJT======");//第一行显示
	lcd1602_show_string(0,1,"  loading...");//第二行显示
	delay_ms(1000);
	delay_ms(1000);
	lcd1602_show_string(0,0,"                ");//第一行显示
	lcd1602_show_string(0,1,"                ");//第二行显示
	lcd1602_show_string(0,0,"100ppm");//第一行显示
	lcd1602_show_string(0,1,"MQ-2:");//第二行显示
	TMOD=0X10;
	TH1=(65536-100)/256;	  //100US定时
	TL1=(65536-100)%256;
	TR1= 1;
	ET1= 1;
	EA = 1;	
	push_val=13;	  //舵机归中，机器执行指令有周期，所以PWM信号有误差
	delay_ms(1000);   //延时1S让舵机转到其位置，停留一下
	while(1)
	{
		/********以下AD-DA处理*************/  
		switch(AD_CHANNEL)
		{
		case 0: ISendByte(PCF8591,0x41);
		D[0]=IRcvByte(PCF8591);  //ADC0 模数转换1      光敏电阻
		break;  

		case 1: ISendByte(PCF8591,0x42);
		D[1]=IRcvByte(PCF8591);  //ADC1  模数转换2	  热敏电阻
		break;  

		case 2: ISendByte(PCF8591,0x43);
		D[2]=IRcvByte(PCF8591);  //ADC2	模数转换3	   悬空
		break;  

		case 3: ISendByte(PCF8591,0x40);
		D[3]=IRcvByte(PCF8591);  //ADC3   模数转换4	   可调0-5v
		break;  

		case 4: DACconversion(PCF8591,0x40, D[4]); //DAC	  数模转换
		break;

		}
	//	D[4]=D[0];  //把模拟输入采样的信号 通过数模转换输出
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
			push_val=4;	  //舵机向正转约90度，机器执行指令有周期，所以PWM信号有误差
			delay_ms(500); //延时500MS让舵机转到其位置
		}
		else if(hongw==1)
		{
			push_val=22;	  //舵机向反转约90度，机器执行指令有周期，所以PWM信号有误差
			delay_ms(500);; //延时500MS让舵机转到其位置
		}

	}
}

