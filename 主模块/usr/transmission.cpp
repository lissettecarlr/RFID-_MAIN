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

u8 *Transmission::MAINToUser(u8 data_H,u8 data_L,u8 adc,u32 moduleNO,u8 type)
{
	//头
	ModuleToUser[0]=0xff;
	ModuleToUser[1]=0xaa;
	//设备号
	ModuleToUser[2]=moduleNO>>24;
	ModuleToUser[3]=moduleNO>>16;
	ModuleToUser[4]=moduleNO>>8;
	ModuleToUser[5]=moduleNO;
	//数据类型
	ModuleToUser[6]=type; //表示二氧化碳
 	//数据
	ModuleToUser[7]=data_H;
	ModuleToUser[8]=data_L;
	//电压值
	ModuleToUser[9]=adc;
	
	ModuleToUser[19]=0;
	for(int i=0;i<19;i++)
	{
		ModuleToUser[19]+=ModuleToUser[i];
	}
	return ModuleToUser;
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
	command[6]=state;
	command[7]=0;
	for(u8 i=0;i<7;i++)
	{
		command[7]+=command[i];
	}
	return command;
}

u8 *Transmission::registered(u8 *basic_information,unsigned char *str1,unsigned char *str2)
{
	u8 temp = 8;
	
	info[0]=0xff;
	info[1]=0x00;
	info[2]=*(basic_information+0);
	info[3]=*(basic_information+1);
	info[4]=*(basic_information+2);
	info[5]=*(basic_information+3);
	info[6]=*(basic_information+4);
	info[7]=*(basic_information+5);
	
	for(u8 i=10;i<16;i++)  
		info[temp++]=*(str1+i);  // 
	for(u8 i=10;i<16;i++)   
		info[temp++]=*(str2+i);
	
	return info;
	
	
}
