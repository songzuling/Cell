

#include "cell.h"

CellTypeDef Cell;
extern TIM_HandleTypeDef htim4;

void Cell_Init(CAN_HandleTypeDef *hcan)
{   
  Cell.Column = 0U;
  Cell.CanID = 0U;  
  Cell.RequestIdEnable = 0U;  
  Cell.AllocaOneLayerDone = 0U;
  
  Cell.GoodsSwitch.CheckAgainInterval = 50;
  Cell.GoodsSwitch.EventOnKeyDown = DoNothing;
  Cell.GoodsSwitch.EventOnDoubleClick = DoNothing;
  Cell.GoodsSwitch.EventOnKeyPress = DoNothing;
  Cell.GoodsSwitch.EventOnKeyUp = DoNothing;
  
  for(int i=0; i<2; i++)
  {
    Cell.Key[i].Index = i;
    Cell.Key[i].EventOnDoubleClick = DoNothing;
    Cell.Key[i].EventOnKeyDown = DoNothing;
    Cell.Key[i].EventOnKeyPress = DoNothing;
    Cell.Key[i].EventOnKeyUp = DoNothing;
  }
  
  /*��flash��ȡ���������*/
  if(HAL_OK != ReadDataFromFlash())
  {
    
  }
  
  /*Flash����û�д���id����addr*/
  if( (0==Cell.Column) || (0==Cell.CanID) )
  {
    //RepuestIdAndAddr(cell, hcan);
  }
  
  DRIVER_ENABLE;
  
  LED1_ON;
  Delay(50);
  LED1_OFF;
  
  return;
}


HAL_StatusTypeDef RepuestIdAndAddr(CAN_HandleTypeDef *hcan)
{    
    /*ʹ����һ���������ID*/
    if(Cell.AllocaOneLayerDone == CMD_MASTER_ALLOCATE_ONE_LAYER_DONE)
    {
        Cell.AllocaOneLayerDone = CMD_NONE;       
        ENABLE_NEXT_LAYER;
    }
    
    /*�յ��������͵�����IDʹ���ź�*/
    if(Cell.RequestIdEnable == CMD_MASTER_REQUEST_ENABLE)
    {      
        Cell.RequestIdEnable = CMD_NONE;
        /*�ȴ�����ʹ���ź�*/
        while(GPIO_PIN_RESET == HAL_GPIO_ReadPin(READ_ENABLE_PORT, READ_ENABLE_PIN));
    }
    else
    {
        return HAL_OK;
    }
    
    /*������������ID*/
    hcan->pRxMsg->Data[0] = 0x00;
    hcan->pTxMsg->Data[0] = CMD_CELL_REQUEST_ID;  
    HAL_CAN_Transmit_IT(hcan);
    
    /*�ȴ����������ID���͹���*/    
    uint32_t tickstart = HAL_GetTick();
    while(hcan->pRxMsg->Data[0] != CMD_MASTER_SEND_ID)
    {
        if(HAL_GetTick() - tickstart > 1000)
        {
            return HAL_TIMEOUT;
        }
    }
    
    /*�յ����������ID*/
    hcan->pRxMsg->Data[0] = 0x00;
    Cell.Row = hcan->pRxMsg->Data[5];
    Cell.Column = (hcan->pRxMsg->Data[6]<<8)|hcan->pRxMsg->Data[7];
    Cell.CanID = (hcan->pRxMsg->Data[1]<<24)
                |(hcan->pRxMsg->Data[2]<<16)
                |(hcan->pRxMsg->Data[3]<<8)
                |(hcan->pRxMsg->Data[4]<<0);
    
    /*������յ���ID��ADDR*/
    SaveDataToFlash();
    
    /*����can��id*/
    hcan->pRxMsg->ExtId = Cell.CanID;
    
    /*ʹ����һ����������ID֮ǰ��������㣬Ҫ��Ȼ�ӵڶ��㿪ʼ�߼�����*/
    Cell.AllocaOneLayerDone = CMD_NONE;  
    
    /*ʹ����һ����������ID*/
    ENABLE_NEXT_BOARD;
    
    /*ָʾ��*/
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
      case CMD_MASTER_REQUEST_ENABLE :
        Cell.RequestIdEnable = CMD_MASTER_REQUEST_ENABLE;
        Cell.AllocaOneLayerDone = CMD_NONE;
        break;  
      case CMD_MASTER_ALLOCATE_ONE_LAYER_DONE :
        Cell.AllocaOneLayerDone = CMD_MASTER_ALLOCATE_ONE_LAYER_DONE;
        break;        
      case CMD_MASTER_DELIVER :
        Cell.IsDeliver = CMD_MASTER_DELIVER;
        Cell.ReceiveRow = hcan->pRxMsg->Data[5];  
        Cell.ReceiveColumn = (hcan->pRxMsg->Data[6]<<8)|hcan->pRxMsg->Data[7];    
        Cell.GoodsCount += (uint32_t)(hcan->pRxMsg->Data[4]);
        break;
      case CMD_MASTER_RESET_CELL:
        CellReset();
        break;
      case CMD_MASTER_RESET_CELL_SYSTEM:
        NVIC_SystemReset();
        break;
      default:;
        break;
    }    
    return;
}
 
/*
 * @brief  ��λ��Ԫ��,ת����Ԫ��ĵ����ֱ��������Ŀ��ص��𣬻ص���ʼ״̬
 * @param  ��
 * @retval ��
 */
void CellReset(void)
{
  START_MOTOR(100);
  while(Cell.GoodsSwitch.IsDown);
  START_MOTOR(0);
  return;
}

/*
 * @brief  ����
 * @param  hcan��can
 * @retval ��
 */
void Deliver(CAN_HandleTypeDef* hcan)
{
    // TODO:����z�ᵽ�ﲻ������д����һ��ȥ����
    if((CMD_MASTER_DELIVER==Cell.IsDeliver) )//;//&& (hcan->pRxMsg->Data[0] == CMD_ZAXIS_REACH))
    {
        Cell.IsDeliver = CMD_NONE;   
        /*����Ǹõ�Ԫ�����*/
        if( (Cell.ReceiveRow==Cell.Row) && (Cell.ReceiveColumn==Cell.Column) )
        {
          LED1_ON;
          uint8_t n = Cell.GoodsCount;
          uint8_t done = 0;      
          for(uint8_t i=0; i<n; i++)
          {
            done = 0;
            uint32_t tickstart = HAL_GetTick();                
            START_MOTOR(100);
            while(!done)
            {          
              if(Cell.GoodsSwitch.IsDown)
              {
                done = 1;
                while(Cell.GoodsSwitch.IsDown)
                { // TODO:����ĳ�ʱʱ����Ҫ���ǣ����ǵ������������ʱʱ��
                  if(HAL_GetTick() - tickstart > 2000)
                  {
                    break;
                  }
                };
                Cell.GoodsCount--;
              }                         
              
              // TODO: ��ʱδ��⵽���ﱻ�ͳ�����ʱʱ����Ҫ����             
              if(HAL_GetTick() - tickstart > 500)
              {
                break;
              }
            }// end while       
          }// end for 
          
          LED1_OFF;
          
          /*�������꣬���ͣת*/
          START_MOTOR(0);
          
          /*֪ͨ�����������*/
          hcan->pTxMsg->Data[0] = CMD_CELL_DELIVER_DONE;
          hcan->pTxMsg->Data[5] = Cell.Row;
          hcan->pTxMsg->Data[6] = (uint8_t) (0xFF & (Cell.Column>>8) );  
          hcan->pTxMsg->Data[7] = (uint8_t) (0xFF & (Cell.Column>>0) );  
          int retrycnt = 10;
          while( HAL_OK != HAL_CAN_Transmit_IT(hcan) && (retrycnt-->0))
          {
            Delay(10);
          }   
          /*֪ͨ����ʧ��*/
          if(retrycnt < 0)
          {
            // TODO:ʧ�ܴ���
          }
        }
        return ;
    }
    return;
}

void DoNothing(int i)
{
    return;
}

// ɨ�迪��
void SwitchScan(void)
{
  KeyScan(&(Cell.GoodsSwitch),1,HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0));
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
        FlashDataBuffer[0] = Cell.Row;
        FlashDataBuffer[1] = Cell.Column;
        FlashDataBuffer[2] = Cell.CanID;     
    }
    else
    {
        Cell.Row = FlashDataBuffer[0];
        Cell.Column = FlashDataBuffer[1];   
        Cell.CanID = FlashDataBuffer[2];   
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


void SetDuty(TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t duty)
{
    uint32_t tmp = 0;
    if(duty > 99)
    {
        duty = 99;
    }
    tmp = htim->Init.Period/100*duty;
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


