#include "LPF.h"

// 注意，角速度为快环，LPF 带宽必须足够大但又能压制一些噪声，不建议历史值占比超过 1%
LowPassFilter LPF_GyroX(0.01f);
LowPassFilter LPF_GyroY(0.01f);
LowPassFilter LPF_GyroZ(0.01f);

LowPassFilter LPF_Roll(0.3f);
LowPassFilter LPF_Joyy(0.2f);
