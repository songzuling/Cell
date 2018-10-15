#ifndef __TEST_H__
#define __TEST_H__

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "cell.h"

void Test_MoveLeftRight(void);

extern int testDuty;
void Test_AddMotorSpeed(int i);
void Test_ReduceMotorSpeed(int i);
void Test_DutyMax(int i);
void Test_DutyMin(int i);
void TestBreak(void);
#endif

