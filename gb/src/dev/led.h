#ifndef	__LED_H
#define __LED_H

#include <stdbool.h>

#define LEDSEQ_CHARGE_CYCLE_TIME  1000
//Led sequence action
#define LEDSEQ_WAITMS(x) (x)
#define LEDSEQ_STOP      -1
#define LEDSEQ_LOOP      -2
typedef struct {
  bool value;
  int action;
} ledseq_t;

typedef enum {
	LED_RED = 0,
	LED_GREEN,
	LED_BLUE
} led_t;

//Existing led sequences
extern ledseq_t seq_armed[];
extern ledseq_t seq_calibrated[];
extern ledseq_t seq_alive[];
extern ledseq_t seq_lowbat[];
extern ledseq_t seq_linkup[];
extern ledseq_t seq_charged[];
extern ledseq_t seq_charging[];
extern ledseq_t seq_bootloader[];
extern ledseq_t seq_NotPassed[];


void ledseq_init(void);
void ledseqRun(led_t led, ledseq_t *sequence);

int led_init(void);

#endif
