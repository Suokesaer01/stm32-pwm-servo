# STM32 PWM 驱动舵机

## 项目简介

基于 STM32F103 + 标准库，用 **TIM2 通道 2（PA1）输出 50Hz PWM**，通过改变高电平宽度（0.5–2.5ms）控制舵机转动角度。当前版本固定输出 1.5ms 中位。

## 实验现象

- 下载程序后，舵机转到 **中位（约 90°）** 并保持
- 用示波器测 PA1：周期 **20ms（50Hz）**，高电平 **1.5ms**

## 硬件

| 模块 | 型号 | 说明 |
|---|---|---|
| 主控 | STM32F103C8T6 | 最小系统板，72MHz |
| 舵机 | SG90 / MG995 | 5V 供电，PWM 信号 |
| 信号线 | PA1 | TIM2_CH2 |

## 接线

| 舵机线 | 接到 |
|---|---|
| 信号线（橙/黄） | PA1 |
| 电源（红） | 外部 5V（建议独立供电） |
| 地（棕/黑） | GND（必须与主控共地） |

> ⚠️ 舵机启动电流可达几百 mA，**不要从开发板的 5V 取电**，用独立 5V 电源，否则可能拉低电压导致主控复位。

## 参数计算

```
72MHz / 72 (PSC)  = 1MHz  → 计数 1 次 = 1µs
1MHz / 20000 (ARR) = 50Hz → 周期 = 20ms
```

- `TIM_Prescaler = 72 - 1`
- `TIM_Period = 20000 - 1`
- `TIM_OCMode_PWM1`，`TIM_OCPolarity_High`，通道 2
- 比较值 = 高电平时间（µs）

| 比较值 | 高电平 | 角度 |
|---|---|---|
| 500 | 0.5ms | 0° |
| 1500 | 1.5ms | 90°（中位） |
| 2500 | 2.5ms | 180° |

## 关键代码

`Hardware/pwm.c`：

```c
void pwm_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period        = 20000 - 1;
    TIM_TimeBaseInitStruct.TIM_Prescaler     = 72 - 1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse       = 0;
    TIM_OC2Init(TIM2, &TIM_OCInitStruct);

    TIM_Cmd(TIM2, ENABLE);
}

void pwm_setcompare(uint16_t Compare2)
{
    TIM_SetCompare2(TIM2, Compare2);
}
```

`User/main.c`：

```c
int main(void)
{
    pwm_init();
    OLED_Init();

    while (1)
    {
        pwm_setcompare(1500);   /* 1.5ms → 中位 90° */
    }
}
```

## 踩过的坑 / 注意点

- 舵机和主控**必须共地**，否则信号无效
- 舵机电源要独立，别用开发板 5V
- 周期必须是 20ms（50Hz），不是 50Hz 的舵机型号先查手册
- 比较值单位是 µs（因为 1MHz 计数），不是角度，要自己换算
- 上电瞬间舵机会抖一下，属于正常现象

## 下一步

- [ ] 角度扫描：0° → 180° → 0° 循环，观察转动
- [ ] 用 OLED 显示当前角度
- [ ] 按键/串口输入目标角度
- [ ] 封装 `pwm_set_angle(uint8_t angle)`，内部做 µs 换算

## 开发环境

- Keil MDK uVision 5
- STM32F10x 标准外设库
- 烧录：ST-Link / FlyMcu
