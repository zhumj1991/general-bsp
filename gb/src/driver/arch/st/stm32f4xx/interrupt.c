#include "gb_driver.h"

/* External Interrupts */
static interrupt_handler irq_handler[82];

#define IrqHandlerLink(func, num)			void func (void)		\
																			{										\
																				irq_handler[num]();	\
																			}
																		
IrqHandlerLink(WWDG_IRQHandler,				WWDG_IRQn)
IrqHandlerLink(PVD_IRQHandler,				PVD_IRQn)
IrqHandlerLink(TAMP_STAMP_IRQHandler,	TAMP_STAMP_IRQn)
IrqHandlerLink(RTC_WKUP_IRQHandler,		RTC_WKUP_IRQn)
IrqHandlerLink(FLASH_IRQHandler,			FLASH_IRQn)
IrqHandlerLink(RCC_IRQHandler,				RCC_IRQn)
IrqHandlerLink(EXTI0_IRQHandler,			EXTI0_IRQn)
IrqHandlerLink(EXTI1_IRQHandler,			EXTI1_IRQn)
IrqHandlerLink(EXTI2_IRQHandler,			EXTI2_IRQn)
IrqHandlerLink(EXTI3_IRQHandler,			EXTI3_IRQn)
IrqHandlerLink(EXTI4_IRQHandler,			EXTI4_IRQn)
IrqHandlerLink(DMA1_Stream0_IRQHandler,	DMA1_Stream0_IRQn)
IrqHandlerLink(DMA1_Stream1_IRQHandler,	DMA1_Stream1_IRQn)
IrqHandlerLink(DMA1_Stream2_IRQHandler,	DMA1_Stream2_IRQn)
IrqHandlerLink(DMA1_Stream3_IRQHandler,	DMA1_Stream3_IRQn)
IrqHandlerLink(DMA1_Stream4_IRQHandler,	DMA1_Stream4_IRQn)
IrqHandlerLink(DMA1_Stream5_IRQHandler,	DMA1_Stream5_IRQn)
IrqHandlerLink(DMA1_Stream6_IRQHandler,	DMA1_Stream6_IRQn)
IrqHandlerLink(ADC_IRQHandler,				ADC_IRQn)
IrqHandlerLink(CAN1_TX_IRQHandler,		CAN1_TX_IRQn)
IrqHandlerLink(CAN1_RX0_IRQHandler,		CAN1_RX0_IRQn)
IrqHandlerLink(CAN1_RX1_IRQHandler,		CAN1_RX1_IRQn)
IrqHandlerLink(CAN1_SCE_IRQHandler,		CAN1_SCE_IRQn)
IrqHandlerLink(EXTI9_5_IRQHandler,		EXTI9_5_IRQn)
IrqHandlerLink(TIM1_BRK_TIM9_IRQHandler,	TIM1_BRK_TIM9_IRQn)
IrqHandlerLink(TIM1_UP_TIM10_IRQHandler,	TIM1_UP_TIM10_IRQn)
IrqHandlerLink(TIM1_TRG_COM_TIM11_IRQHandler,	TIM1_TRG_COM_TIM11_IRQn)
IrqHandlerLink(TIM1_CC_IRQHandler,		TIM1_CC_IRQn)
IrqHandlerLink(TIM2_IRQHandler,				TIM2_IRQn)
IrqHandlerLink(TIM3_IRQHandler,				TIM3_IRQn)
IrqHandlerLink(TIM4_IRQHandler,				TIM4_IRQn)
IrqHandlerLink(I2C1_EV_IRQHandler,		I2C1_EV_IRQn)
IrqHandlerLink(I2C1_ER_IRQHandler,		I2C1_ER_IRQn)
IrqHandlerLink(I2C2_EV_IRQHandler,		I2C2_EV_IRQn)
IrqHandlerLink(I2C2_ER_IRQHandler,		I2C2_ER_IRQn)
IrqHandlerLink(SPI1_IRQHandler,				SPI1_IRQn)
IrqHandlerLink(SPI2_IRQHandler,				SPI2_IRQn)
IrqHandlerLink(USART1_IRQHandler,			USART1_IRQn)
IrqHandlerLink(USART2_IRQHandler,			USART2_IRQn)
IrqHandlerLink(USART3_IRQHandler,			USART3_IRQn)
IrqHandlerLink(EXTI15_10_IRQHandler,	EXTI15_10_IRQn)
IrqHandlerLink(RTC_Alarm_IRQHandler,	RTC_Alarm_IRQn)
IrqHandlerLink(OTG_FS_WKUP_IRQHandler,OTG_FS_WKUP_IRQn)
IrqHandlerLink(TIM8_BRK_TIM12_IRQHandler,	TIM8_BRK_TIM12_IRQn)
IrqHandlerLink(TIM8_UP_TIM13_IRQHandler,	TIM8_UP_TIM13_IRQn)
IrqHandlerLink(TIM8_TRG_COM_TIM14_IRQHandler,	TIM8_TRG_COM_TIM14_IRQn)
IrqHandlerLink(TIM8_CC_IRQHandler,		TIM8_CC_IRQn)
IrqHandlerLink(DMA1_Stream7_IRQHandler,		DMA1_Stream7_IRQn)
IrqHandlerLink(FSMC_IRQHandler,				FSMC_IRQn)
IrqHandlerLink(SDIO_IRQHandler,				SDIO_IRQn)
IrqHandlerLink(TIM5_IRQHandler,				TIM5_IRQn)
IrqHandlerLink(SPI3_IRQHandler,				SPI3_IRQn)
IrqHandlerLink(UART4_IRQHandler,			UART4_IRQn)
IrqHandlerLink(UART5_IRQHandler,			UART5_IRQn)
IrqHandlerLink(TIM6_DAC_IRQHandler,		TIM6_DAC_IRQn)
IrqHandlerLink(TIM7_IRQHandler,				TIM7_IRQn)
IrqHandlerLink(DMA2_Stream0_IRQHandler,	DMA2_Stream0_IRQn)
IrqHandlerLink(DMA2_Stream1_IRQHandler,	DMA2_Stream1_IRQn)
IrqHandlerLink(DMA2_Stream2_IRQHandler,	DMA2_Stream2_IRQn)
IrqHandlerLink(DMA2_Stream3_IRQHandler,	DMA2_Stream3_IRQn)
IrqHandlerLink(DMA2_Stream4_IRQHandler,	DMA2_Stream4_IRQn)		
IrqHandlerLink(ETH_IRQHandler,				ETH_IRQn)
IrqHandlerLink(ETH_WKUP_IRQHandler,		ETH_WKUP_IRQn)
IrqHandlerLink(CAN2_TX_IRQHandler,		CAN2_TX_IRQn)
IrqHandlerLink(CAN2_RX0_IRQHandler,		CAN2_RX0_IRQn)
IrqHandlerLink(CAN2_RX1_IRQHandler,		CAN2_RX1_IRQn)
IrqHandlerLink(CAN2_SCE_IRQHandler,		CAN2_SCE_IRQn)
IrqHandlerLink(OTG_FS_IRQHandler,			OTG_FS_IRQn)
IrqHandlerLink(DMA2_Stream5_IRQHandler,	DMA2_Stream5_IRQn)
IrqHandlerLink(DMA2_Stream6_IRQHandler,	DMA2_Stream6_IRQn)
IrqHandlerLink(DMA2_Stream7_IRQHandler,	DMA2_Stream7_IRQn)
IrqHandlerLink(USART6_IRQHandler,			USART6_IRQn)
IrqHandlerLink(I2C3_EV_IRQHandler,		I2C3_EV_IRQn)
IrqHandlerLink(I2C3_ER_IRQHandler,		I2C3_ER_IRQn)
IrqHandlerLink(OTG_HS_EP1_OUT_IRQHandler,	OTG_HS_EP1_OUT_IRQn)
IrqHandlerLink(OTG_HS_EP1_IN_IRQHandler,	OTG_HS_EP1_IN_IRQn)
IrqHandlerLink(OTG_HS_WKUP_IRQHandler,		OTG_HS_WKUP_IRQn)
IrqHandlerLink(OTG_HS_IRQHandler,			OTG_HS_IRQn)
IrqHandlerLink(DCMI_IRQHandler,				DCMI_IRQn)
IrqHandlerLink(CRYP_IRQHandler,				CRYP_IRQn)
IrqHandlerLink(HASH_RNG_IRQHandler,		HASH_RNG_IRQn)
IrqHandlerLink(FPU_IRQHandler,				FPU_IRQn)


void interrupt_init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	return;
}

int8_t register_interupt(interrupt_handler handler, uint8_t irq, uint8_t prio)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	if(!IS_NVIC_PREEMPTION_PRIORITY(prio) || irq > 81)
		return -1;
	
	NVIC_InitStructure.NVIC_IRQChannel = irq;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	irq_handler[irq] = handler;
	
	return 0;
}

