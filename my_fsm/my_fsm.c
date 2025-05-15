#include "my_fsm.h"

typedef struct {
    // 配置
    FsmConfig_t *config;
    FsmPort_t *port;

    // 当前状态
    uint32_t currentState;

    // 状态与事件表
    fsmFptr *stateList;   
    fsmFptr *entryList;   
    fsmFptr *exitList;  
    char    *stateName;
    char    *eventName;

    // 状态转移表
    uint32_t *transferTable; // arr[状态][事件]=下一个状态
    fsmFptr  *actionTable;   // arr[状态][事件]=行为状态函数
    char     *actionName; 
} FsmCtx;

Fsm_t FsmCreate(const FsmConfig_t *config,const FsmPort_t *port,const char *fsmName)
{
    if (config == NULL || port == NULL || port->mallocFptr == NULL ||
        port->freeFptr == NULL || fsmName == NULL) {
            return NULL;
    }

    FsmCtx *handle = port->mallocFptr(sizeof(FsmCtx));

    handle->stateList = port->mallocFptr(sizeof(fsmFptr*)*config->maxStateCount);
    handle->entryList = port->mallocFptr(sizeof(fsmFptr*)*config->maxStateCount);
    handle->exitList  = port->mallocFptr(sizeof(fsmFptr*)*config->maxStateCount);
    handle->stateName = port->mallocFptr(sizeof(char*)*config->maxStateCount);
    handle->eventName = port->mallocFptr(sizeof(char*)*config->maxEventCount);

    handle->transferTable = port->mallocFptr(sizeof(uint32_t*)*config->maxStateCount*config->maxEventCount);
    handle->actionTable   = port->mallocFptr(sizeof(fsmFptr*)*config->maxStateCount*config->maxEventCount);
    handle->actionName    = port->mallocFptr(sizeof(char*)*config->maxStateCount*config->maxEventCount);


}