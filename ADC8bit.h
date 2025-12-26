#include "hardware/adc.h"
#define ADC0 26
#define ADC_INPUTSELCT_26 0 
#define MAX255 255
#define AVERAGE 10

struct ADC0_8Bit
{
	ADC0_8Bit():vref{3.284f},_12bit{(1<<12)-1},DAconv{vref/_12bit}
		{
			adc_init();
			adc_select_input(ADC_INPUTSELCT_26);
			adc_gpio_init(ADC0);
			gpio_disable_pulls(ADC0);

		}	
		volatile uint32_t unit{};
		float _12bit;
		float vref;
		const float DAconv;
		float voltage;

		uint8_t analg2_8bit()
		{
			int c;
			uint8_t avg{};
			voltage=0;
			for( c = 0;c < AVERAGE; c++ )
			{	
				unit = adc_read();
				sleep_ms(10);
				voltage += unit*DAconv;
			}
			voltage /= AVERAGE;
			return ( voltage/vref ) * MAX255;

		}
};
