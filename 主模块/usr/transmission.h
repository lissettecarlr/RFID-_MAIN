#ifndef __TRANSMISSION_H
#define __TRANSMISSION_H

#include "stm32f10x.h"

class Transmission{
	
	private:
		u8 ModuleToModule[20];
		u8 ModuleToUser[20];
		u8 command[8];
	public:
		Transmission();
		u8 *C02_ModuleToUser(u8 data[20]);
	
		u8 CommandParsing(u8 command[8]);
	
		u8 *CmmandTomodule(u8 state,u32 moduleNumber);
	
};

extern Transmission packaging;

#endif


/*

*/
