#include "BAT.h"

static esp_adc_cal_characteristics_t AdcChars;      // ADC校准参数结构体
static const adc1_channel_t Channel = ADC1_CHANNEL_7;       // ADC1通道7
static const adc_bits_width_t Width = ADC_WIDTH_BIT_12;     // ADC分辨率12位
static const adc_atten_t Atten = ADC_ATTEN_DB_12;       // ADC衰减12dB
static const adc_unit_t Unit = ADC_UNIT_1;      // ADC单元1

/**
 * @brief 获取电池电压（放大 1000 倍后写入 sReg）
 */
void BAT_GetVoltage(void)
{
    unsigned int uiTmp = esp_adc_cal_raw_to_voltage(analogRead(BAT_Pin), &AdcChars);
    sReg[BAT] = (short)((float)uiTmp * 3.97f);       // 放大了 1000 倍
}

/**
 * @brief 初始化 ADC 参数（宽度、衰减、校准）
 */
void BAT_Init(void)
{
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK){printf("eFuse Two Point: Supported\n");}
    else{printf("eFuse Two Point: NOT supported\n");}
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK){printf("eFuse Vref: Supported\n");}
    else{printf("eFuse Vref: NOT supported\n");}

    adc1_config_width(Width);
    adc1_config_channel_atten(Channel, Atten);
    esp_adc_cal_characterize(Unit, Atten, Width, 0, &AdcChars);
}

/**
 * @brief 获取电压进程函数
 */
void BAT_GetVoltage_Proc(void)
{
    BAT_GetVoltage();
}
