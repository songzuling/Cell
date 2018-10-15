#ifndef __CELL_H__
#define __CELL_H__


#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "key.h"
#include "test.h"
/*�������ʹ��*/
#define ENABLE_DRIVER0      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
#define ENABLE_DRIVER1       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

/*���������*/
#define DISENABLE_DRIVER0       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
#define DISENABLE_DRIVER1       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

///*ת�����*/
#define START_MOTOR(duty)    do{SetDuty(&htim4,TIM_CHANNEL_1,(duty));\
                                SetDuty(&htim4,TIM_CHANNEL_2,100);\
                                SetDuty(&htim4,TIM_CHANNEL_3,(duty));\
                                SetDuty(&htim4,TIM_CHANNEL_4,100);}while(0);


#define LED1_ON              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);   
#define LED1_OFF             HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);   

/*ʹ����һ����������ID�ź�*/
#define ENABLE_NEXT_BOARD    do{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);Delay(2);\
                                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);}while(0);     

/*ʹ����һ���������ID�ź�*/
#define ENABLE_NEXT_LAYER    do{HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);Delay(10);\
                                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);}while(0);                                
/*�ȴ�ʹ���źŵĶ˿�*/
#define READ_ENABLE_PORT     GPIOA
/*�ȴ�ʹ���źŵ�����*/
#define READ_ENABLE_PIN      GPIO_PIN_8

typedef struct
{
  uint32_t  CanID;               /*Can���ߵ�ID*/
  uint8_t   Row;                 /*��Ԫ����б��*/
  uint32_t  Column;              /*��Ԫ����б��*/
  uint8_t   RequestIdEnable;     /*����ID��addrʹ�ܱ�־*/
  uint8_t   AllocaOneLayerDone;  /*����һ�����*/
  uint8_t   IsDeliver;           /*����*/
  int32_t   RemainingGoodsNum;   /*ʣ���ȡ��*/
  uint8_t   DeliverGoodsNum;     /*Ҫ�����������*/
  uint8_t   ReceiveRow;          /*���յ��ĳ�����*/
  uint16_t  ReceiveColumn;       /*���յ��ĳ�����*/
  uint8_t   IsZAxisReach;        /*z�ᴫ�ʹ��Ƿ��Ѿ�׼���ý��ջ���*/  
  KeyTypedef GoodsSwitch[2];     /*������Ŀ���*/  
  KeyTypedef Key[2];             /*����*/
}CellTypeDef;

enum{
  CMD_NONE = 0U,
  CMD_MASTER_RESET_CELL,              /*������λ��Ԫ��*/
  CMD_MASTER_RESET_CELL_SYSTEM,       /*������λ��Ԫ��ϵͳ*/
  CMD_MASTER_REQUEST_ENABLE,          /*����ʹ������ID*/
  CMD_CELL_REQUEST_ID,                /*��Ԫ���������ID*/
  CMD_MASTER_ALLOCATE_ONE_LAYER_DONE, /*��������һ�����*/
  CMD_MASTER_SEND_ID,                 /*��������ID*/
  CMD_MASTER_DELIVER,                 /*������������*/
  CMD_CELL_DELIVER_DONE,              /*��Ԫ��������*/
  CMD_ZAXIS_REACH,                    /*z�ᵽ��*/
  CMD_MASTER_SET_CELL_NUMS,           /*���õ�Ԫ��Ļ������*/
  CMD_MASTER_GET_CELL_NUMS,           /*��ȡ��Ԫ��Ļ������*/  
};

enum{
  CELL_BUSY = 0U,       /*æ����*/
  CELL_READY,           /*׼���ýӵ�*/
  CELL_OUTPUT_TIMEOUT,  /*������ʱ���Ȱ�û�л��������*/
  CELL_WAITING_FOR_CAR, /*��С�����ӻ�*/
  CELL_SAVE_DATA_FAILED,/*����������ʧ����*/
};

extern CellTypeDef Cell;

void Delay(uint32_t ms);
void OnSwitchDown(void);
void OnSwitchUp(void);
void DoNothing(int i);
void SwitchScan(void);
void PraseCommand(CAN_HandleTypeDef* hcan);
void Cell_Init(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan);
HAL_StatusTypeDef RepuestIdAndAddr(CAN_HandleTypeDef *hcan);
uint8_t GetRequestState(CellTypeDef* cell);
void Deliver(CAN_HandleTypeDef* hcan);
HAL_StatusTypeDef SaveDataToFlash(void);
HAL_StatusTypeDef ReadDataFromFlash(void);
void SetDuty(TIM_HandleTypeDef *htim, uint32_t Channel, int32_t duty);
void CellReset(void);
HAL_StatusTypeDef ResetRemainingGoods(int32_t n);
#endif