#ifndef __TAIL_H__
#define __TAIL_H__

#include "stdint.h"

typedef enum{
    TAIL_SET_ANGLE_BIT   =   1 << 0,
    TAIL_WAG_BIT         =   1 << 1,  
}DrinkActionGroupBits_t;

void TailSetTargetAngle(uint16_t angle);
void TailStartWagging(uint16_t start_angle, uint16_t end_angle, uint16_t cycle_delay, uint16_t count);
void TailTaskStart(void);

#endif /* __TAIL_H__ */
