
#include "key.h"


void KeyScan(KeyTypedef *key,uint8_t scanInterval ,bool isKeyPress)
{
  //³õÊ¼»¯°´¼ü¼ì²â
  if(!(key->InitDisable))
  {
    key->PressCnt = 0;
		key->LongPressCnt = 0;
    key->InitDisable = true;
    key->KeyDown = false;
		key->EventOnKeyDown = NULL;
		key->EventOnKeyUp = NULL;
		key->EventOnDoubleClick = NULL;
		key->EventOnKeyPress = NULL;
  }
  
	if(key->OnDoubleClickEnable)
	{
		key->DoubleClickCnt += scanInterval;
	}
  if(isKeyPress)
  {
    key->PressCnt += scanInterval;
    if((key->PressCnt >= 20) && !key->KeyDown)
    {
      key->KeyDown = true;
			if(key->OnKeyDownEnable)
			{
				key->EventOnKeyDown();
			}
      Buzzer(90,1,0);
      if(key->OnDoubleClickEnable)
			{
				key->DoubleClickCheckFlag = !key->DoubleClickCheckFlag;
				if(key->DoubleClickCheckFlag)
				{
					key->Click_S_Time = key->DoubleClickCnt;
				}
				else
				{
					key->Click_E_Time = key->DoubleClickCnt;
				}
				if((key->Click_E_Time > key->Click_S_Time)&&((key->Click_E_Time - key->Click_S_Time) < 200) )
				{
					key->EventOnDoubleClick();
					key->DoubleClickCnt = 0;
				}
				else if((key->Click_S_Time > key->Click_E_Time)&&((key->Click_S_Time - key->Click_E_Time) < 200) )
				{
					key->EventOnDoubleClick();
					key->DoubleClickCnt = 0;
				}
			}
    }
    if(key->PressCnt >= 400)
		{
			key->IsLongPress = true;
			key->DoubleClickCnt = 0;
		}
		if(key->IsLongPress && key->OnKeyPressEnable)
		{
			key->LongPressCnt += scanInterval;
			if(key->LongPressCnt >= 80)
			{
				key->EventOnKeyPress();
				key->LongPressCnt = 0;
				Buzzer(40,1,1);
			}
		}
  }
  else
  {
		if((key->KeyDown) && (key->OnKeyUpEnable))
		{
			key->EventOnKeyUp();
		}
		key->PressCnt = 0;
		key->LongPressCnt = 0;
    key->KeyDown = false;
		key->IsLongPress = false;
  }
  return;
}

