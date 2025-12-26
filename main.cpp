#include <string>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hd44780_pico.h"
#include "ADC8bit.h"
#include "hardware/irq.h"


void core1_main()
{
	uint32_t adcbyte;
	ADC0_8Bit adc8;
	multicore_fifo_drain();
	while(true)
	{
		adcbyte=adc8.analg2_8bit();
		if(multicore_fifo_wready())
			multicore_fifo_push_blocking(adcbyte);
		sleep_ms(100);
	}

}

int main()
{
	multicore_reset_core1();
	sleep_ms(1000);
	multicore_launch_core1(core1_main);
	HD44780_1602 lcd;
	LCDField field{lcd,0,0,16};
	uint32_t adcbyte;
	std::string text;
	while(true)
	{
		if(multicore_fifo_rvalid())
		{
			adcbyte=multicore_fifo_pop_blocking();
			text=std::to_string(adcbyte);
			field.writeField(text.c_str());
		}	
		else
		{
			field.writeField("Nothing!!");
			sleep_ms(2000);
			field.clearField();
		}	
	}

}
