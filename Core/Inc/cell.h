#ifndef __CELL_H__
#define __CELL_H__


#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "key.h"

typedef struct
{
  uint32_t  CanID;              /*Can 总线的ID*/
  uint8_t   Row;                /*单元体的行编号*/
  uint32_t  Column;             /*单元体的列编号*/
  uint8_t   RequestIdEnable;    /*请求ID和addr使能标志*/
  uint8_t   AllocaOneLayerDone; /*分配一层完成*/
  uint8_t   Deliver;            /*出货*/
  uint8_t   GoodsCount;         /*要求出货的数量*/
  uint8_t   ReceiveRow;         /*接收到的出货行*/
  uint16_t  ReceiveColumn;      /*接收到的出货列*/
  uint8_t   IsZAxisReady;       /*z轴传送带是否已经准备好接收货物*/
  
  KeyTypedef GoodsSwitch;    /*检测货物的开关*/
}CellTypeDef;

enum{
  CELL_NONE = 0U,
  CELL_RESET,                   /*复位单元体*/
  CELL_RESET_SYSTEM,            /*复位单元体系统*/
  CELL_REQUEST_ENABLE,          /*使能请求ID*/
  CELL_REQUEST_ID,              /*请求分配ID*/
  CELL_ALLOCATE_ONE_LAYER_DONE, /*分配一层完成*/
  CELL_GET_ID,                  /*收到ID*/
  CELL_DELIVER,                 /*出货命令*/
  CELL_DELIVER_DONE,            /*单元体出货完成*/
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
void CellReset(void);
#endif