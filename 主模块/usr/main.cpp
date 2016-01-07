#include "ADC.h"
#include "Timer.h"
#include "USART.h"
#include "Interrupt.h"
#include "TaskManager.h"
#include "transmission.h"




USART com(1,115200,false);   //USART1
USART DATA_COM(2,115200,false);
USART WIFI(3,115200,false);   //USART3
//ADC voltage(8);

u8 Command1[13]={0x41,0x54,0x2B,0x43,0x49,0x50,0x4D,0x55,0x58,0x3D,0x31,0x0D,0x0A}; //设置多路连接
//开启服务器模式 端口 8080
u8 Command2[21]={0x41,0x54,0x2B,0x43,0x49,0x50,0x53,0x45,0x52,0x56,0x45,0x52,0x3D,0x31,0x2C,0x38,0x30,0x38,0x30,0x0D,0x0A};
	
//发送指令
u8 Command3[17]={0x41,0x54,0x2B,0x43,0x49,0x50,0x53,0x45,0x4E,0x44,0x3D,0x30,0x2C,0x32,0x30,0x0D,0x0A};
//			   41 54 2B 43 49 50 53 45 4E 44 3D 31 2C 32 30 0D 0A 
//			   41 54 2B 43 49 50 53 45 4E 44 3D 32 2C 32 30 0D 0A 
//			   41 54 2B 43 49 50 53 45 4E 44 3D 33 2C 32 30 0D 0A 


u8 CLOSS[6]={0x41,0x54,0x45,0x30,0x0D,0x0A};
u8 A[4]={0X41,0X54,0X0D,0X0A};	
u8 B[20]={0x01,0x02,0x03,0x04,0x05,0x01,0x02,0x03,0x04,0x05,0x01,0x02,0x03,0x04,0x05,0x01,0x02,0x03,0x04,0x05};

int main()
{
	u8 command[8];
	u32 equipment[10];  //用于保存设备号
	u8 datatemp[20];//数据暂存
	
	tskmgr.DelayMs(1000);
	tskmgr.DelayMs(1000);
	
	WIFI.SendData(CLOSS,6);  //关闭回显
	tskmgr.DelayMs(1000);
	WIFI.SendData(Command1,13);  //开启多路连接
	tskmgr.DelayMs(1000);
	WIFI.SendData(Command2,21); //开启服务器模式
	tskmgr.DelayMs(1000);
	
	
    while(1)
	{
	   WIFI.SendData(Command3,17);
	   tskmgr.DelayMs(1000);
		WIFI.ClearSendBuffer();
	    WIFI.SendData(B,20);
		tskmgr.DelayMs(1000);
		tskmgr.DelayMs(1000);
		tskmgr.DelayMs(1000);
	}
	
	equipment[0]=2;//测试用 将设备二加入
	
    while(1)
	{
	  //接收从模块数据
	 if(DATA_COM.ReceiveBufferSize()>=20) //如果接收到了一帧数据
	 {
		 u32 check;
		 DATA_COM.GetReceivedData(datatemp,20);
		 check=datatemp[2]+datatemp[3]+datatemp[4]+datatemp[5];
		 for(u8 i=0;i<10;i++)
		  {
			if(check==equipment[i])
				check=0;//有这个设备
		  }
		  //使用WIFI发送数据给上位机
		  if(check==0)
		  com.SendData(datatemp,20);
		  WIFI.SendData(datatemp,20);
		  WIFI.ClearReceiveBuffer();
	 }
	
//     //数据上位机的命令
//	    if(com.ReceiveBufferSize()>=8) //想 
//		{
//			com.GetReceivedData(command,8); //接收命令
//			if(command[6]==0XAA)//如果命令是注册
//			{
//				//读取RFID 将结果给上位机
//			}
//			else if(command[6]==0XDD)//如果命令是删除
//			{
//				u32 temp;
//		        temp=command[2]+command[3]+command[4]+command[5];
//				for(int n=0;n<10;n++)
//				{
//					if(temp==equipment[n])
//						equipment[n]=0;  //删除这个设备
//				}
//			}
//			else
//				DATA_COM.SendData(command,8); //将其发送给下位机
//		}

		if(WIFI.ReceiveBufferSize()>50)
		{
			u8 ch=0;
		
			while(WIFI.ReceiveBufferSize()<8)
			{
				WIFI.GetReceivedData(&ch,1);
				if(ch==0xff)//判断包头
				{
					WIFI.GetReceivedData(&ch,1);
					if(ch==0xdd)//判断包头
					{
					  //得到指令
					}
					
				}
			}
			WIFI.ClearReceiveBuffer();
		}

	}
	
}
	
/*
主模块的功能：
接接收从模块的信息转发给上位机
读取RFID

*/
