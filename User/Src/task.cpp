#include "stm32g4xx.h"
#include "tim.h"
#include "adc.h"
#include "dac.h"
#include <math.h>
#include <gui/screen_screen/ScreenView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>
#include <stdlib.h>
#include <string.h>
#include <usart.h>
#include <stdio.h>
////////////////////////Semaphores and occupancy////////////////////
uint8_t flag_start=0;
void Process_Init();
class Semaphores_t{
	public:
	bool isHRTIMidle;
	bool isTIM16idle;
	bool isDACidle;
	bool isSPI1idle;
	bool isSPI2idle;
	bool isADCidle;
	bool isLCDidle;
	void Init(){
		isHRTIMidle=1;
		isTIM16idle=1;
		isDACidle=1;
		isSPI1idle=1;
		isSPI2idle=1;
		isADCidle=1;
		isLCDidle=1;
	}
}occupancy;
class Process_t{
	public:
		Semaphores_t use_what;
	  uint16_t estimate_tim16_cycles;
	  uint8_t priority;
	  void Init(uint8_t useconfig,uint16_t cycles,uint8_t prio){
			if((useconfig&0x01)==1)use_what.isHRTIMidle=0;
			if((useconfig&0x02)==1)use_what.isTIM16idle=0;
			if((useconfig&0x04)==1)use_what.isDACidle=0;
			if((useconfig&0x08)==1)use_what.isSPI1idle=0;
			if((useconfig&0x10)==1)use_what.isSPI2idle=0;
			if((useconfig&0x20)==1)use_what.isADCidle=0;
			estimate_tim16_cycles=cycles;
			priority=prio;
		}
		bool SetS(){
			if((occupancy.isADCidle|use_what.isADCidle)>=1)occupancy.isADCidle=use_what.isADCidle;
			else return 0;
			if((occupancy.isDACidle|use_what.isDACidle)>=1)occupancy.isDACidle=use_what.isDACidle;
			else return 0;
			if((occupancy.isSPI1idle|use_what.isSPI1idle)>=1)occupancy.isSPI1idle=use_what.isSPI1idle;
			else return 0;
			if((occupancy.isSPI2idle|use_what.isSPI2idle)>=1)occupancy.isSPI2idle=use_what.isSPI2idle;
			else return 0;
			if((occupancy.isHRTIMidle|use_what.isHRTIMidle)>=1)occupancy.isHRTIMidle=use_what.isHRTIMidle;
			else return 0;
			return 1;
		}
		void DeS(){
			occupancy.isADCidle=!use_what.isADCidle;
			occupancy.isDACidle=!use_what.isDACidle;
			occupancy.isSPI1idle=!use_what.isSPI1idle;
			occupancy.isSPI2idle=!use_what.isSPI2idle;
			occupancy.isHRTIMidle=!use_what.isHRTIMidle;
		}
	  void (*process)(void);
};
Process_t Control_CC;
Process_t Control_CP;
Process_t Control_CR;
Process_t Control_Cp;
//////////////////////////////////////////////////////////////////
extern "C" void touchgfxSignalVSync(void);
int cycles_t16=0;
int dcycles_t16=0;
/////////////////////////Timer and Timer Callbacks////////////////////
void TIM16_Callback();
void DrawWildCards();
void getTouchScreen();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
		if(flag_start==1){
			Process_Init();
			flag_start=0;
		}
  }
	if(htim->Instance==TIM16){
		occupancy.isTIM16idle=0;
		if(cycles_t16%9==0){
		TIM16_Callback();
		}
		if(cycles_t16%77==0){
		getTouchScreen();
			
		}
		if(cycles_t16%101==0){
		touchgfxSignalVSync();
			
		}
		if(cycles_t16%291==0){
			dcycles_t16++;
			DrawWildCards();
			cycles_t16=0;
		}
		cycles_t16++;
		occupancy.isTIM16idle=1;
	}
}

/////////////////////////////////////////////////////////////////////
//////////////////Global Vars////////////////////////////////////////
uint16_t adc1[49];
float current_ADC1;
float volt_ADC1;
/////////////////////////////////////////////////////////////////////
///////////////////////////Sampling//////////////////////////////////
class Corr_t{
	public:
	float zero_factor=0;
	float ratio_factor=1.0f;
};
Corr_t Current_corr;
Corr_t Volt_corr;
void ADC_Sample(){
	//HAL_ADC_Start_DMA(&hadc1,(uint32_t*)(adc1),48);
	uint32_t adc1_prs[3]={0};
	for(int i=0;i<48;i+=1){
		adc1_prs[i%3]+=adc1[i%3];
	}
		adc1_prs[0]/=16;
	adc1_prs[1]/=16;
	adc1_prs[2]/=16;
	int currentraw=adc1_prs[1];
	currentraw-=32767;
	current_ADC1=fmaxf(((((currentraw/65535.0f)*3.0f*100.0f)+Current_corr.zero_factor)*Current_corr.ratio_factor),-0.005);
	volt_ADC1=(((adc1_prs[0]-32767.0f)/65536.0f)*3.0f*16.0f*(-1.7605f)+Volt_corr.zero_factor)*Volt_corr.ratio_factor;
//	HAL_ADC_Stop_DMA(&hadc1);
}
/////////////////////////////////PID//////////////////////
int last_target222=0;
int last_out=0;
float F_factor=2000;
extern float target_current;
int32_t current_P(float target,float now,float p,int last){
	int now_out=last;
	int diff=target-now;
	now_out+=(target-last_target222)*F_factor;
	last_target222=target;
	now_out+=diff*p;
	if(now_out>233415)now_out=233415;
	if(now_out<0)now_out=0;
	return now_out;
}
class LEPID_t{
	public:
	float Klog=0;
	float Kexp;
	float Kpro=0;
	float Kint;
	float Kdif;
	float max_out=233415;
	float last_out=0;
	float now_value;
	float last_value=100000;
	float integral=0;
	float differential=0;
	float integral_limit;
	float target=0;
	float last_target=0;
	float ff_factor=2000;
	uint32_t output;
};
float fsgn(float k){
	if(k>=0)return 1;
	else return -1;
}
bool dac_allowed=1;
void LEPID_Execute(LEPID_t& lepid){
	float result=0;
	result=lepid.last_out;
	if(dac_allowed){
	if(lepid.target!=lepid.last_target){
		if(fabsf(lepid.target-lepid.last_target)*lepid.ff_factor<40000){
		result+=(lepid.target-lepid.last_target)*lepid.ff_factor;
			lepid.last_target=lepid.target;
		}else{
			result+=fsgn(lepid.target-lepid.last_target)*100.0f*lepid.ff_factor;
			lepid.last_target+=(40000.0f/lepid.ff_factor)*fsgn(lepid.target-lepid.last_target);
		}
	}
	float error=lepid.target-lepid.now_value;
	result+=logf(fabs(error)+1)*fsgn(error)*lepid.Klog;
	result+=powf(2,fminf(fabs(error*lepid.Kexp),15))*fsgn(error);
	result+=error*lepid.Kpro;
	lepid.integral+=error;
	if(fabs(lepid.integral)>lepid.integral_limit){
		lepid.integral=fsgn(lepid.integral)*lepid.integral_limit;
	}
	result+=lepid.integral*lepid.Kint;
	lepid.differential=lepid.now_value-lepid.last_value;
	result+=lepid.differential*lepid.Kdif;
	if(result>lepid.max_out){
		result=lepid.max_out;
	}
	if(result<0){
		result=0;
	}
}
	lepid.output=result;
	lepid.last_out=result;
	lepid.last_value=lepid.now_value;

}
	

//////////////////////////////DAC Output/////////////////////////

void DAC_Update(uint32_t value,uint16_t *DAC_H, uint16_t *DAC_L,uint16_t DAC_H_Last, uint16_t DAC_L_Last) {

    if (value > 233415) value = 233415;
    uint16_t H0 = value / 56;
    uint16_t L0 = value % 56;
    if (H0 > 4095) {
        uint16_t excess = H0 - 4095;
        H0 = 4095;
        L0 = L0 + excess * 56;
        if (L0 > 4095) {
            H0 += L0 / 4096;
            L0 = L0 % 4096;
            if (H0 > 4095) H0 = 4095;
        }
    }
    int max_borrow = (4095 - L0) / 56;
    if (max_borrow > H0) max_borrow = H0;
    uint16_t H = H0 - max_borrow;
    uint16_t L = L0 + max_borrow * 56;

    int32_t L_from_H = (int32_t)value - (int32_t)DAC_H_Last * 56;
    if (L_from_H >= 0 && L_from_H <= 4095) {
        *DAC_H = DAC_H_Last;
        *DAC_L = (uint16_t)L_from_H;
        return;
    }

    int32_t rem = (int32_t)value - (int32_t)DAC_L_Last;
    if (rem >= 0 && rem % 56 == 0) {
        int32_t H_from_L = rem / 56;
        if (H_from_L >= 0 && H_from_L <= 4095) {
            *DAC_H = (uint16_t)H_from_L;
            *DAC_L = DAC_L_Last;
            return;
        }
    }

    *DAC_H = H;
    *DAC_L = L;
}
void WriteCurrentDAC(int32_t value){
if(dac_allowed){
	if(value>233415)value=233415;
	if(value<0)value=0;
	uint16_t dach,dacl;
	static uint16_t dach_last,dacl_last;
	DAC_Update(value,&dach,&dacl,dach_last,dacl_last);
	if(dacl!=dacl_last){
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,dacl);
	}
	if(dach!=dach_last){
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,dach);
	}
	dach_last=dach;
	dacl_last=dacl;
}else{
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,0);
		HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,0);
}
}
///////////////////////////////////////////////////////////////
enum Mode_t{
	CC,
	CP,
	CR,
	COMPLI,
	ERRORMODE
};
Mode_t mode;
LEPID_t cc_lepid;
LEPID_t cp_lepid;
LEPID_t cs_lepid;
float compli_states=0;
void Set_CC_LEPID(){
	cc_lepid.Klog=24;
	cc_lepid.Kpro=10;
}
void Set_CP_LEPID(){
	cp_lepid.Klog=7;
	cp_lepid.Kpro=0;
	cp_lepid.Kdif=4;
//	cp_lepid.Kexp=0.001;
}
void Set_CR_LEPID(){
	cs_lepid.Klog=0;
	cs_lepid.Kpro=2000;
	cs_lepid.Kdif=4000;
	cs_lepid.Kint=0.12;
	cs_lepid.target=1000;
}
void Set_IV_Corr(){
	Current_corr.zero_factor=0.5;
	Current_corr.ratio_factor=1.006;//1.03115 //不要用MT-1220 3位半校准！！！
}
void CC_funct(){
	ADC_Sample();
	cc_lepid.now_value=current_ADC1;
	if(current_ADC1<0.1&&cc_lepid.last_out<100000)cc_lepid.last_out=100000;
	LEPID_Execute(cc_lepid);
	if(cc_lepid.output==233415&&cc_lepid.now_value<0.1)cc_lepid.last_out=140000;//冲顶饱和复活
	WriteCurrentDAC(cc_lepid.output);
}
void CP_funct(){
	ADC_Sample();
	cp_lepid.now_value=volt_ADC1*current_ADC1;
	cp_lepid.ff_factor=2000.0f/(volt_ADC1+2);
	if(volt_ADC1<0.05)cp_lepid.last_out=80000;
	LEPID_Execute(cp_lepid);
		if(volt_ADC1<0.05)cp_lepid.output=80000;
	if(cp_lepid.output==233415&&current_ADC1<0.01)cp_lepid.last_out=140000;//冲顶饱和复活
	WriteCurrentDAC(cp_lepid.output);
}
float r_accum=0;
float CR_target=1000;
uint16_t Cp_timer=0;
void CR_funct(){
	ADC_Sample();
	cs_lepid.now_value=fmaxf(current_ADC1,0.01f)/fmaxf(volt_ADC1,0.01f);
	r_accum+=cs_lepid.now_value*0.2;
	r_accum/=1.2;
	cs_lepid.now_value=r_accum;
	//cs_lepid.ff_factor=100;//2000.0f/(volt_ADC1+2);
	cs_lepid.target=1.00f/CR_target;
/*	if(cr_lepid.now_value<cr_lepid.target-100)cr_lepid.output-=10;
	if(cr_lepid.now_value>cr_lepid.target+100)cr_lepid.output+=10;
		if(cr_lepid.now_value<cr_lepid.target)cr_lepid.output-=1;
	if(cr_lepid.now_value>cr_lepid.target)cr_lepid.output+=1;
	if(cr_lepid.output>233415)cr_lepid.output=233415;
	if(cr_lepid.output<300)cr_lepid.output=300;*/
	LEPID_Execute(cs_lepid);

	WriteCurrentDAC(cs_lepid.output);
}
float pre_target=0;
bool Cp_isfirst=0;
uint16_t second_param=0;
void Cp_funct(){
	if(second_param==1){
		Cp_timer++;
		if(Cp_isfirst){
		pre_target=cc_lepid.target;
			Cp_isfirst=0;
		}
		cc_lepid.target=sinf((float)(Cp_timer%62)/20.0f)*pre_target;
		if(Cp_timer>=62)Cp_timer=0;
		Control_CC.SetS();
	Control_CC.process();
	Control_CC.DeS();
	}
}
void Process_Init(){
	Control_CC.Init(0x02+0x04+0x20,1,16);
	Control_CC.process=CC_funct;
	Control_CP.Init(0x02+0x04+0x20,1,16);
	Control_CP.process=CP_funct;
	Control_CR.Init(0x02+0x04+0x20,1,16);
	Control_CR.process=CR_funct;
	Control_Cp.Init(0x02+0x04+0x20,1,16);
	Control_Cp.process=Cp_funct;
	occupancy.Init();
	Set_CC_LEPID();
	Set_CP_LEPID();
	Set_CR_LEPID();
	Set_IV_Corr();
	HAL_TIM_Base_Start_IT(&htim16);
}
void TIM16_Callback(){
	switch(mode){
		case CC:{
	Control_CC.SetS();
			Cp_isfirst=1;
	Control_CC.process();
	Control_CC.DeS();
			break;
		}
		case CP:{
	Control_CP.SetS();
	Control_CP.process();
	Control_CP.DeS();
			break;
		}
		case CR:{
	Control_CR.SetS();
	Control_CR.process();
	Control_CR.DeS();
			break;
		}
		case COMPLI:{
	Control_Cp.SetS();
	Control_Cp.process();
	Control_Cp.DeS();
			break;
		}
		case ERRORMODE:{
	Control_CC.SetS();
	Control_CC.process();
	Control_CC.DeS();
			break;
		}
	}
}
uint16_t param1=0;

Mode_t last_mode;
float disp_amp,disp_volt,disp_watt,disp_set;

void DrawWildCards(){
	param1=mode;
	
	disp_amp=current_ADC1;
	disp_volt=volt_ADC1;
	disp_watt=current_ADC1*volt_ADC1;
  switch(mode){
	case CC:
		disp_set=cc_lepid.target;break;
  case CP:
		disp_set=cp_lepid.target;break;
	case CR:
		disp_set=CR_target;break;
	case COMPLI:{
		second_param=((int)(fabsf(compli_states)*2))%4;
		}
	case ERRORMODE:
		break;
  
  }

}
extern SPI_HandleTypeDef hspi2;
extern LPTIM_HandleTypeDef hlptim1;
extern USART_HandleTypeDef husart1;
extern "C" uint8_t Touch_Read(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin, uint16_t *x, uint16_t *y);
uint16_t xaxis,yaxis,xaxis_pre,yaxis_pre;
uint16_t last_encoder=0;
int32_t last_encoder_val=0;
float target_val=0;
bool onoffstates=0;
char usart_results[20];
void getTouchScreen()
{
	if(Touch_Read(&hspi2,GPIOF,GPIO_PIN_0,&xaxis_pre,&yaxis_pre)){
		xaxis=xaxis_pre;
		yaxis=yaxis_pre;
	}
	if(onoffstates==1){
	if(yaxis<16000){
	if(xaxis<9000){
		last_mode=mode;
		mode=CC;
		
		target_val=0;
	}else if(xaxis<16000){
		last_mode=mode;
		mode=CP;
		target_val=0;
	}else if(xaxis<22000){
		last_mode=mode;
		mode=CR;
		target_val=0;
	}else if(xaxis<32000){
		mode=COMPLI;
	}
}
	}
	if(yaxis<20500&&yaxis>17095&&xaxis>2483&&xaxis<8400){
		onoffstates=!onoffstates;
	}
if(HAL_LPTIM_ReadCounter(&hlptim1)!=0){
	int32_t encoder_val=HAL_LPTIM_ReadCounter(&hlptim1);
	if(encoder_val>32767)encoder_val-=65536;
	target_val-=0.1f*(encoder_val-last_encoder_val);
	last_encoder_val=encoder_val;
	//__HAL_LPTIM_RESET_COUNTER(&hlptim1);
}
if(target_val<0)target_val=0;
if(onoffstates==1){
	dac_allowed=1;
}else if(onoffstates==0){
	dac_allowed=0;
}
switch(mode){
	case CC:
		cc_lepid.target=target_val;break;
  case CP:
		cp_lepid.target=10.0f*target_val;break;
	case CR:
		CR_target=1/(target_val/10+0.001f);break;
	case COMPLI:
		compli_states=target_val;
		break;
	case ERRORMODE:
		cc_lepid.target=-90;break;
}
snprintf(&usart_results[1],18,"%.4f/%.4f",current_ADC1,volt_ADC1);
usart_results[0]=0x5A;
HAL_USART_Transmit(&husart1,(uint8_t*)(usart_results),20,9);

}
	