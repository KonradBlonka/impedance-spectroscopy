/*!
 *****************************************************************************
 @file:    AD5940Main.c
 @author:  Neo Xu
 @brief:   Used to control specific application and process data.
 -----------------------------------------------------------------------------

Copyright (c) 2017-2019 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 
*****************************************************************************/
/** 
 * @addtogroup AD5940_System_Examples
 * @{
 *  @defgroup BioElec_Example
 *  @{
  */
#include "ad5940.h"
#include "AD5940.h"
#include <stdio.h>
#include "string.h"
#include "math.h"
#include "BodyImpedance.h"
#include <cstdio> //dodalem

#define APPBUFF_SIZE 512

uint32_t AppBuff[APPBUFF_SIZE];

/* It's your choice here how to do with the data. Here is just an example to print them to UART */
int32_t BIAShowResult(uint32_t *pData, uint32_t DataCount)
{
	//AD5940_ReadWriteNBytes
  float freq;
	
  //fImpPol_Type *pImp = (fImpPol_Type*)pData; //to jest dobre ale testuje pOut
  AppBIACtrl(BIACTRL_GETFREQ, &freq);

	
	AppBIACfg_Type *pBIACfg;
	AppBIAGetCfg(&pBIACfg);

	//dopisuje
 // AppBIACfg_Type *pBIACfg; 
  //AppBIAGetCfg(&pBIACfg);
	
	//to jest dobrze ale testuje pOut
	//fImpCar_Type *pReIz = (fImpCar_Type*)pData;	//wywolanie klasy, pRI czyli rzeczywiste i urojone zamiast pSrcData
	
	fImpPol_Type * const pOut = (fImpPol_Type*)pData;
	
		
	for(int i=0;i<DataCount;i++)
  {
		
		//to jest dobrze ale testuje pOut
//		fImpCar_Type res; //dobrze
//		res = pReIz[i]; //dobrze
//		printf("Re1 = %f ohm , Iz1 = %f ohm", res.Real, res.Image); //dobrze
//    printf("RzMag: %f Ohm , RzPhase: %f\n",pImp[i].Magnitude,pImp[i].Phase*180/MATH_PI); //dobrze
		printf("%.2f, %f, %f\n", freq,pOut[i].Magnitude,pOut[i].Phase); //dobrze
}


	
	return 0;
	}

/* Initialize AD5940 basic blocks like clock */
static int32_t AD5940PlatformCfg(void)
{
  CLKCfg_Type clk_cfg;
  FIFOCfg_Type fifo_cfg;
  AGPIOCfg_Type gpio_cfg;

  /* Use hardware reset */
  AD5940_HWReset();
  /* Platform configuration */
  AD5940_Initialize();
  /* Step1. Configure clock */
  clk_cfg.ADCClkDiv = ADCCLKDIV_1;
  clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
  clk_cfg.SysClkDiv = SYSCLKDIV_1;
  clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
  clk_cfg.HfOSC32MHzMode = bFALSE;
  clk_cfg.HFOSCEn = bTRUE;
  clk_cfg.HFXTALEn = bFALSE;
  clk_cfg.LFOSCEn = bTRUE;
  AD5940_CLKCfg(&clk_cfg);
  /* Step2. Configure FIFO and Sequencer*/
  fifo_cfg.FIFOEn = bFALSE;
  fifo_cfg.FIFOMode = FIFOMODE_FIFO;
  fifo_cfg.FIFOSize = FIFOSIZE_4KB;                       /* 4kB for FIFO, The reset 2kB for sequencer */
  fifo_cfg.FIFOSrc = FIFOSRC_DFT;
  fifo_cfg.FIFOThresh = 4;//AppBIACfg.FifoThresh;        /* DFT result. One pair for RCAL, another for Rz. One DFT result have real part and imaginary part */
  AD5940_FIFOCfg(&fifo_cfg);                             /* Disable to reset FIFO. */
  fifo_cfg.FIFOEn = bTRUE;  
  AD5940_FIFOCfg(&fifo_cfg);                             /* Enable FIFO here */
  
  /* Step3. Interrupt controller */
  
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);           /* Enable all interrupt in Interrupt Controller 1, so we can check INTC flags */
  AD5940_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH, bTRUE);   /* Interrupt Controller 0 will control GP0 to generate interrupt to MCU */
  AD5940_INTCClrFlag(AFEINTSRC_ALLINT);
  /* Step4: Reconfigure GPIO */
  gpio_cfg.FuncSet = GP6_SYNC|GP5_SYNC|GP4_SYNC|GP2_TRIG|GP1_SYNC|GP0_INT;
  gpio_cfg.InputEnSet = AGPIO_Pin2;
  gpio_cfg.OutputEnSet = AGPIO_Pin0|AGPIO_Pin1|AGPIO_Pin4|AGPIO_Pin5|AGPIO_Pin6;
  gpio_cfg.OutVal = 0;
  gpio_cfg.PullEnSet = 0;

  AD5940_AGPIOCfg(&gpio_cfg);
  AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);  /* Allow AFE to enter sleep mode. */
  return 0;
}

/* !!Change the application parameters here if you want to change it to none-default value */
void AD5940BIAStructInit(void)
{	
	
	//for (int czesto=1; czesto<=200000; czesto++){
		
  AppBIACfg_Type *pBIACfg;
  
  AppBIAGetCfg(&pBIACfg);
  
  pBIACfg->SeqStartAddr = 0;
  pBIACfg->MaxSeqLen = 512; /** @todo add checker in function */
	//pBIACfg->SinFreq = czesto; /*sobie dodalem */

	
	//for(i=1; i <= 20000; i++) {	//dodana petla zlogarytmowanego dziesietnie czestotliwosci 
	//	a = log10(i);
		//a = pBIACfg->SinFreq;
		
		/*sobie dodalem */
	//}
  
  pBIACfg->RcalVal = 10000.0;	//bylo 10000
	
  pBIACfg->DftNum = DFTNUM_8192;
  pBIACfg->NumOfData = -1;      /* Never stop until you stop it manually by AppBIACtrl() function */
  pBIACfg->BiaODR = 20;         /* ODR(Sample Rate) 20Hz */
  pBIACfg->FifoThresh = 4;      /* 4 */
  pBIACfg->ADCSinc3Osr = ADCSINC3OSR_2;
//	
	pBIACfg->SweepCfg.SweepEn = bTRUE;
//	pBIACfg->SweepCfg.SweepStart = 50;				//od 0.015 Hz
//  pBIACfg->SweepCfg.SweepStop = 195000.0;		//do 200kHz
//  pBIACfg->SweepCfg.SweepPoints = 100;
//  pBIACfg->SweepCfg.SweepLog = bTRUE;
//  pBIACfg->SweepCfg.SweepIndex = 0;
//	
}
//}

uint8_t dzialaj = 1;
void AD5940_Main(void)
{


	//printf("Dawaj wartosc");
		//scanf("%f", &var_1);
	//	printf("Blagam dzialaj wartosc: %f", var_1);
	static uint32_t IntCount;
  static uint32_t count;
  uint32_t temp;
	
  AD5940PlatformCfg();
  
  AD5940BIAStructInit(); /* Configure your parameters in this function */
  
  AppBIAInit(AppBuff, APPBUFF_SIZE);    /* Initialize BIA application. Provide a buffer, which is used to store sequencer commands */
  //AppBIACtrl(BIACTRL_START, 0);         /* Control BIA measurement to start. Second parameter has no meaning with this command. */

	printf("ZBIOR DOSTEPNYCH KOMEND WYSWIETLI SIE PO WPISANIU: \"help\"\n");
	printf("USTAWIENIE CZESTOTLIWOSCI POCZATKOWEJ ZA POMOCA KOMENDY: \"startfreq\"\n");
  while(1)
  {
    /* Check if interrupt flag which will be set when interrupt occurred. */
    if(dzialaj == 0){
			if(AD5940_GetMCUIntFlag())
			{
				IntCount++;
				AD5940_ClrMCUIntFlag(); /* Clear this flag */
				temp = APPBUFF_SIZE;
				AppBIAISR(AppBuff, &temp); /* Deal with it and provide a buffer to store data we got */
				BIAShowResult(AppBuff, temp); /* Show the results to UART */

				if(IntCount == 240)
				{
					IntCount = 0;
					//AppBIACtrl(BIACTRL_SHUTDOWN, 0);
				}
			}
    
			count++;
			if(count > 1000000)
			{
				count = 0;
				//AppBIAInit(0, 0);    /* Re-initialize BIA application. Because sequences are ready, no need to provide a buffer, which is used to store sequencer commands */
				//AppBIACtrl(BIACTRL_START, 0);          /* Control BIA measurement to start. Second parameter has no meaning with this command. */
			}
		}
	}
}

uint32_t ustaw_start_freq(uint32_t para1, uint32_t para2){
	AppBIACfg_Type *pBIACfg;
  AppBIAGetCfg(&pBIACfg);
	printf("Wartosc poczatkowa czestotliwosci: %d\n", para1);
	pBIACfg->SweepCfg.SweepStart = para1;
	printf("Ustaw wartosc koncowa czestotliwosci za pomoca komendy \"endfreq\"\n");
	return 0;
}

uint32_t ustaw_end_freq(uint32_t para1, uint32_t para2){
	AppBIACfg_Type *pBIACfg;
  AppBIAGetCfg(&pBIACfg);
	printf("Wartosc koncowa czestotliwosci: %d\n", para1);
	pBIACfg->SweepCfg.SweepStop = para1;
	printf("Ustaw ilosc punktow charakterystyki czestotliwosciowej za pomoca komendy \"punkty\"\n");
	return 0;
}

uint32_t start_pomiarow(uint32_t para1, uint32_t para2){
	printf("Freq[Hz],   Z[Ohm],   Phase[rad]\n");
	AppBIACtrl(BIACTRL_START,0);
	dzialaj = 0;
	return 0;
}
uint32_t stop_pomiarow(uint32_t para1, uint32_t para2){
	printf("Zatrzymanie pomiarow\n");
	AppBIACtrl(BIACTRL_STOPNOW,0);
	return 0;
}

uint32_t ustaw_punkty(uint32_t para1, uint32_t para2){
    AppBIACfg_Type *pBIACfg;
    AppBIAGetCfg(&pBIACfg);
    printf("Ilosc punktow wynosi: %d\n", para1);
    pBIACfg->SweepCfg.SweepPoints = para1;
		printf("Wybierz charakterystyke logarytmiczna za pomoca komendy \"log\" lub liniowa za pomoca komendy \"lin\"\n");
    return 0;
}

uint32_t char_log(uint32_t para1, uint32_t para2){
	  AppBIACfg_Type *pBIACfg;
    AppBIAGetCfg(&pBIACfg);
		pBIACfg->SweepCfg.SweepLog = bTRUE;
		printf("Punkty beda sie zwiekszac logarytmicznie\n");
		printf("Rozpocznij pomiary komenda \"start\", mozesz je przerwac za pomoca komendy \"stop\"\n");
	  return 0;
}
	
uint32_t char_lin(uint32_t para1, uint32_t para2){
		AppBIACfg_Type *pBIACfg;
    AppBIAGetCfg(&pBIACfg);
		pBIACfg->SweepCfg.SweepLog = bFALSE;
		printf("Punkty beda sie zwiekszac liniowo\n");
		printf("Rozpocznij pomiary komenda \"start\", mozesz je przerwac za pomoca komendy \"stop\"\n");
	  return 0;
}


/**
 * @}
 * @}
 * */
 
