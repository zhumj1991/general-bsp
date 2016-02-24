#include "rcc.h"



int8_t rcc_config(struct rcc *rcc, GB_StateType NewState)
{
	switch(rcc->bus) {
		case AHB1:
			RCC_AHB1PeriphClockCmd(rcc->base, (FunctionalState)NewState);
			break;
		case AHB2:
			RCC_AHB2PeriphClockCmd(rcc->base, (FunctionalState)NewState);
			break;
		case AHB3:
			RCC_AHB3PeriphClockCmd(rcc->base, (FunctionalState)NewState);
			break;
		case APB1:
			RCC_APB1PeriphClockCmd(rcc->base, (FunctionalState)NewState);
			break;
		case APB2:
			RCC_APB2PeriphClockCmd(rcc->base, (FunctionalState)NewState);
			break;
		default:
			return -1;
	}
	
	return 0;
}

uint8_t rcc_scale(struct rcc *rcc, uint32_t freq)
{
	uint8_t prescale = 1;
	uint8_t scale = 1;
	uint8_t temp = 0;
	
	switch(rcc->bus) {
		case APB1:
			if(RCC->CFGR & 0x1000) {
				prescale = ((RCC->CFGR & 0x0C00) >> 10) + 2;
			}
			
			break;
		case APB2:
			if(RCC->CFGR & 0x8000) {
				prescale = 1 << (((RCC->CFGR & 0x6000) >> 13) + 1);
			}
			
			break;
		default:
			return 0;
	}
	scale = SystemCoreClock/prescale/freq;
	
	while(scale) {
		scale >>= 2;
		temp++;
	}
	
	return temp;
}

