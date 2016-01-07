#include "MHZ14.h"

//这里使用串口2作为驱动
USART CO2(2,9200,false); 


MHZ14::MHZ14()
{
	//一帧数据读取指令
	Command_getvalue[0]=0xff;
	Command_getvalue[1]=0x01;
	Command_getvalue[2]=0x86;
	Command_getvalue[3]=0;
	Command_getvalue[4]=0;
	Command_getvalue[5]=0;
	Command_getvalue[6]=0;
	Command_getvalue[7]=0;
	Command_getvalue[8]=0x79;
	
	DATA_H=0;
	DATA_L=0;
}


bool MHZ14::SumCheck(u8 data[9])
{
    u8 sum=0,check=0;	
	for(u8 i=1;i<8;i++)
	{
		sum+=data[i];
	}
     check=0xff-sum+0x01;
	if(data[8]==check)
		return true ;
	else
		return false;
}

u16 MHZ14::GetValue()
{
	return CO2_Concentration;
}


bool MHZ14::Updata()
{	
	//向模块发送获取数据
	   CO2.SendData(Command_getvalue,9);
	
	//判断是否有数据返回
	  if(CO2.ReceiveBufferSize()<9)
	  {
		  CO2.ClearReceiveBuffer(); //清空接收缓存
		  return false;
	  }
	  else
	  {
		  CO2.GetReceivedData(rev_buffer,9); //取出一帧数据
		   CO2.ClearReceiveBuffer(); //清空接收缓存
		  
		  if(SumCheck(rev_buffer)==false)  //校验和
			  return false;
		  else
		  {
			  DATA_H=rev_buffer[2];
			  DATA_L=rev_buffer[3];
			  CO2_Concentration=rev_buffer[2]*256+rev_buffer[3];  //计算浓度
			  return true;
		  }
	  }
}

