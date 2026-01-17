#ifndef __PID_H__
#define __PID_H__

#include <stdbool.h>

/* 增强型PID控制器结构体 */
typedef struct {
    // 基本参数
    float kp;                  // 比例系数
    float ki;                  // 积分系数
    float kd;                  // 微分系数
    float kff;                 // 前馈系数
    
    // 控制范围
    float minOutput;           // 输出下限
    float maxOutput;           // 输出上限
    float maxIntegral;         // 积分上限
    
    // 状态变量
    float setpoint;            // 设定值
    float measurement;         // 测量值
    float lastMeasurement;     // 上次测量值
    float integral;            // 积分项
    float lastError;           // 上次误差
    
    // 增强功能参数
    float integralSeparationThreshold; // 积分分离阈值
    float deadBand;            // 死区大小
    bool useVariableIntegral;  // 是否使用变速积分
    bool useDerivativePV;      // 是否对测量值求微分
    
    // 输出滤波
    float outputFilter;        // 输出滤波系数(0-1)
    float lastOutput;          // 上次输出值
    
    // 性能指标
    float maxError;            // 最大误差
    float sumSquaredError;     // 误差平方和
    int sampleCount;           // 样本数量
} EnhancedPIDController_t;

void InitEnhancedPIDControllerDefault(EnhancedPIDController_t *pid, 
                               float kp, float ki, float kd, float kff);
float UpdateEnhancedPIDController(EnhancedPIDController_t *pid, 
                                 float setpoint, float measurement, float dt);
void CalculatePIDPerformance(EnhancedPIDController_t *pid, 
                            float *maxError, float *rmse);

#endif // __PID_H__
/* pid.h - Header file for Enhanced PID Controller */
