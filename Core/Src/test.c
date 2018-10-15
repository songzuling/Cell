#include "test.h"
extern TIM_HandleTypeDef htim4;




int testDuty = 0;
void Test_AddMotorSpeed(int i)
{
  ENABLE_DRIVER0;
  DISENABLE_DRIVER1;
  testDuty += 100;
  if(testDuty > 100) testDuty = 100;
  START_MOTOR(testDuty);
}

void Test_ReduceMotorSpeed(int i)
{
  DISENABLE_DRIVER0;
  DISENABLE_DRIVER1;
  testDuty -= 100;
  if(testDuty < 0) testDuty = 0;
  START_MOTOR(testDuty);
}

void Test_DutyMax(int i)
{
   testDuty = 50;   
   START_MOTOR(testDuty);
}

void Test_DutyMin(int i)
{
   testDuty = 0;   
   START_MOTOR(testDuty);
}

int32_t intVal = -100;
uint32_t uintVal = 0;

void TestBreak(void)
{

}






