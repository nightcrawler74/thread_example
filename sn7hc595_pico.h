//		****pico implementation******
#ifndef _LCD
#define _LCD
#include <iostream>
#include "pico/stdlib.h"
#include <unistd.h>
#define OBDLED 25
#define SRCLR 21
#define SRCLK 16
#define RCLK 20
#define SER 12
#define MSB 1<<15		//16 bit word MSB boundary
#define SHIFT_REG_CONTROL_MASK (1<SRCLR|1<<SRCLK|1<<RCLK|1<<SER)
#define LOW 0
#define HIGH 1
struct SHIFT_16Bit
{
	SHIFT_16Bit()
	{
		gpio_init_mask(SHIFT_REG_CONTROL_MASK);
		gpio_set_dir_out_masked(SHIFT_REG_CONTROL_MASK);
		gpio_pull_up(SRCLR);	//active low
		gpio_pull_down(SRCLK);
		gpio_pull_down(SER);
		gpio_pull_down(RCLK);
													
		gpio_put(SRCLK,LOW);
		gpio_put(SRCLR,LOW);
		gpio_put(SRCLR,HIGH);
		gpio_put(OBDLED,HIGH);

	
	}
	~SHIFT_16Bit()
	{
	
	}
	unsigned short shiftReg;
	void __attribute__ ((noinline)) shiftOut(unsigned short num)
	{
		clrShift();
		sleep_us(100);
		int i{};
		unsigned short mask{MSB};
		std::cout << "0b";
		for(;i<16;i++)
		{
			if(num&mask)
			{
				std::cout << 1;
				shiftReg|=mask;
				gpio_put(SER,HIGH);
				sleep_us(100);
				gpio_put(SRCLK,HIGH);
				sleep_us(100);
				gpio_put(SRCLK,LOW);
				sleep_us(100);
				gpio_put(SER,LOW);
				sleep_us(100);

			}
			else
			{
				gpio_put(SRCLK,HIGH);
				sleep_us(100);
				gpio_put(SRCLK,LOW);
				sleep_us(100);

				std::cout << 0;
			}
			mask>>=1;
		}
		gpio_put(RCLK,HIGH);
		sleep_us(100);
		gpio_put(RCLK,LOW);
		sleep_us(100);
		//std::cout << std::endl;
		sleep_ms(1);

	}
	void clrShift()
	{
		gpio_put(SRCLR,LOW);
		gpio_put(SRCLR,HIGH);

	}
};
#endif
