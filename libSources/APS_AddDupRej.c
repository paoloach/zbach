struct s1 {
  uint16 shortAddress;
  uint8 apsCounter;
  uint8 zgApscDupRejTimeoutCount;
  struct s1 * next;
}

static struct s1 * listLastMsg;

void APS_AddDupRej(uint8 ApsCounter, uint16 shortAddress){
	uint16 index=0;
	if (*listLastMsg != 0){
		index++;
		struct s1 * iter = listLastMsg;
		while (*iter != null){
		   index++;
		   if (iter->next == NULL){
			 break;
		   }
		   iter = iter->next;
		}
	}
	if (index >= zgApsMinDupRejTableSize){
		return;
	}
	struct s1 * newElement = osal_mem_alloc(sizeof(struct s1);
	if (newElement == NULL){
	   return;
	}
	newElement->shortAddress = shortAddress;
	newElement->apsCounter = apsCounter;
	newElement->zgApscDupRejTimeoutCount = zgApscDupRejTimeoutCount;
	newElement->next = NULL;
	if(*listLastMsg == NULL){
	   *listLastMsg = newElement;
	   if((_NIB.CapabilityFlags & 0x04) == 0){
		   uint32 clock = osal_GetSystemClock();
		   APS_lastDupTime = clock;
	   }
	} else {
	   s1->next = val;
	   val=next;
	}
	if((_NIB.CapabilityFlags & 0x04) == 0){
		return;
	}
	if (osal_get_timeoutEx(APS_TaskID, 8) != 0 ){
		return;
	};
	osal_start_reload_timer(APS_TaskID, 8, zgApscDupRejTimeoutInc);
    
}
