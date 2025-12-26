/*

		*****	HD44780 Debug display implementation ********
		using a HD44780 driven 2x16 charcter 1602 display
		interface uses 2x  sn74hc595 shift registers

		************PICO GPIO VERSION***********
*/
#ifndef LCDHD44780
#define LCDHD44780
#include <unistd.h>
#include <bitset>
#include "sn7hc595_pico.h"
#include <iostream>
#include <string>
#define MILISECS 1000
#define BFLAG 18		//GPIO18
#define ENABLE 1
#define RW 2
#define RS 4

//SHIFT REGISTER to LCD MAPPING

//	SHIFTREG:			15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//	LCD			:			D7 D6 D5 D4 D3 D2 D1 D0 X  X  X  X  X  RS RW  E
//  LCD pin :																							4	5		6
//function set
#define FUNCTSET 1<<13
#define DATABUSLENGHT 1<<12
#define NLINES 1<<11
#define LED 25
#define ON 1
#define OFF 0
#define SLEEP3SECS sleep_ms(3000);
//display
#define DISPLAY 1<<11
#define CURSOR 1<<9
#define DON 1<<10
#define BLINKCURS 1<<8
#define CLEARSCREEN 1<<8
#define CURSORRETURN 1<<9
#define SETDDRAM 1<<15
#define VALID_ADDRESS	(addr <=0x27 || addr > 40 && addr <= 0x67)

//wrappers

#define setDDRAMLCD(x,y,z) x.setDDRAM(x.LCD_2x16[y][z])
#define putLCD(x,y) x.putWord(y)
#define clrLCD(x) x.clearScreen() 

//entrymode
#define ENTRYMODE 1<<10
#define ID 1<<9

#ifdef DEBUG
void log(const char *msg)
{
	std::cout << "Debug:" << msg << std::endl;	
	sleep(5);
}
#else
void log(const char *msg)
{

}
#endif

//#define DEBUG

struct HD44780_1602	: public SHIFT_16Bit 
{
	HD44780_1602():addrcount{0}
	{
		//std::cout << "Initialising the display." << std::endl;
		gpio_init(BFLAG);		//defaults to input.
		gpio_init(LED);
		gpio_set_dir(LED,GPIO_OUT);
		gpio_pull_down(BFLAG);
		blink(1);//blinks then turns on

		Data=FUNCTSET|DATABUSLENGHT|NLINES;
		hd44780(Data,"FUNCTSET|DATABUSLENGHT|NLINES");
		//std::cout << "function set: 8bit mpu connection,2 line display,5x8 font" <<std::endl;
		log("function set enabled.");
		
		Data=ENTRYMODE|ID;
		hd44780(Data,"ENTRYMODE|ID");
		//std::cout << "Entry mode :Increment cursor shifting disabled. " << std::endl;
		log("Entry mode enabled.");

		Data=DISPLAY|CURSOR|DON;
		hd44780(Data,"DISPLAY|CURSOR|BLINKCURS");
		//std::cout << "Display:Turn on display and cursor." << std::endl;
		log("Display enabled");

		//initialise LCD address space.	
		limit=39;
		for(row=0;row<=1;++row)
		{
			for(col=0;col<=limit;col++,addrcount++)
			looping:
				LCD_2x16[row][col]=addrcount;

			addrcount=0x40;
		}
		hd44780(CLEARSCREEN);
		hd44780(CURSORRETURN);
		//std::cout << "HD44780 Initialisation complete." << std::endl;
		stop:
		blink(0);	//LED blinks then turns off after initialization
	}
	char LCD_2x16[2][40];
	unsigned short Data;
	unsigned char addrcount;
	unsigned char row,col,limit;         
	bool setDDRAM(unsigned char addr)
	{
		if(!VALID_ADDRESS)
		{
		//	putWord("Invalid LCD address",addr);
			putWord("Invalid LCD address");
			return false;
		}
		unsigned short ddram=addr;
		ddram<<=8;
		ddram|=SETDDRAM;
		hd44780(ddram);
		sleep_us(1);
		addrcount=addr+1;
		return true;
	}
	template <typename T> 
	void putWord(const T &s)
	{
		if constexpr(std::is_same_v<const char *,T>)
		{
			std::string str=s;
			std::string::iterator itstr=str.begin();
			for(;itstr!=str.end();++itstr)
				putChar(*itstr);
		}		
		else
		if constexpr(std::is_same_v<float,T>)
		{
			std::string str=std::to_string(s);
			std::string::iterator itstr=str.begin();
			for(;itstr!=str.end();++itstr)
				putChar(*itstr);
		}
		else
		putWord("Invalid type");
	}
	void clearScreen()
	{
		hd44780(CLEARSCREEN);
	}
	void cursorReturn()
	{
		hd44780(CURSORRETURN);
	}
	//private:
	void putChar(const char c)
	{

		//std::cout << c<< std::endl;
		unsigned short character;
		character=c;
		character<<=8;
		character|=RS;
		hd44780(character,"DDRAM write");
		addrcount+=1;

	}
	// whenRS=0 and RW=1 BF(DB7) is set high until the HD44780U is available for next operation
	void busyFlag()
	{
		while(true)
		{

			shiftReg=RW;
			shiftOut(shiftReg);
			while(gpio_get(BFLAG))
			{
				//std::cout << "Busy flag set\n";
				sleep_ms(100);
			}
			shiftReg^=RW;
			shiftOut(shiftReg);
			//std::cout << "LCD busy flag clear.\n";
			break;
		}
	}

	// CHECK IF PICO IS TOO FAST FOR SHIFT REGISTER!!!
	template <typename ...T>
	void hd44780(unsigned short Data,T... t)
	{
		//std::cout <<"hd44780 binary instruction:" << std::bitset<16>(Data) << std::endl;
		//((std::cout << t << " "), ...);	//fold expression.
		busyFlag();
		shiftReg=Data;
		shiftOut(shiftReg);
		sleep_us(10);
		shiftReg|=ENABLE;
		shiftOut(shiftReg);
		sleep_us(10);
		shiftReg^=ENABLE;
		sleep_us(10);
		shiftOut(shiftReg);

	}
	void blink(unsigned char state)
	{
		unsigned char logic=state;
		for(int i=0;i<10;i++)
		{
			gpio_put(LED,state);
			sleep_ms(80);
			state^=1;
			gpio_put(LED,state);
			sleep_ms(10);
		}

	}
};
struct LCDField
{
	LCDField(HD44780_1602& lcd,char y,char x,uint8_t maxsize):y{y},x{x},max{maxsize},characters{},displ{lcd},head{}
	{}
	const char y;
	char x;
	uint8_t max;
	HD44780_1602& displ;
	void clearField()
	{
		if(displ.setDDRAM(displ.LCD_2x16[y][x]))
		{
			std::string::iterator itr=characters.begin();
			for(;itr!=characters.end();++itr)
				displ.putChar(' ');
		}	
	}
	void writeField(const char *word)
	{
		const char *error{"Word too big"};
		std::string newWord=word;
		displ.setDDRAM(displ.LCD_2x16[y][x]);
		if(newWord.length() > max)
			displ.putWord(error);
		else
		{
			displ.putWord(newWord.c_str());
			characters=newWord;

		}
	}
	void setHeader(const char *header)
	{
		const char *error{"Word too big"};
		head=header;
		displ.setDDRAM(displ.LCD_2x16[y][x]);
		if(head.length() > max)
		{
			displ.clearScreen();
			displ.cursorReturn();
			displ.putWord(error);
		}	
		else
		{
			displ.putWord(head.c_str());
			x+=head.length();
		}

	}
	private:
	std::string characters;
	std::string head;
};
#endif
