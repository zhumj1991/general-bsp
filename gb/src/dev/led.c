#include <stdio.h>
#include "gb_driver.h"
#include "gb_dev.h"
#include "gpio.h"
#include "led.h"


/* Led sequences */
ledseq_t seq_lowbat[] = {
  { true, LEDSEQ_WAITMS(1000)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_armed[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(250)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_calibrated[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(450)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_alive[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(1950)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_linkup[] = {
  { true, LEDSEQ_WAITMS(10)},
  {false, LEDSEQ_WAITMS(0)},
  {    0, LEDSEQ_STOP},
};

ledseq_t seq_charged[] = {
  { true, LEDSEQ_WAITMS(1000)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_charging[] = {
  { true, LEDSEQ_WAITMS(200)},
  {false, LEDSEQ_WAITMS(800)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_bootloader[] = {
  { true, LEDSEQ_WAITMS(500)},
  {false, LEDSEQ_WAITMS(500)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_NotPassed[] = {
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};

/* Led sequence priority */
static ledseq_t * sequences[] = {
  seq_NotPassed,
  seq_lowbat,
  seq_charged,
  seq_charging,
  seq_bootloader,
  seq_armed,
  seq_calibrated,
  seq_alive,
  seq_linkup,
};


/* Led sequence handling machine implementation */
#define SEQ_NUM					(sizeof(sequences) / sizeof(sequences[0]))
#define LED_NUM					2

//State of every sequence for every led: LEDSEQ_STOP if stopped or the current 
//step
static int state[LED_NUM][SEQ_NUM];
//Active sequence for each led
static int activeSeq[LED_NUM];

static sem_handle_t ledseq_sem;
static timer_handle_t ledseq_timer[LED_NUM];

static void runLedseq(OS_TMR *p_tmr, void *p_arg);
static void ledSet(led_t led, bool value);

void ledseq_init()
{
	unsigned char i, j;

  //Initialise the sequences state
  for(i=0; i<LED_NUM; i++) {
    activeSeq[i] = LEDSEQ_STOP;
    for(j=0; j<SEQ_NUM; j++)
      state[i][j] = LEDSEQ_STOP;
  }
	
	//Init the soft timers that runs the led sequences for each leds
  for(i=0; i<LED_NUM; i++) {		
		if(timer_creat(&ledseq_timer[i], "ledseqTimer", 1000, runLedseq, (void *)i) != 0)
			return;
	}
	
	if(sem_creat(&ledseq_sem, 1) != 0)
		return;
}

//Utility functions
static int getPrio(ledseq_t *seq)
{
  int prio;

  //Find the priority of the sequence
  for(prio=0; prio<SEQ_NUM; prio++)
    if(sequences[prio] == seq) return prio;
  
  return -1; //Invalid sequence
}

static void updateActive(led_t led)
{
  int prio;
  
  activeSeq[led] = LEDSEQ_STOP;
  ledSet(led, false);
  
  for(prio=0; prio<SEQ_NUM; prio++)
  {
    if (state[led][prio] != LEDSEQ_STOP)
    {
      activeSeq[led] = prio;
      break;
    }
  }
}

/* Center of the led sequence machine. This function is executed by the FreeRTOS
 * timer and runs the sequences
 */
static void runLedseq(OS_TMR *p_tmr, void *p_arg)
{	
	led_t led = (led_t)p_arg;
  ledseq_t *step;
  bool leave = false;

  while(!leave) {
    int prio = activeSeq[led];
  
    if (prio == LEDSEQ_STOP)
      return;
    
    step = &sequences[prio][state[led][prio]];

    state[led][prio]++;
    
		sem_wait(&ledseq_sem, 0);
		
    switch(step->action)
    {
      case LEDSEQ_LOOP:
        state[led][prio] = 0;
        break;
      case LEDSEQ_STOP:
        state[led][prio] = LEDSEQ_STOP;
        updateActive(led);
        break;
      default:  //The step is a LED action and a time
        ledSet(led, step->value);
        if (step->action == 0)
          break;
				timer_set_period(p_tmr, step->action);
				timer_start(p_tmr);
        leave = true;
        break;
    }
		
		sem_post(&ledseq_sem);
  }
}

void ledseqRun(led_t led, ledseq_t *sequence)
{
  int prio = getPrio(sequence);
  
  if(prio < 0) return;
  
  sem_wait(&ledseq_sem, 0);
	
  state[led][prio] = 0;  //Reset the seq. to its first step
  updateActive(led);
	
  sem_post(&ledseq_sem);
  
  //Run the first step if the new seq is the active sequence
  if(activeSeq[led] == prio)
    runLedseq(&ledseq_timer[led], (void *)led);
}

void ledseqSetTimes(ledseq_t *sequence, int32_t onTime, int32_t offTime)
{
  sequence[0].action = onTime;
  sequence[1].action = offTime;
}

void ledseqStop(led_t led, ledseq_t *sequence)
{	
  int prio = getPrio(sequence);
  
  if(prio < 0) return;
  
  sem_wait(&ledseq_sem, 0);
	
  state[led][prio] = LEDSEQ_STOP;  //Stop the seq.
  updateActive(led);
	
  sem_post(&ledseq_sem);
  
  //Run the next active sequence (if any...)
  runLedseq(&ledseq_timer[led], (void *)led);
}

/*
 * led device
 */

static struct dev led_dev = {
	.name = "led",
	.driver_name = "gpio",
};

static struct pin_value ledseq_pin[LED_NUM];

int led_init(void)
{
	unsigned char i;
	struct gpio_attr *attr;
	
	if(dev_mach(&led_dev) == 0) {
		attr = (struct gpio_attr *)led_dev.driver->attr;
		for(i=0; i<LED_NUM; i++) {
			ledseq_pin[i].pin = (attr->gpio + i + 1)->pin;
			ledseq_pin[i].port = (attr->gpio + i + 1)->port;
			printf("[ledseq_init]LED%d connect to P%c%d\r\n", i, PORT(ledseq_pin[i].port), ledseq_pin[i].pin);
		}
		return 0;
	}

	printf("\r\n***ERROR: no gpio driver found!!\r\n");
	return -1;
}

static void ledSet(led_t led, bool value) 
{
	struct driver * driver = led_dev.driver;
	
	if (led > LED_NUM)
    return;
		
  ledseq_pin[led].value = !value;
	driver->ioctl(led_dev.driver, GPIO_OUT, &ledseq_pin[led]);

}
