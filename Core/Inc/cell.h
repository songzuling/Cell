#ifndef __CELL_H__
#define __CELL_H__


#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "key.h"

typedef struct
{
  uint32_t  CanID;              /*Can ���ߵ�ID*/
  uint8_t   Row;                /*��Ԫ����б��*/
  uint32_t  Column;             /*��Ԫ����б��*/
  uint8_t   RequestIdEnable;    /*����ID��addrʹ�ܱ�־*/
  uint8_t   AllocaOneLayerDone; /*����һ�����*/
  uint8_t   Deliver;            /*����*/
  uint8_t   GoodsCount;         /*Ҫ�����������*/
  uint8_t   ReceiveRow;         /*���յ��ĳ�����*/
  uint16_t  ReceiveColumn;      /*���յ��ĳ�����*/
  uint8_t   IsZAxisReady;       /*z�ᴫ�ʹ��Ƿ��Ѿ�׼���ý��ջ���*/
  
  KeyTypedef GoodsSwitch;    /*������Ŀ���*/
}CellTypeDef;

enum{
  CELL_NONE = 0U,
  CELL_RESET,                   /*��λ��Ԫ��*/
  CELL_RESET_SYSTEM,            /*��λ��Ԫ��ϵͳ*/
  CELL_REQUEST_ENABLE,          /*ʹ������ID*/
  CELL_REQUEST_ID,              /*�������ID*/
  CELL_ALLOCATE_ONE_LAYER_DONE, /*����һ�����*/
  CELL_GET_ID,                  /*�յ�ID*/
  CELL_DELIVER,                 /*��������*/
  CELL_DELIVER_DONE,            /*��Ԫ��������*/
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