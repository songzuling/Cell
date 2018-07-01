#ifndef __CELL_H__
#define __CELL_H__


#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "key.h"

typedef struct
{
  uint32_t  CanID;           /*Can 总线的ID*/
  uint32_t  Addr;            /*单元体的编号，地址*/
  uint8_t   RequestIdEnable; /*请求ID和addr使能标志*/
  uint8_t   Deliver;         /*出货*/
  uint8_t   GoodsCount;      /*要求出货的数量*/
  uint16_t  AddrReceive;     /*接收到的出货addr，即主机要求哪个单元体出货*/
  uint8_t   IsZAxisReady;    /*z轴传送带是否已经准备好接收货物*/
  
  KeyTypedef GoodsSwitch;    /*检测货物的开关*/
}CellTypeDef;

enum{
  CELL_NONE           = 0x00,
  CELL_REQUEST_ENABLE = 0x01, /*使能请求ID*/
  CELL_REQUEST_ID     = 0x02, /*请求分配ID*/
  CELL_GET_ID         = 0x03, /*收到ID*/
  CELL_DELIVER        = 0x04, /*出货命令*/
  CELL_DELIVER_DONE   = 0x05, /*单元体出货完成*/
};

extern CellTypeDef Cell;

void OnSwitchDown(void);
void OnSwitchUp(void);
void DoNothing(void);
void CheckCommand(CAN_HandleTypeDef* hcan);
void Cell_Init(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan);
HAL_StatusTypeDef RepuestIdAndAddr(CAN_HandleTypeDef *hcan);
uint8_t GetRequestState(CellTypeDef* cell);
void Deliver(CAN_HandleTypeDef* hcan);
HAL_StatusTypeDef SaveDataToFlash(void);
HAL_StatusTypeDef ReadDataFromFlash(void);
void SetDuty(TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t duty);
#endif