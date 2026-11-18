#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "pwm.h"

int main(void)
{
	pwm_init();
	OLED_Init();		
	
	
	while (1)
	{
		pwm_setcompare(1500);
	}
}
