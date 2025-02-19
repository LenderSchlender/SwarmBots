#include "BalanceController.h"

BalanceController::BalanceController(SingleMotorOutput *left, SingleMotorOutput *right,
    RotaryEncoder *leftEnc, RotaryEncoder *rightEnc, MPU6050 *mpu) {
        this->left = left;
        this->right = right;
        this->leftEnc = leftEnc;
        this->rightEnc = rightEnc;
        this->mpu = mpu;
}

void BalanceController::init() {

}

void BalanceController::tick() {

}