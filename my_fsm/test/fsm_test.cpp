#include <stdlib.h>
#include <stdint.h>
#include <gtest.h>

#include "my_fsm.h"

class FsmTestInit : ::testing::test {
    Fsm_t fsm;
    int prvData;

    int MockState(void *data){}

    void SetUp() override
    {
        FsmConfig_t config = {
            .maxStateCount = 5,
            .maxEventCount = 30,
            .prvData = &prvData,
        };
        FsmPort_t port = {
            .mallocFptr = malloc,
            .freeFptr = free,
            .printfFptr = printf,
        };
        fsm = FsmCreate(&config,&port,"TEST");
    }

    void TearDown() override
    {
        FsmDelete(fsm);
    }
}

TEST_F(FsmTestInit,初始化测试：给定不合适的参数)
{
    ASSERT_EQ(FsmAddState(fsm,0,NULL,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddState(fsm,6,MockState,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddState(fsm,6,NULL,NULL),FSM_INVALID_ARG);

    ASSERT_EQ(FsmAddStateEntry(fsm,0,NULL,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddStateEntry(fsm,6,MockState,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddStateEntry(fsm,6,NULL,NULL),FSM_INVALID_ARG);

    ASSERT_EQ(FsmAddStateExit(fsm,0,NULL,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddStateExit(fsm,6,MockState,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddStateExit(fsm,6,NULL,NULL),FSM_INVALID_ARG);

    ASSERT_EQ(FsmAddEvent(fsm,0,0,6,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddEvent(fsm,0,6,0,NULL),FSM_INVALID_ARG);
    ASSERT_EQ(FsmAddEvent(fsm,0,7,8,NULL),FSM_INVALID_ARG);
}

TEST_F(FsmTestInit,初始化测试：添加重复状态)
{
    FsmAddState(fsm,0,MockState,NULL);
    EXPECT_EQ(FsmAddState(fsm,0,MockState,NULL),FSM_MULTI_DEFINE);
}

TEST_F(FsmTestInit,初始化测试：事件重复)
{
    FsmAddState(fsm,0,MockState,NULL);
    FsmAddState(fsm,1,MockState,NULL);
    FsmAddEvent(fsm,0,0,1,NULL);

    EXPECT_EQ(FsmAddEvent(fsm,0,0,1,NULL),FSM_MULTI_DEFINE);
    EXPECT_EQ(FsmAddEvent(fsm,1,0,1,NULL),FSM_OK);
    EXPECT_EQ(FsmAddEvent(fsm,2,1,0,NULL),FSM_OK);
}

TEST_F(FsmTestInit,初始化测试：事件转移到无效的状态)
{
    FsmAddState(fsm,0,MockState,NULL);
    FsmAddState(fsm,1,MockState,NULL);

    EXPECT_EQ(FsmAddEvent(fsm,0,0,2,NULL),FSM_INVALID_STATE);
    EXPECT_EQ(FsmAddEvent(fsm,0,2,0,NULL),FSM_INVALID_STATE);
}

TEST_F(FsmTestInit,初始化测试：事件转移到不存在的状态)
{
    FsmAddState(fsm,1,MockState,NULL);

    EXPECT_EQ(FsmAddEvent(fsm,0,1,6,NULL),FSM_INVALID_ARG);
    EXPECT_EQ(FsmAddEvent(fsm,0,6,1,NULL),FSM_INVALID_ARG);
}

TEST_F(FsmTestInit,初始化测试：行为重复)
{
    FsmAddState(fsm,0,MockState,NULL);
    FsmAddState(fsm,1,MockState,NULL);
    FsmAddEvent(fsm,0,0,1,NULL);
    FsmAddAction(fsm,0,0,)

    EXPECT_EQ(FsmAddEvent(fsm,0,0,1,NULL),FSM_MULTI_DEFINE);
    EXPECT_EQ(FsmAddEvent(fsm,1,0,1,NULL),FSM_OK);
    EXPECT_EQ(FsmAddEvent(fsm,2,1,0,NULL),FSM_OK);
}

TEST_F(FsmTestInit,初始化测试：为不存在的事件+状态组合指定动作)
{
    FsmAddState(fsm,0,MockState,NULL);
    FsmAddState(fsm,1,MockState,NULL);
    FsmAddEvent(fsm,0,0,1,NULL);

    EXPECT_EQ(FsmAddAction(fsm,0,1))
}