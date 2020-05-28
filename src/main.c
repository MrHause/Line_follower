#include <math.h>
#include "stm32f10x.h"
#include "uart/uart.h"

volatile uint32_t timer_ms = 0,flag=0,tik=0;

void SysTick_Handler()
{
	if(tik)
	{
		tik--;
	}
}

void delay_ms(int time)
{
	 timer_ms = time;
	 while (timer_ms) {};
}

void left_forward(int v)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
	TIM_SetCompare2(TIM4, v);
}
void right_forward(int v)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	TIM_SetCompare1(TIM4, v);
}
void right_stop_fast()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
}
void left_stop_fast()
{
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
	GPIO_SetBits(GPIOA, GPIO_Pin_12);
}
void right_stop_slow()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}
void left_stop_slow()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}
void right_back(int v)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
	TIM_SetCompare1(TIM4, v);
}
void left_back(int v)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	GPIO_SetBits(GPIOA, GPIO_Pin_12);
	TIM_SetCompare2(TIM4, v);
}




int adc_read(int channel)
{
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_13Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != SET);

	return ADC_GetConversionValue(ADC1);
}
void adc_init()
{
	 ADC_InitTypeDef adc;
	 GPIO_InitTypeDef gpio;
	 RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	 gpio.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	 gpio.GPIO_Mode = GPIO_Mode_AIN;
	 GPIO_Init(GPIOA, &gpio);

	 ADC_StructInit(&adc);
	 adc.ADC_ContinuousConvMode = DISABLE;
	 adc.ADC_NbrOfChannel = 1;
	 adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	 ADC_Init(ADC1, &adc);

	 //ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71Cycles5);
	 ADC_Cmd(ADC1, ENABLE);

	 ADC_ResetCalibration(ADC1);
	 while (ADC_GetResetCalibrationStatus(ADC1));

	 ADC_StartCalibration(ADC1);
	 while (ADC_GetCalibrationStatus(ADC1));

	 ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

void DC_init()
{
	GPIO_InitTypeDef gpio;
	//piny pwm
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &gpio);
	//piny kierunku silnik prawy
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &gpio);
	//piny kierunku silnik lewy
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &gpio);
	//stan niski na wszystkich silnikach
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);

	//czujniki skrajne lewy/prawy
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_3|GPIO_Pin_4;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &gpio);
}

void pwm_init()
{
	TIM_TimeBaseInitTypeDef tim;
	TIM_OCInitTypeDef  channel;
	NVIC_InitTypeDef nvic;

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Prescaler = 720 - 1;
	tim.TIM_Period = 1000 - 1;
	TIM_TimeBaseInit(TIM4, &tim);

	TIM_OCStructInit(&channel);
	channel.TIM_OCMode = TIM_OCMode_PWM1;
	channel.TIM_OutputState = TIM_OutputState_Enable;

	TIM_OC1Init(TIM4, &channel);
	TIM_OC2Init(TIM4, &channel);

	TIM_Cmd(TIM4, ENABLE);
	 nvic.NVIC_IRQChannel = TIM4_IRQn;
	 nvic.NVIC_IRQChannelPreemptionPriority = 0;
	 nvic.NVIC_IRQChannelSubPriority = 0;
	 nvic.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&nvic);
}
void encode_init()
{
	/*
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	GPIO_InitTypeDef gpio;
	TIM_TimeBaseInitTypeDef timbase;
	gpio.GPIO_Pin = GPIO_Pin_6;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);

	timbase.TIM_Period=65535;
	timbase.TIM_Prescaler = 0;
	timbase.TIM_ClockDivision = 0;
	timbase.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimBaseInit(TIM3, &timbase);
	TIM_TIxEcternalCloclConfig(TIM3,TIM_TIxExternalCLK1Source_TI1,TIM_ICPolarity_Falling,1);
	TIM_Cmd(TIM3,ENABLE);
	*/
}
int main(void)
{

	//zegary
	SysTick_Config(SystemCoreClock / 1000);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //zegar dla timera 4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //zegar dla adc

	uart_init();
	adc_init();
	DC_init();
	pwm_init();

	uint32_t warunek=0,flag_turn=0,skret=0;

	 while (1) {
		 uint32_t cnt = TIM_GetCounter(TIM3);
		 uint32_t sens_right2 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
		 uint32_t sens_left2 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
		 //uint32_t sens_right = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
		 //uint32_t sens_left = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
		 uint32_t sens_right = adc_read(ADC_Channel_0);
		 uint32_t sens_left = adc_read(ADC_Channel_1);
		 uint32_t v_max = 500;
		 uint32_t v_slow = 230;

		 if (sens_left > 1000 && sens_right > 1000 && sens_right2==1 && sens_left2==1 && flag_turn==0 && flag_turn == 0) { //brak linii
			 skret=0;
		 } else if (sens_left2==1 && sens_left < 1000 && sens_right > 1000 && sens_right2 == 1 && (flag_turn==0||flag_turn==2)) {
			 skret=1;
		 } else if (sens_left2==1 && sens_left > 1000 && sens_right < 1000 && sens_right2==1 && (flag_turn==0||flag_turn==4)) {
			 skret=2;
		 }else if(sens_right2==0 && sens_left2==1 ){ //ostry prawo
			 skret=3;
		 } else if(sens_left2==0 &&  sens_right2==1){ //ostry lewy
			 skret=4;
		 }


		 if (skret==0) { //brak linii
			 if(warunek!=1){
				 flag_turn=0;
				 warunek=1;
				 tik=60;
				 left_forward(v_max);
				 right_forward(v_max);
			 }
			 else
			 {
				 if(tik>2)
				 {
					 left_forward(v_max);
					 right_forward(v_max);
				 }
				 else
				 {
					 left_forward(v_slow);
					 right_forward(v_slow);
				 }
			 }
		 } else if (skret==1) { //lewy linia
			 if(warunek!=2){
				 flag_turn=0;
				 warunek=2;
				 tik=60;
				 left_stop_fast();
				 right_forward(v_max);
			 }
			 else{
				 if(tik>2)
				 {
					 right_forward(v_max);
				 }
				 else{
					 right_forward(v_slow);
				 }
			 }
		 } else if (skret==2) { //prawy linia
			 if(warunek!=3){
				 flag_turn=0;
				 warunek=3;
				 tik=60;
				 right_stop_fast();
				 left_forward(v_max);
			 }
			 else{
				 if(tik>2){
					 left_forward(v_max);
				 }
				 else{
					 left_forward(v_slow);
				 }
			 }
		 }else if(skret==3 ){ //ostry prawo
			 right_stop_fast();
			 if(warunek!=4){

				 if(flag_turn==0)
					 flag_turn=3;
				 else if(flag_turn==1)
					 flag_turn=2;

				 warunek=4;
				 tik=62;
				 //right_back(v_max+50);
				 //right_forward(0);
				 left_forward(v_max);
			 }
			 else{
				 if(tik>2){

					 //right_back(v_max+50);
					 //right_forward(0);
					 left_forward(v_max);
				 }
				 else{
					 //right_stop_fast();
					 //right_back(v_slow+20);
					 //right_forward(0);
					 left_forward(v_slow);
				 }
			 }
		 } else if(skret==4){ //ostry lewy
			 left_stop_fast();
			 if(warunek!=5){
				 if(flag_turn==0)
					 flag_turn=1;
				 else if(flag_turn==3)
					 flag_turn=4;

				 warunek=5;
				 tik=62;

				 //left_back(v_max+50);
				 //left_forward(0);
				 right_forward(v_max);
			 }
			 else{
				 if(tik>2){
					 //left_stop_fast();
					 //left_back(v_max+50);
					 //left_forward(0);
					 right_forward(v_max);
				 }
				 else{
					 //left_stop_fast();
					 //left_back(v_slow+20);
					 //left_forward(0);
					 right_forward(v_slow);
				 }
			 }
		 }

		 uart_putInt(cnt);
		 //uart_putInt(sens_right2);
	 } //end while
} //end main
