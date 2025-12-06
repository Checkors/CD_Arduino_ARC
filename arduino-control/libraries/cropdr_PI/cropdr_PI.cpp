#include "cropdr_PI.h"

#define SPEED_LIMIT_MM_PER_SEC 1400
//#define CROPDR_PI_DEBUG 0


#define KP 1
#define KI 1
#define KD 0

double pwm_FR; // 0 for front, 1 for back
double pwm_BR;

double pwm_FL; // 0 for front, 1 for back
double pwm_BL;

double enc_FR;
double enc_BR;

double enc_FL;
double enc_BL;

double setpoint_R = 0;
double setpoint_L = 0;

bool stopOverride;

PID Motor_FR(&enc_FR, &pwm_FR, &setpoint_R, KP, KI, KD, 0);
PID Motor_BR(&enc_BR, &pwm_BR, &setpoint_R, KP, KI, KD, 0);

PID Motor_FL(&enc_FL, &pwm_FL, &setpoint_L, KP, KI, KD, 0);
PID Motor_BL(&enc_BL, &pwm_BL, &setpoint_L, KP, KI, KD, 0);

void Motor_PI_Init(void){
    Motor_FR.SetOutputLimits(0, SPEED_LIMIT_MM_PER_SEC);
    Motor_FR.SetMode(1);

    Motor_BR.SetOutputLimits(0, SPEED_LIMIT_MM_PER_SEC);
    Motor_BR.SetMode(1);

    Motor_FL.SetOutputLimits(0, SPEED_LIMIT_MM_PER_SEC);
    Motor_FL.SetMode(1);

    Motor_BL.SetOutputLimits(0, SPEED_LIMIT_MM_PER_SEC);
    Motor_BL.SetMode(1);

    mtrPwmInit();
    Encoder_Init();

    
}


void Motor_PI_Update(void){
    enc_FR = Enc_GetSpeed(0);
    enc_BR = Enc_GetSpeed(1);

    enc_FL = Enc_GetSpeed(2);
    enc_BL = Enc_GetSpeed(3);

    Motor_FR.Compute();
    Motor_BR.Compute();
    Motor_FL.Compute();
    Motor_BL.Compute();

    if (!stopOverride){
        RightMotor1_SetDutyCycle(int(pwm_FR)); // Front
        RightMotor2_SetDutyCycle(int(pwm_BR)); // Back

        LeftMotor1_SetDutyCycle(int(pwm_FL)); // Front
        LeftMotor2_SetDutyCycle(int(pwm_BL)); // Back
    } else {
        RightMotor1_SetDutyCycle(0); // Front
        RightMotor2_SetDutyCycle(0); // Back

        LeftMotor1_SetDutyCycle(0); // Front
        LeftMotor2_SetDutyCycle(0); // Back
    }


    #ifdef CROPDR_PI_DEBUG

    char testStr[64];
    sprintf(testStr, "SETPOINT R: %i, SETPOINT L: %i\r\n", int(setpoint_R), int(setpoint_L));
    Serial.write(testStr);
    sprintf(testStr, "FR_ENC: %i, BR_ENC: %i, FR_PWM: %i, BR_PWM: %i\r\n", int(enc_FR), int(enc_BR), int(pwm_FR), int(pwm_BR) );
    Serial.write(testStr);
    sprintf(testStr, "FL_ENC: %i, BL_ENC: %i, FL_PWM: %i, BL_PWM: %i\r\n", int(enc_FL), int(enc_BL), int(pwm_FL), int(pwm_BL) );
    Serial.write(testStr);

    #endif

}

void Motor_PI_SetRight(int target){

    if (abs(target) < SPEED_LIMIT_MM_PER_SEC){
        if (target > 0){
            stopOverride = false;
            //Motor_FR.SetControllerDirection(DIRECT);
            //Motor_BR.SetControllerDirection(DIRECT);
            setpoint_R = double(abs(target));
        } else if (target == 0){
            stopOverride = true;
            setpoint_R = 0;

        }
        //Implement reverse code here.



    }
}

void Motor_PI_SetLeft(int target){

    if (abs(target) < SPEED_LIMIT_MM_PER_SEC){
        if (target > 0){
            stopOverride = false;
            //Motor_FL.SetControllerDirection(DIRECT);
            //Motor_BL.SetControllerDirection(DIRECT);
            setpoint_L = double(abs(target));
        } else if (target == 0){
            stopOverride = true;
            setpoint_L = 0;

        }
        //Implement reverse code here.



    }
}
