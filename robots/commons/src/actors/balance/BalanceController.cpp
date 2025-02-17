#include "BalanceController.h"

BalanceController::BalanceController(SingleMotorOutput *left, SingleMotorOutput *right,
    RotaryEncoder *leftEnc, RotaryEncoder *rightEnc) {
        this->left = left;
        this->right = right;
        this->leftEnc = leftEnc;
        this->rightEnc = rightEnc;
}

void BalanceController::init() {

}

void BalanceController::tick() {
    
}