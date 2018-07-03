

#include "cell.h"

CellTypeDef Cell;
extern TIM_HandleTypeDef htim4;
static void Delay(uint32_t ms);


void Cell_Init(CAN_HandleTypeDef *hcan)
{
    Cell.Column = 0U;
    Cell.CanID = 0U;  
    Cell.RequestIdEnable = 0U;  
    Cell.AllocaOneLayerDone = 0U;
    
    Cell.GoodsSwitch.OnKeyDownEnable = 1;
    Cell.GoodsSwitch.OnKeyPressEnable = 0;
    Cell.GoodsSwitch.OnDoubleClickEnable = 0;
    Cell.GoodsSwitch.OnKeyUpEnable = 1;
    Cell.GoodsSwitch.CheckAgainInterval = 50;
    Cell.GoodsSwitch.EventOnKeyDown = DoNothing;
    Cell.GoodsSwitch.EventOnDoubleClick = DoNothing;
    Cell.GoodsSwitch.EventOnKeyPress = DoNothing;
    Cell.GoodsSwitch.EventOnKeyUp = DoNothing;
    
    /*从flash读取保存的数据*/
    if(HAL_OK != ReadDataFromFlash())
    {
        
    }
    
    /*Flash里面没有存有id或者addr*/
    if( (0==Cell.Column) || (0==Cell.CanID) )
    {
        //RepuestIdAndAddr(cell, hcan);
    }
    
    return;
}


HAL_StatusTypeDef RepuestIdAndAddr(CAN_HandleTypeDef *hcan)
{    
    /**/
    if(Cell.AllocaOneLayerDone == CELL_ALLOCATE_ONE_LAYER_DONE)
    {
        Cell.AllocaOneLayerDone = CELL_NONE;
        /*使能下一层板子请求ID*/
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);  
        Delay(20);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);  
    }
    
    /*收到主机发送的请求ID使能信号*/
    if(Cell.RequestIdEnable == CELL_REQUEST_ENABLE)
    {      
        Cell.RequestIdEnable = CELL_NONE;
        /*等待引脚使能信号*/
        while(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10));
    }
    else
    {
        return HAL_OK;
    }
    
    /*Data[0]=0x02,请求主机分配ID*/
    hcan->pRxMsg->Data[0] = 0x00;
    hcan->pTxMsg->Data[0] = CELL_REQUEST_ID;  
    HAL_CAN_Transmit_IT(hcan);
    
    /*等待主机分配的ID发送过来*/    
    uint32_t tickstart = HAL_GetTick();
    while(hcan->pRxMsg->Data[0] != CELL_GET_ID)
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
    
    /*指示灯*/
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);   
    Delay(50);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);  
    
    /*使能下一个板子请求ID*/
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);  
    Delay(2);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);   
    
    return HAL_OK;
}


void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{  
    HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
    CheckCommand(hcan);  
}

int rxcnt = 0;
void CheckCommand(CAN_HandleTypeDef* hcan)
{
    rxcnt++;
    switch(hcan->pRxMsg->Data[0])
    {
      case CELL_NONE:;
        break;
      case CELL_REQUEST_ENABLE :
        Cell.RequestIdEnable = CELL_REQUEST_ENABLE;
        Cell.AllocaOneLayerDone = CELL_NONE;
        break;  
      case CELL_ALLOCATE_ONE_LAYER_DONE :
        Cell.AllocaOneLayerDone = CELL_ALLOCATE_ONE_LAYER_DONE;
        break;        
      case CELL_DELIVER :
        Cell.Deliver = CELL_DELIVER;
        Cell.ReceiveRow = hcan->pRxMsg->Data[5];  
        Cell.ReceiveColumn = (hcan->pRxMsg->Data[6]<<8)|hcan->pRxMsg->Data[7];    
        Cell.GoodsCount = hcan->pRxMsg->Data[4];
        break;
      case CELL_RESET:
        CellReset();
        break;
      case CELL_RESET_SYSTEM:
        NVIC_SystemReset();
        break;
      default:;
        break;
    }    
    return;
}

// 复位单元体
void CellReset(void)
{
    while(Cell.GoodsSwitch.IsDown)
    {
        SetDuty(&htim4,TIM_CHANNEL_1,100);
        SetDuty(&htim4,TIM_CHANNEL_2,0);
        SetDuty(&htim4,TIM_CHANNEL_3,100);
        SetDuty(&htim4,TIM_CHANNEL_4,0);
    }
    SetDuty(&htim4,TIM_CHANNEL_1,0);
    SetDuty(&htim4,TIM_CHANNEL_2,0);
    SetDuty(&htim4,TIM_CHANNEL_3,0);
    SetDuty(&htim4,TIM_CHANNEL_4,0);
    return;
}

// 出货
void Deliver(CAN_HandleTypeDef* hcan)
{
    if(Cell.Deliver)
    {
        Cell.Deliver = 0;   
        /*如果是该单元体出货*/
        if( (Cell.ReceiveRow==Cell.Row) && (Cell.ReceiveColumn==Cell.Column) )
        {
            uint8_t n = Cell.GoodsCount;
            uint8_t done = 0;      
            for(uint8_t i=0; i<n; i++)
            {
                done = 0;
                uint32_t tickstart = HAL_GetTick();                
                SetDuty(&htim4,TIM_CHANNEL_1,100);
                SetDuty(&htim4,TIM_CHANNEL_2,0);
                SetDuty(&htim4,TIM_CHANNEL_3,100);
                SetDuty(&htim4,TIM_CHANNEL_4,0);
                while(!done)
                {          
                    if(Cell.GoodsSwitch.IsDown)
                    {
                        done = 1;
                        while(Cell.GoodsSwitch.IsDown){};
                        Cell.GoodsCount--;
                    }                         
                    ////////////////////////////////////////////// 
                    //  超时未检测到货物被送出，超时时间需要考量
                    ////////////////////////////////////////////// 
                    if(HAL_GetTick() - tickstart > 20000)
                    {
                        //break;
                    }
                }// end while       
            }// end for 
            
            /*货物送完，电机停转*/
            SetDuty(&htim4,TIM_CHANNEL_1,0);
            SetDuty(&htim4,TIM_CHANNEL_2,0);
            SetDuty(&htim4,TIM_CHANNEL_3,0);
            SetDuty(&htim4,TIM_CHANNEL_4,0);
            
            /*通知主机出货完成*/
            hcan->pTxMsg->Data[0] = CELL_DELIVER_DONE;
            hcan->pTxMsg->Data[6] = (uint8_t) (0xFF & (Cell.Column>>8) );  
            hcan->pTxMsg->Data[7] = (uint8_t) (0xFF & (Cell.Column>>0) );  
            int retrycnt = 10;
            while( HAL_OK != HAL_CAN_Transmit_IT(hcan) && (retrycnt-->0))
            {
                Delay(10);
            }   
            /*通知主机失败*/
            if(retrycnt < 0)
            {
                
            }
        }
        return ;
    }  
    return;
}

void DoNothing(void)
{
    return;
}

static void Delay(uint32_t ms)
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


