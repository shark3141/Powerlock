#include "stm32f10x.h"                  // Device header
#include "Delay.h"

uint8_t KeyNum=0;


//void Key_Init(void)
//{
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	
//	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//}

//uint8_t Key_GetNum(void)
//{
//	uint8_t KeyNum = 0;
//	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
//	{
//		Delay_ms(20);
//		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);
//		Delay_ms(20);
//		KeyNum = 1;
//	}
//	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
//	{
//		Delay_ms(20);
//		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0);
//		Delay_ms(20);
//		KeyNum = 2;
//	}
//	
//	return KeyNum;
//}

void Key_Init(void){
	
	//总线配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	//按键配置
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//设置引脚上拉输入
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//配置定时器
	
	//TIM_InternalClockConfig(TIM3);
	
	//时基单元
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;//不分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=1000-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=72-1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;//高级定时器才有
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	//
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);//清除默认产生的事件更新
	
	//配置中断使能标志位
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	
	//使能定时器
	TIM_Cmd(TIM3,ENABLE);
	
	
}

uint8_t Key_Scan(void){

	uint8_t temp;
	if(KeyNum){
		//有按键
		temp=KeyNum;
		KeyNum=0;
		return temp;
	}
	
	//无按键
	return 0;

}

uint8_t Key_GetNum(void){

	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)==0){
	
		return 1;
	}
	else if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0){
	
		return 2;
	}
	
	//无按下
	return 0;

}

void Key_Tick(void){

	static uint8_t CurrState,PrevState;
	static uint8_t Count=0;
	Count++;
	if(Count>=20){
	
		
		PrevState=CurrState;
		CurrState=Key_GetNum();
		Count=0;
		
		//松手判断放在里面
		if(PrevState!=0&&CurrState==0){
		
			KeyNum=PrevState;
		}
	
	}
	

}



////定时器中断函数
//void TIM3_IRQHandler(void){

//	//确定是中断类型
//	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET){
//		
//		
//		
//		
//		
//		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
//	}



//}


