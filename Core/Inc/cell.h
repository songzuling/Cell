#ifndef __CELL_H__
#define __CELL_H__


#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "key.h"
#include "test.h"
/*电机驱动使能*/
#define ENABLE_DRIVER0      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
#define ENABLE_DRIVER1       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

/*电机驱禁用*/
#define DISENABLE_DRIVER0       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
#define DISENABLE_DRIVER1       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

///*转动电机*/
#define START_MOTOR(duty)    do{SetDuty(&htim4,TIM_CHANNEL_1,(duty));\
                                SetDuty(&htim4,TIM_CHANNEL_2,100);\
                                SetDuty(&htim4,TIM_CHANNEL_3,(duty));\
                                SetDuty(&htim4,TIM_CHANNEL_4,100);}while(0);


#define LED1_ON              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);   
#define LED1_OFF             HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);   

/*使能下一个板子请求ID信号*/
#define ENABLE_NEXT_BOARD    do{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);Delay(2);\
                                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);}while(0);     

/*使能下一层板子请求ID信号*/
#define ENABLE_NEXT_LAYER    do{HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);Delay(10);\
                                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);}while(0);                                
/*等待使能信号的端口*/
#define READ_ENABLE_PORT     GPIOA
/*等待使能信号的引脚*/
#define READ_ENABLE_PIN      GPIO_PIN_8

typedef struct
{
  uint32_t  CanID;               /*Can总线的ID*/
  uint8_t   Row;                 /*单元体的行编号*/
  uint32_t  Column;              /*单元体的列编号*/
  uint8_t   RequestIdEnable;     /*请求ID和addr使能标志*/
  uint8_t   AllocaOneLayerDone;  /*分配一层完成*/
  uint8_t   IsDeliver;           /*出货*/
  int32_t   RemainingGoodsNum;   /*剩余获取量*/
  uint8_t   DeliverGoodsNum;     /*要求出货的数量*/
  uint8_t   ReceiveRow;          /*接收到的出货行*/
  uint16_t  ReceiveColumn;       /*接收到的出货列*/
  uint8_t   IsZAxisReach;        /*z轴传送带是否已经准备好接收货物*/  
  KeyTypedef GoodsSwitch[2];     /*检测货物的开关*/  
  KeyTypedef Key[2];             /*按键*/
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
  CMD_ZAXIS_REACH,                    /*z轴到达*/
  CMD_MASTER_SET_CELL_NUMS,           /*设置单元体的货物个数*/
  CMD_MASTER_GET_CELL_NUMS,           /*获取单元体的货物个数*/  
};

enum{
  CELL_BUSY = 0U,       /*忙着呢*/
  CELL_READY,           /*准备好接单*/
  CELL_OUTPUT_TIMEOUT,  /*出货超时，等半没有货物掉下来*/
  CELL_WAITING_FOR_CAR, /*等小车来接货*/
  CELL_SAVE_DATA_FAILED,/*保存数据数失败了*/
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