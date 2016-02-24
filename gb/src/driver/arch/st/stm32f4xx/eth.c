#include "eth.h"


static int eth_init(struct driver * driver)
{
	struct eth_attr *attr = (struct eth_attr *)driver->attr;
	unsigned char i;
	
	for(i=0; i<3; i++) {
		if(rcc_config(driver->rcc+i, (GB_StateType)ENABLE))
			return -1;
	}
	
  ETH_SoftwareReset();/* Software reset */
  while (ETH_GetSoftwareResetStatus() == SET);/* Wait for software reset */

	ETH_InitTypeDef ETH_InitStructure;
	/* ETHERNET Configuration --------------------------------------------------*/
  /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
  ETH_StructInit(&ETH_InitStructure);

  /* Fill ETH_InitStructure parametrs */
  /*------------------------   MAC   -----------------------------------*/
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
//	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
//	ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
//	ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif	

  /*------------------------   DMA   -----------------------------------*/
  /* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum,
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

  /* Configure Ethernet */
	if(ETH_SUCCESS == ETH_Init(&ETH_InitStructure, attr->phy_addr))
		ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
	else
		return -1;

return 0;
}

static int eth_read(struct driver * driver, unsigned char* buf, unsigned int count)
{

	return 0;
}

static int eth_write(struct driver * driver, unsigned char* buf, unsigned int count)
{

	return 0;
}

static int eth_ioctl(struct driver * driver, unsigned char cmd, void * arg)
{

	return 0;
}

static int eth_close(struct driver * driver)
{

	return 0;
}

static const struct rcc eth_rcc[] = {
	{
		.bus				= AHB1,
		.base				= RCC_AHB1Periph_ETH_MAC
	},
	{
		.bus				= AHB1,
		.base				= RCC_AHB1Periph_ETH_MAC_Tx
	},
	{
		.bus				= AHB1,
		.base				= RCC_AHB1Periph_ETH_MAC_Rx	
	},
};

/*
 * sram variable and function
 */
static struct rcc eth_gpio_rcc[] = {
	{
		.bus			= AHB1, 
		.base			= RCC_AHB1Periph_GPIOA
	},
	{
		.bus			= AHB1, 
		.base			= RCC_AHB1Periph_GPIOC
	},
	{
		.bus			= AHB1, 
		.base			= RCC_AHB1Periph_GPIOG
	},
	{
		.bus			= AHB1, 
		.base			= RCC_AHB1Periph_GPIOH
	}
};

static struct rcc eth_syscfg_rcc = {
	.bus					= APB2,
	.base					= RCC_APB2Periph_SYSCFG
};

struct eth_attr eth_attr = {
	.init					= 0,
	.phy_addr			= 0x01,
};

int eth_gpio_init(struct driver* driver)
{
	unsigned char i;
	
	for(i=0; i<(sizeof(eth_gpio_rcc)/sizeof(eth_gpio_rcc[0])); i++) {
		if(rcc_config(&eth_gpio_rcc[i], (GB_StateType)ENABLE))
			return -1;
	}
	
	if(rcc_config(&eth_syscfg_rcc, (GB_StateType)ENABLE))
			return -1;

	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	/* Configure PA1, PA2 and PA7 */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	/* Configure PC1, PC4 and PC5 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

	/* Configure PG11, PG14 and PG13 */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);
	
	/* Configure PH6 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	return 0;
}

int eth_nvic(struct driver * driver)
{
	EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Connect EXTI Line to INT Pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOH, EXTI_PinSource6);
	
	/* Configure EXTI line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line6;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set the EXTI interrupt to the highest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	return 0;
}

static struct driver console __init_driver = {
	.name					= "eth",
	.base					= 0,
	.rcc					= (struct rcc *)&eth_rcc[0],
	
	.attr					= (void *)&eth_attr,
	
	.read_sem			= 0,
	.write_sem		= 0,

	.gpio_init		= eth_gpio_init,
	.func_init		= eth_init,
	.nvic_init		= eth_nvic,
	.read 				= eth_read,
	.write				= eth_write,
	.ioctl				= eth_ioctl,
	.close				= eth_close
};

void eth_irq(void)
{
  if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line6);
  }
}
