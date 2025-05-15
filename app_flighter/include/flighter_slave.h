#pragma once

#include <stdint.h>
#include <soc/soc_caps.h>
#include <esp_err.h>
#include <driver/ledc.h>

#include "flighter_ptl.h"

///< 配置：舵机使用哪一个定时器
#define SLAVE_SERVO_TIMER_PERI LEDC_TIMER_0
///< 配置：DRV8870电机使用哪一个定时器
#define SLAVE_MOTOR_TIMER_PERI LEDC_TIMER_1
///< 配置：电机占空比小于多少视为0
#define SLAVE_MOTOR_ZERO_FLOAT 0e-3
///< 配置：DRV8870电机定时器占空比的精度
#define SLAVE_MOTOR_TIMER_SOLUTION LEDC_TIMER_10_BIT

/**
 * @brief PWM设备基类
 * 
 */
class FlighterPWM{
    public:
        static ledc_channel_t channelCount;
};

/**
 * @brief 舵机类
 * 
 */
class FlighterServo : FlighterPWM{
    private:
        uint8_t channel;
    public:
        FlighterServo(gpio_num_t gpioServo,uint16_t maxAngle);
        esp_err_t setPos(float angle);
        float getPos(void);
};

/**
 * @brief 双向电机类(DRV8870模板)
 * 
 */
class FlighterMotorBidir : FlighterPWM{
    private:
        bool isForward   : 1;
        bool isInverse   : 1;
        gpio_num_t gpioA : 7;
        gpio_num_t gpioB : 7;
        ledc_channel_t channelA : 4;
        ledc_channel_t channelB : 4;
        float duty;
    public:
        FlighterMotorBidir(gpio_num_t gpioMotorA,gpio_num_t gpioMotorB,bool isHighInverse);
        esp_err_t setDuty(float duty);
        float getDuty(void);
};

/**
 * @brief 单向电机类(DRV8870模板)
 * 
 */
class FlighterMotorSidir : FlighterPWM{
    private:
        gpio_num_t gpioA;
        gpio_num_t gpioB;
        float duty;
        bool isBrakePWM : 1;
        bool isForward  : 1;
        uint8_t channel : 5;
    public:
        FlighterMotorSidir(gpio_num_t gpioMotorA,gpio_num_t gpioMotorB,bool isBrakeForPWM);
        esp_err_t setDuty(float duty);
        float getDuty(void);
};
