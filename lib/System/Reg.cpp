#include "Reg.h"

short sReg[REGSIZE] = {0};

/**
 * @brief 初始化寄存器
 */
void Reg_Init(void)
{
    sReg[REGLAST] = REGLAST - 1;
}
