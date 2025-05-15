#include <soc/soc_caps.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/ledc.h>
#include <math.h>

#include "iot_servo.h"
#include "flighter_slave.h"

#define SLAVE_MOTOR_TIMER_DUTY_MAX ((1ULL << SLAVE_MOTOR_TIMER_SOLUTION) - 1UL)

ledc_channel_t FlighterPWM::channelCount = (ledc_channel_t)0;
static const char *TAG = "Flighter-Slave";

/**
 * @brief Construct a new Flighter Servo:: Flighter Servo object
 * 
 * @param gpioServo 选择那一路GPIO
 * @param maxAngle 舵机的转角
 */
FlighterServo::FlighterServo(gpio_num_t gpioServo,uint16_t maxAngle)
{
    if (FlighterPWM::channelCount == SOC_LEDC_CHANNEL_NUM){
        ESP_LOGE(TAG,"舵机设备构造函数:超过最大允许的PWM设备数量");
        this->channel = 255;
        return;
    }
    else{
        //配置外设
        servo_config_t servo_cfg = {
            .max_angle = maxAngle,
            .min_width_us = 500,
            .max_width_us =2500,
            .freq = 50,
            .timer_number = SLAVE_SERVO_TIMER_PERI,
            .channels = {
                .servo_pin = {
                    (gpio_num_t)gpioServo,
                },
                .ch = {
                    (ledc_channel_t)FlighterPWM::channelCount,
                },
            },
            .channel_number = 1,
        };
        esp_err_t err = iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG,"舵机设备构造函数:外设初始化失败！");
            return;
        }

        //配置对象
        this->channel = this->channelCount;
        FlighterPWM::channelCount = (ledc_channel_t)(FlighterPWM::channelCount + 1);
        ESP_LOGW(TAG,"舵机设备构造函数完成:下一个编号%d",(ledc_channel_t)FlighterPWM::channelCount);
    }
}

/**
 * @brief 控制舵机旋转
 * 
 * @param angle 目标角度
 * @return esp_err_t 
 */
esp_err_t FlighterServo::setPos(float angle)
{
    if (this->channel != 255){
        return iot_servo_write_angle(LEDC_LOW_SPEED_MODE,this->channel,angle);
    }
    ESP_LOGE(TAG,"舵机设备控制：控制的通道无效");
    return ESP_ERR_INVALID_ARG;
}

/**
 * @brief 获取舵机角度
 * 
 * @return float 
 */
float FlighterServo::getPos(void)
{
    float pos;
    iot_servo_read_angle(LEDC_LOW_SPEED_MODE,this->channel,&pos);
    return pos;
}

/**
 * @brief Construct a new Flighter Motor Bidir:: Flighter Motor Bidir object
 * 
 * @param gpioMotorA 正转GPIO
 * @param gpioMotorB 反转GPIO
 * @param isHighInverse 高电平导通填false，低电平导通填true
 */
FlighterMotorBidir::FlighterMotorBidir(gpio_num_t gpioMotorA,gpio_num_t gpioMotorB,bool isHighInverse)
{
    this->gpioA = gpioMotorA;
    this->gpioB = gpioMotorB;
    
    if((FlighterPWM::channelCount +1) == SOC_LEDC_CHANNEL_NUM){
        ESP_LOGE(TAG,"双向电机构造函数:超过最大允许的PWM设备数量");
        this->channelA = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
        this->channelB = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
        return;
    }
    else{
        // 配置定时器
        ledc_timer_config_t pwm_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = SLAVE_MOTOR_TIMER_SOLUTION,
            .timer_num = SLAVE_MOTOR_TIMER_PERI,
            .freq_hz = 1000,
            .clk_cfg = LEDC_USE_APB_CLK,//LEDC_USE_XTAL_CLK
            .deconfigure = false,
        };
        esp_err_t err = ledc_timer_config(&pwm_conf);
        if (err != ESP_OK) {
            this->channelA = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
            this->channelB = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
            ESP_LOGE(TAG,"双向电机构造函数：LEDC配置错误，错误码%X",err);
            return;
        }

        // 配置定时器通道A
        ledc_channel_config_t ledc_channel = {
            .gpio_num       = this->gpioA,
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = (ledc_channel_t)FlighterPWM::channelCount,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = SLAVE_MOTOR_TIMER_PERI,
            .duty           = 0, 
            .hpoint         = 0,
            .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags = {.output_invert = 0},
        };
        err = ledc_channel_config(&ledc_channel);
        if (err != ESP_OK) {
            this->channelA = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
            this->channelB = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
            ESP_LOGE(TAG,"双向电机构造函数：LEDC通道配置错误，错误码%X",err);
            return;
        }
        this->gpioA = gpioMotorA;
        this->channelA = FlighterPWM::channelCount;
        FlighterPWM::channelCount = (ledc_channel_t)(FlighterPWM::channelCount + 1);

        // 配置定时器通道B
        ledc_channel = {
            .gpio_num       = this->gpioB,
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = (ledc_channel_t)FlighterPWM::channelCount,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = SLAVE_MOTOR_TIMER_PERI,
            .duty           = 0, 
            .hpoint         = 0,
            .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags = {.output_invert = 0},
        };
        err = ledc_channel_config(&ledc_channel);
        if (err != ESP_OK) {
            this->channelA = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
            this->channelB = (ledc_channel_t)(SOC_LEDC_CHANNEL_NUM + 1);
            ESP_LOGE(TAG,"双向电机构造函数：LEDC通道配置错误，错误码%X",err);
            return;
        }
        this->channelB = FlighterPWM::channelCount;
        this->gpioB = gpioMotorB;
        FlighterPWM::channelCount = (ledc_channel_t)(FlighterPWM::channelCount + 1);

        // 结束
        this->duty = 0;
        this->isInverse = isHighInverse;
        this->isForward = false;
        ESP_LOGI(TAG,"双向电机构造函数：完成，下一个LEDC通道为%hhd",FlighterPWM::channelCount);
    }
}

/**
 * @brief 设置输出占空比
 * 
 * @param dutyToSend 
 * @return esp_err_t 
 */
esp_err_t FlighterMotorBidir::setDuty(float dutyToSend)
{
    this->duty = dutyToSend;
    // 停转
    if (fabsf(duty) <= SLAVE_MOTOR_ZERO_FLOAT) {
        if (this->isInverse == true) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelA,SLAVE_MOTOR_TIMER_DUTY_MAX);
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelB,SLAVE_MOTOR_TIMER_DUTY_MAX);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelA);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelB);
            ESP_LOGD(TAG,"双向电机:通道%d占空比为%lld,通道%d占空比为%lld",(int)this->channelA,(long long int)SLAVE_MOTOR_TIMER_DUTY_MAX,(int)this->channelB,(long long int)SLAVE_MOTOR_TIMER_DUTY_MAX);
        }
        else {
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelA,0);
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelB,0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelA);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelB);
            ESP_LOGD(TAG,"双向电机:通道%d占空比为%d,通道%d占空比为%d",this->channelA,0,this->channelB,0);
        }
    }
    // 正转
    else if (duty > SLAVE_MOTOR_ZERO_FLOAT) {
        if (this->isInverse == true) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelA,(1.0f-dutyToSend)*(float)SLAVE_MOTOR_TIMER_DUTY_MAX);
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelB,SLAVE_MOTOR_TIMER_DUTY_MAX);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelA);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelB);
            ESP_LOGI(TAG,"双向电机:通道%d占空比为%lld,通道%d占空比为%lld",this->channelA,(long long int)((1.0f-dutyToSend)*(float)SLAVE_MOTOR_TIMER_DUTY_MAX),this->channelB,SLAVE_MOTOR_TIMER_DUTY_MAX);
        }
        else {
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelA,dutyToSend*(float)SLAVE_MOTOR_TIMER_DUTY_MAX);
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelB,0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelA);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelB);
            ESP_LOGI(TAG,"双向电机:通道%d占空比为%d,通道%d占空比为%d",this->channelA,(int)(dutyToSend*(float)SLAVE_MOTOR_TIMER_DUTY_MAX),this->channelB,0);
        }
    }
    // 反转
    else {
        if (this->isInverse == true) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelA,SLAVE_MOTOR_TIMER_DUTY_MAX);
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelB,(int)((1.0f+dutyToSend)*(float)SLAVE_MOTOR_TIMER_DUTY_MAX));
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelA);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelB);
        }
        else {
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelA,0);
            ledc_set_duty(LEDC_LOW_SPEED_MODE,this->channelB,(int)(-dutyToSend*(float)SLAVE_MOTOR_TIMER_DUTY_MAX));
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelA);
            ledc_update_duty(LEDC_LOW_SPEED_MODE,this->channelB);
        }
    }
    return ESP_OK;
}

/**
 * @brief 获取上次输出占空比
 * 
 * @return float 
 */
float FlighterMotorBidir::getDuty(void)
{
    return this->duty;
}
