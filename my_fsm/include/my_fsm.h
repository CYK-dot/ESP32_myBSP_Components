#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>

typedef enum{
    STATE_ENTRY = 0,
    STATE_EXIT = 1,
    STATE_MAIN = 2,
}FsmStateFlag_t;

typedef void* Fsm_t;

typedef int   (*fsmFptr)     (Fsm_t fsm,FsmStateFlag_t stateFlag,void *prvData);
typedef void* (*fsmAllocate) (size_t size);
typedef void  (*fsmFree)     (void *data);
typedef int   (*fsmLogger)   (const char *format,...);

typedef struct {
    uint32_t maxStateCount;  ///< 最多有多少个状态
    uint32_t maxEventCount;  ///< 最多有多少个事件
    void    *prvData;        ///< 私有数据的位置(可以为NULL)
}FsmConfig_t;

typedef struct 
{
    fsmAllocate mallocFptr;  ///< 使用的内存分配器
    fsmFree freeFptr;        ///< 使用的内存释放器
    fsmLogger printfFptr;    ///< 打印日志的函数(可以为NULL)
}FsmPort_t;

typedef enum
{
    FSM_OK = 0,
    FSM_INVALID_ARG,
    FSM_INVALID_STATE,
    FSM_MULTI_DEFINE,
}FsmErr_t;

//初始化：创建与删除
Fsm_t FsmCreate(const FsmConfig_t *config,const FsmPort_t *port,const char *fsmName);
void  FsmDelete(Fsm_t fsm);

//初始化：状态的入口、出口、反复执行函数
int FsmAddState(Fsm_t fsm,fsmFptr stateFunc,const char *stateName);

//初始化：状态的转移和事件的触发
int FsmAddTransfer(Fsm_t fsm,const char *stateFromName,const char *stateToName,const char *EventName);
int FsmAddAction(Fsm_t fsm,uint32_t stateFrom,uint32_t eventID,fsmFptr actionFunc,const char *message);

//使用：
int FsmPush(Fsm_t fsm,uint32_t eventID,const char *message);
int FsmPushDetailed(Fsm fsm,uint32_t eventID,char *FILE,int line,const char *message);
int FsmOn(Fsm_t fsm);

//调试：
int FsmLogAll(Fsm_t fsm);
uint32_t FsmGetState(Fsm_t fsm);


