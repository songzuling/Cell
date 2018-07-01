#ifndef __CELL_H__
#define __CELL_H__


#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "key.h"

typedef struct
{
  uint32_t  CanID;           /*Can ���ߵ�ID*/
  uint32_t  Addr;            /*��Ԫ��ı�ţ���ַ*/
  uint8_t   RequestIdEnable; /*����ID��addrʹ�ܱ�־*/
  uint8_t   Deliver;         /*����*/
  uint8_t   GoodsCount;      /*Ҫ�����������*/
  uint16_t  AddrReceive;     /*���յ��ĳ���addr��������Ҫ���ĸ���Ԫ�����*/
  uint8_t   IsZAxisReady;    /*z�ᴫ�ʹ��Ƿ��Ѿ�׼���ý��ջ���*/
  
  KeyTypedef GoodsSwitch;    /*������Ŀ���*/
}CellTypeDef;

enum{
  CELL_NONE           = 0x00,
  CELL_REQUEST_ENABLE = 0x01, /*ʹ������ID*/
  CELL_REQUEST_ID     = 0x02, /*�������ID*/
  CELL_GET_ID         = 0x03, /*�յ�ID*/
  CELL_DELIVER        = 0x04, /*��������*/
  CELL_DELIVER_DONE   = 0x05, /*��Ԫ��������*/
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