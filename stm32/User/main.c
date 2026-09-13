#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Key.h"
#include "LED.h"
#include <stdio.h>
#include "string.h"
#include "Serial.h"

int main(void)
{
	uint8_t Keynum=0;
	_Bool lockstate=0;
	_Bool streamstate=0;
	_Bool cmd=0;
	_Bool mode=0;
	Key_Init();
	LED_Init();
	Serial_Init();
	while(1){
	
//采集区
		Keynum=Key_Scan();
		if(Keynum==1){
		
			//代表正在充电中
			streamstate=0;
			LED1_ON();
			LED2_OFF();
		}
		else if(Keynum==2){
		
			//代表充满
			streamstate=1;
			LED2_ON();
			LED1_OFF();
		}
		
//接收区
		if(Serial_RxFlag==1){
		
			if(strstr(Serial_RxPacket,"mode:true")){
			
				mode=1;
				cmd=lockstate;//切换保持原状，第一次，对应在应用端
			
			}
			if(strstr(Serial_RxPacket,"mode:false")){
			
				mode=0;
				cmd=lockstate;//切换保持原状，第一次，对应在应用端
			
			}
			if(strstr(Serial_RxPacket,"cmd:true")){
			
				cmd=1;
			
			}
			if(strstr(Serial_RxPacket,"cmd:false")){
			
				cmd=0;
			
			}
			
		
			Serial_RxFlag=0;//清空恢复
		}
		
//命令下达区
		if(mode){
		
			//手动模式
			if(cmd){
			
				lockstate=1;
			}
			else{
			
				lockstate=0;
			
			}
		
		}
		else{
		
			//自动模式
			if(streamstate){
			
				lockstate=1;
			}
			else{
			
				lockstate=0;
			
			}
		
		}
		
//接受命令执行区		

		if(lockstate){
		
			Lock_ON();
		
		}
		else{
		
			Lock_OFF();
		
		}
		
		
//发送区，发送四个值
		
		Serial_Printf("[streamstate:%s,lockstate:%s,cmd:%s,mode:%s]\r\n",streamstate?"true":"false"
										,lockstate?"true":"false",cmd?"true":"false",mode?"true":"false");
		

		
		Delay_ms(1000);
		
	}

	
}

//定时器中断函数
void TIM3_IRQHandler(void){

	//确定是中断类型
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET){
		
		
		Key_Tick();
		
		
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}



}

