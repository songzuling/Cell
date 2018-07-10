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
  uint8_t   IsDeliver;          /*出货*/
  uint32_t  GoodsCount;         /*要求出货的数量*/
  uint8_t   ReceiveRow;         /*接收到的出货行*/
  uint16_t  ReceiveColumn;      /*接收到的出货列*/
  uint8_t   IsZAxisReady;       /*z轴传送带是否已经准备好接收货物*/
  
  KeyTypedef GoodsSwitch;       /*检测货物的开关*/
}CellTypeDef;

enum{
  CMD_NONE = 0U,
  CMD_MASTER_RESET_CELL,              /*主机复位单元体*/
  CMD_MASTER_RESET_CELL_SYSTEM,       /*主机复位单元体系统*/
  CMD_MASTER_REQUEST_ENABLE,          /*主机使能请求ID*/
  CMD_CELL_REQUEST_ID,                /*单元体请求分配ID*/
  CMD_MASTER_ALLOCATE_ONE_LAYER_DONE, /*主机分配一层完成*/
  CMD_MASTER_SEND_ID,                 /*主机发送ID*/
  CMD_MASTER_DELIVER,                 /*主机出货命令*/
  CMD_CELL_DELIVER_DONE,              /*单元体出货完成*/
};

extern CellTypeDef Cell;

void OnSwitchDown(void);
void OnSwitchUp(void);
void DoNothing(void);
void PraseCommand(CAN_HandleTypeDef* hcan);
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