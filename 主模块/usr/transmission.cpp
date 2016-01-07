#include "transmission.h"

Transmission packaging;

Transmission::Transmission()
{
	for(int i=0;i<20;i++)
	{
		ModuleToUser[i]=0;
		ModuleToModule[i]=0;
	}
}

u8 *Transmission::C02_ModuleToUser(u8 data[20])
{
	data[1]=0xaa;
	return data;
}

//得到上位机命令
u8 Transmission::CommandParsing(u8 command[8])
{
	if(command[0]==0xff&&command[1]==0xdd)
	{
		u32 equipment;
		equipment=(u32)(command[2]<<24)+(u32)(command[3]<<16)+(u32)(command[4]<<8)+(u32)command[5];
		if(equipment==2) //看是否是该设备
		{
			u32 sum=0;
			for(u8 i=0;i<7;i++)
			  {
				sum+=command[i];
			  }
			  if(command[7]==(u8)sum) //和校验
			  {
				return command[6];
			  }
		}
	}
	return 0;
}
//命令封装
u8 *Transmission::CmmandTomodule(u8 state,u32 moduleNumber)
{
	command[0]=0xFF;
	command[1]=0xDD;
	command[2]=(u8)(moduleNumber>>24);
	command[3]=(u8)(moduleNumber>>16);
	command[4]=(u8)(moduleNumber>>8);
	command[5]=(u8)(moduleNumber);
	command[6]=0xaa;
	command[7]=0;
	for(u8 i=0;i<7;i++)
	{
		command[7]+=command[i];
	}
	return command;
}
