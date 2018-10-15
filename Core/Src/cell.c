

#include "cell.h"


CellTypeDef Cell;
extern TIM_HandleTypeDef htim4;

void Cell_Init(CAN_HandleTypeDef *hcan)
{   
  Cell.Column = 0U;
  Cell.CanID = 0U;  
  Cell.RequestIdEnable = 0U;  
  Cell.AllocaOneLayerDone = 0U;
  Cell.RemainingGoodsNum = 0;
  
  // 检测货物的开关0
  Cell.GoodsSwitch[0].CheckAgainInterval = 50;
  Cell.GoodsSwitch[0].EventOnKeyDown = DoNothing;
  Cell.GoodsSwitch[0].EventOnDoubleClick = DoNothing;
  Cell.GoodsSwitch[0].EventOnKeyPress = DoNothing;
  Cell.GoodsSwitch[0].EventOnKeyUp = DoNothing;
  // 检测货物的开关1
  Cell.GoodsSwitch[1].CheckAgainInterval = 50;
  Cell.GoodsSwitch[1].EventOnKeyDown = DoNothing;
  Cell.GoodsSwitch[1].EventOnDoubleClick = DoNothing;
  Cell.GoodsSwitch[1].EventOnKeyPress = DoNothing;
  Cell.GoodsSwitch[1].EventOnKeyUp = DoNothing;
  
  // 按键0
  Cell.Key[0].Index = 0;
  Cell.Key[0].EventOnDoubleClick = DoNothing;
  Cell.Key[0].EventOnKeyDown = Test_AddMotorSpeed;
  Cell.Key[0].EventOnKeyPress = DoNothing;
  Cell.Key[0].EventOnKeyUp = DoNothing;
  
  // 按键1
  Cell.Key[1].Index = 1;
  Cell.Key[1].EventOnDoubleClick = DoNothing;
  Cell.Key[1].EventOnKeyDown = Test_ReduceMotorSpeed;
  Cell.Key[1].EventOnKeyPress = DoNothing;
  Cell.Key[1].EventOnKeyUp = DoNothing;
   
  /*从flash读取保存的数据*/
  if(HAL_OK != ReadDataFromFlash())
  {
    
  }
  
  /*Flash里面没有存有id或者addr*/
  if( (0==Cell.Column) || (0==Cell.CanID) )
  {
    
  }
  
   // TODO:测试用的，实际每个单元体没有100个货物
  if(Cell.RemainingGoodsNum == 0)
  {
    ResetRemainingGoods(100);
  }
     
  DISENABLE_DRIVER0;
  DISENABLE_DRIVER1;  
  START_MOTOR(0);
  
  LED1_ON;
  Delay(50);
  LED1_OFF;  
  
  return;
}


HAL_StatusTypeDef RepuestIdAndAddr(CAN_HandleTypeDef *hcan)
{    
    /*使能下一层板子请求ID*/
    if(Cell.AllocaOneLayerDone == CMD_MASTER_ALLOCATE_ONE_LAYER_DONE)
    {
        Cell.AllocaOneLayerDone = CMD_NONE;       
        ENABLE_NEXT_LAYER;
    }
    
    /*收到主机发送的请求ID使能信号*/
    if(Cell.RequestIdEnable == CMD_MASTER_REQUEST_ENABLE)
    {      
        Cell.RequestIdEnable = CMD_NONE;
        /*等待引脚使能信号*/
        while(GPIO_PIN_RESET == HAL_GPIO_ReadPin(READ_ENABLE_PORT, READ_ENABLE_PIN));
    }
    else
    {
        return HAL_OK;
    }
    
    /*请求主机分配ID*/
    hcan->pRxMsg->Data[0] = 0x00;
    hcan->pTxMsg->Data[0] = CMD_CELL_REQUEST_ID;  
    HAL_CAN_Transmit_IT(hcan);
    
    /*等待主机分配的ID发送过来*/    
    uint32_t tickstart = HAL_GetTick();
    while(hcan->pRxMsg->Data[0] != CMD_MASTER_SEND_ID)
    {
        if(HAL_GetTick() - tickstart > 1000)
        {
            return HAL_TIMEOUT;
        }
    }
    
    /*收到主机分配的ID*/
    hcan->pRxMsg->Data[0] = 0x00;
    Cell.Row = hcan->pRxMsg->Data[5];
    Cell.Column = (hcan->pRxMsg->Data[6]<<8)|hcan->pRxMsg->Data[7];
    Cell.CanID = (hcan->pRxMsg->Data[1]<<24)
                |(hcan->pRxMsg->Data[2]<<16)
                |(hcan->pRxMsg->Data[3]<<8)
                |(hcan->pRxMsg->Data[4]<<0);
    
    /*保存接收到的ID和ADDR*/
    SaveDataToFlash();
    
    /*更新can的id*/
    hcan->pRxMsg->ExtId = Cell.CanID;
    
    /*使能下一个板子请求ID之前将这个清零，要不然从第二层开始逻辑会乱*/
    Cell.AllocaOneLayerDone = CMD_NONE;  
    
    /*使能下一个板子请求ID*/
    ENABLE_NEXT_BOARD;
    
    /*指示灯*/
    LED1_ON;   
    Delay(200);
    LED1_OFF;  
         
    return HAL_OK;
}


void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{  
  HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
  PraseCommand(hcan);  
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
  HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
}

void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef *hcan)
{
  HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
}




void PraseCommand(CAN_HandleTypeDef* hcan)
{
    switch(hcan->pRxMsg->Data[0])
    {
      case CMD_NONE:;
        break;
        
      /*使能请求*/
      case CMD_MASTER_REQUEST_ENABLE :
        Cell.RequestIdEnable = CMD_MASTER_REQUEST_ENABLE;
        Cell.AllocaOneLayerDone = CMD_NONE;
        break;  
        
      /*分配一层完成*/  
      case CMD_MASTER_ALLOCATE_ONE_LAYER_DONE :
        Cell.AllocaOneLayerDone = CMD_MASTER_ALLOCATE_ONE_LAYER_DONE;
        break;   
        
      /*出货信号*/
      case CMD_MASTER_DELIVER :        
        Cell.ReceiveRow = hcan->pRxMsg->Data[5];  
        Cell.ReceiveColumn = (hcan->pRxMsg->Data[6]<<8)|hcan->pRxMsg->Data[7];    
        if( (Cell.ReceiveRow==Cell.Row) && (Cell.ReceiveColumn==Cell.Column) )
        {          
          Cell.IsDeliver = CMD_MASTER_DELIVER;
          Cell.DeliverGoodsNum = hcan->pRxMsg->Data[4];
        }
        break;
        
      /*z轴大传动带到达该行*/
      case CMD_ZAXIS_REACH:        
        if(hcan->pRxMsg->Data[5] == Cell.Row)
        {
          Cell.IsZAxisReach = CMD_ZAXIS_REACH;
        }
        else
        {
          Cell.IsZAxisReach = CMD_NONE;
        }
        break;
        
      /*复位单元体，出货机构*/
      case CMD_MASTER_RESET_CELL:
        CellReset();
        break;
        
      /*复位单元体系统*/
      case CMD_MASTER_RESET_CELL_SYSTEM:
        NVIC_SystemReset();
        break;
        
      default:;
        break;
    }    
    return;
}
 
/*
 * @brief  复位单元体,转动单元体的电机，直到检测货物的开关弹起，回到初始状态
 * @param  空
 * @retval 空
 */
void CellReset(void)
{
  START_MOTOR(100);  
  HAL_Delay(25);
  while(Cell.GoodsSwitch[0].IsDown || Cell.GoodsSwitch[1].IsDown);  
  START_MOTOR(0);
  return;
}

HAL_StatusTypeDef ResetRemainingGoods(int32_t n)
{  
  Cell.RemainingGoodsNum = n;
  return SaveDataToFlash();
}

/*
 * @brief  出货
 * @param  hcan：can
 * @retval 空
 */
void Deliver(CAN_HandleTypeDef* hcan)
{
    if((CMD_MASTER_DELIVER==Cell.IsDeliver) /*收到出货信号*/
       && (Cell.IsZAxisReach == CMD_ZAXIS_REACH)/*z轴传动带已经到达*/
       && Cell.RemainingGoodsNum > 0)/*剩余货物量大于0*/
    {
        Cell.IsDeliver = CMD_NONE;   
        Cell.IsZAxisReach = CMD_NONE;
        /*如果是该单元体出货*/
        if( Cell.DeliverGoodsNum > 0 )
        {
          LED1_ON;
          uint8_t n = Cell.DeliverGoodsNum;
          uint8_t done = 0;     
          /*1~n把货物送出去*/
          for(uint8_t i = 0; i < n; i++)
          {
            done = 0;
            uint32_t tickstart = HAL_GetTick();                
            START_MOTOR(100);
            ENABLE_DRIVER0;
            ENABLE_DRIVER1;
            Delay(50);
            while(!done)
            {         
              /*开关被按下了，货物已出*/
              if(Cell.GoodsSwitch[0].IsDown || Cell.GoodsSwitch[1].IsDown)
              {
                done = 1;
                /*等待被按下的开关抬起*/
                uint32_t tickstart1 = HAL_GetTick(); 
                uint8_t isTimeOut = 0;
                while(Cell.GoodsSwitch[0].IsDown || Cell.GoodsSwitch[1].IsDown)
                { // TODO:这里的超时时间需要考虑，就是单个货物出货超时时间
                  if(HAL_GetTick() - tickstart1 > 8000)
                  {
                    isTimeOut = 1;
                    break;
                  }
                }
                /*正常出货了*/
                if(isTimeOut == 0)
                {
                  Cell.RemainingGoodsNum--;
                  /*更新剩余货物量，如果保存失败需要上报*/
                  // TODO;失败处理
                  if(HAL_OK != SaveDataToFlash())
                  {
                    
                  }
                }
                /*单个货物出货超时*/
                else
                {
                  
                }
              }                         
              
              // TODO: 超时未检测到货物被送出，超时时间需要考量             
              if(HAL_GetTick() - tickstart > 30000)
              {
                break;
              }
            }// end while       
          }// end for 
          
          LED1_OFF;
          
          /*货物送完，电机停转*/          
          START_MOTOR(0);
          DISENABLE_DRIVER0;
          DISENABLE_DRIVER1;
          
          /*通知出货完成*/
          hcan->pTxMsg->Data[0] = CMD_CELL_DELIVER_DONE;
          hcan->pTxMsg->Data[5] = Cell.Row;
          hcan->pTxMsg->Data[6] = (uint8_t) (0xFF & (Cell.Column>>8) );  
          hcan->pTxMsg->Data[7] = (uint8_t) (0xFF & (Cell.Column>>0) );  
          HAL_CAN_Transmit_IT(hcan);

          // TODO:等待回应已经收到
          
          
        }// end if
        else if(Cell.RemainingGoodsNum <= 0)
        {
          // TODO:货物不足
          
        }
        else
        {
          
        }
        return ;
    }
    return;
}

void DoNothing(int i)
{
    return;
}

// 扫描开关
void SwitchScan(void)
{
  // 普通开关接成常开，即按下才接通
  KeyScan(&(Cell.GoodsSwitch[0]),1,!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4));
  KeyScan(&(Cell.GoodsSwitch[1]),1,!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3));
  KeyScan(&(Cell.Key[0]),1,!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0));
  KeyScan(&(Cell.Key[1]),1,!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1));  
  return;
}

void Delay(uint32_t ms)
{
    uint32_t tickstart = HAL_GetTick();
    
    while( HAL_GetTick() - tickstart < ms ){};
    
    return;
}

/////////////////////////////////////////////////////////
#define  FLASH_ADDR        0x0800F000 
#define  FLASH_BUFFE_SIZE  10
uint32_t FlashDataBuffer[FLASH_BUFFE_SIZE];

void MoveFlashBufferData(uint8_t dir)
{
    if(dir)
    {
        FlashDataBuffer[0] = (uint32_t)Cell.Row;
        FlashDataBuffer[1] = (uint32_t)Cell.Column;
        FlashDataBuffer[2] = (uint32_t)Cell.CanID;     
        FlashDataBuffer[3] = (uint32_t)Cell.RemainingGoodsNum;
    }
    else
    {
        Cell.Row = (uint8_t)FlashDataBuffer[0];
        Cell.Column = FlashDataBuffer[1];   
        Cell.CanID = FlashDataBuffer[2];   
        Cell.RemainingGoodsNum = (int32_t)FlashDataBuffer[3];   
    }
    return;
}


HAL_StatusTypeDef SaveDataToFlash(void)
{
    MoveFlashBufferData(1);
    FLASH_EraseInitTypeDef f;
    f.TypeErase = FLASH_TYPEERASE_PAGES;
    f.NbPages = 1;
    f.PageAddress = FLASH_ADDR;   
    uint32_t pageError = 0;
    if((HAL_OK==HAL_FLASH_Unlock()) && (HAL_OK == HAL_FLASHEx_Erase(&f,&pageError)))
    {
        for(int i=0; i<FLASH_BUFFE_SIZE; i++)
        {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_ADDR+i*8, (uint64_t)FlashDataBuffer[i]);
        }   
        HAL_FLASH_Lock();
        memset(FlashDataBuffer, 0, sizeof(uint64_t)*FLASH_BUFFE_SIZE);        
        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef ReadDataFromFlash(void)
{
    if(HAL_OK==HAL_FLASH_Unlock())
    {
        for(int i=0; i<FLASH_BUFFE_SIZE; i++)
        {
            FlashDataBuffer[i] = *((uint32_t *)(__IO uint32_t*)(FLASH_ADDR+i*8));
        }   
        HAL_FLASH_Lock();
        MoveFlashBufferData(0);
        return HAL_OK;
    }
    return HAL_ERROR;
}


void SetDuty(TIM_HandleTypeDef *htim, uint32_t Channel, int32_t duty)
{
    uint32_t tmp = 0;
    if(duty >= 100){duty = 100;}
    if(duty <= 0){duty = 0;}    
    tmp = htim->Init.Period/100*(uint32_t)duty;
    switch(Channel)
    {
      case TIM_CHANNEL_1:htim->Instance->CCR1 = tmp;break;
      case TIM_CHANNEL_2:htim->Instance->CCR2 = tmp;break;
      case TIM_CHANNEL_3:htim->Instance->CCR3 = tmp;break;
      case TIM_CHANNEL_4:htim->Instance->CCR4 = tmp;break;
      default:break;
    }
    return;
}


