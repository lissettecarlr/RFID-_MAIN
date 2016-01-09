#include "ADC.h"
#include "Timer.h"
#include "USART.h"
#include "Interrupt.h"
#include "TaskManager.h"
#include "transmission.h"
#include "MFRC522.h"



USART com(1,9600,true);   //USART1
USART DATA_COM(2,115200,false);
USART WIFI(3,115200,false);   //USART3
//ADC voltage(8);
Transmission packgroup;
MFRC522 rfid(&com);


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
u8 B[20]={0x99,0xAA,0x03,0x04,0x05,0x01,0x02,0x03,0x04,0x05,0x01,0x02,0x03,0x04,0x05,0x01,0x02,0x03,0x04,0x05};
u8 c[8]={0xFF,0xDD,0x00,0x00,0x00,0x02,0x03,0xE1};
int main()
{
	u8 command[6];
	u32 equipment[10];  //用于保存设备号
	u8 datatemp[18];//数据暂存
	u8 tagInfo[18];
	
	
	tskmgr.DelayMs(1000);
	tskmgr.DelayMs(1000);
	
	WIFI.SendData(CLOSS,6);  //关闭回显
	tskmgr.DelayMs(1000);
	WIFI.SendData(Command1,13);  //开启多路连接
	tskmgr.DelayMs(1000);
	WIFI.SendData(Command2,21); //开启服务器模式
	tskmgr.DelayMs(1000);
	
	
//    while(1)
//	{
//	   WIFI.SendData(Command3,17);
//	   tskmgr.DelayMs(1000);
//		WIFI.ClearSendBuffer();
//	    WIFI.SendData(B,20);
//		tskmgr.DelayMs(1000);
//		tskmgr.DelayMs(1000);
//		tskmgr.DelayMs(1000);
//	}
	
	  equipment[0]=2;//测试用 将设备二加入
	  DATA_COM.SendData(c,8);  //连续发送命令
	
	
    while(1)
	{
	  //接收从模块数据
	 if(DATA_COM.ReceiveBufferSize()>=40) //如果接收到了一帧数据
	 {
		 u32 check=10,moduleNo;
		 u8 sum=0;
		 u8 FFAA=0;
		 DATA_COM.GetReceivedData(&FFAA,1); //找包头
		 if(FFAA==0xff)
		{
		  DATA_COM.GetReceivedData(&FFAA,1); //找包头
		  if(FFAA==0xcc)
		{
		 DATA_COM.GetReceivedData(datatemp,18);
		 moduleNo=datatemp[0]+datatemp[1]+datatemp[2]+datatemp[3];
		 for(u8 i=0;i<10;i++) //查询是否有这个设备
		  {
			if(moduleNo==equipment[i])
				check=0;//有这个设备
		  }
		  for(u8 i=0;i<16;i++)
		  {
			sum+=datatemp[i];  //j校验和
		  }
		  sum=sum+0XCB;  //加上包头
		  if(sum==datatemp[17])
			  check=check+1;
		  
		  //使用WIFI发送数据给上位机
		  if(check==1)
		  {
		    WIFI.SendData(Command3,17);
	        tskmgr.DelayMs(100);
			WIFI.ClearSendBuffer();
			  
			WIFI.SendData(packgroup.MAINToUser(datatemp[5],datatemp[6],0xaa,moduleNo,datatemp[4]),20);
			  //有问题，在手机上显示的数据不对 怀疑是网络调试助手的错
//			  com.SendData(datatemp,20);
			  
//			  WIFI.SendData(B,20);
			tskmgr.DelayMs(100); 
			  
			WIFI.ClearReceiveBuffer();//清除接收缓存
			WIFI.ClearSendBuffer();  //清除发送缓存
		  }
	  }
	  }
	 }
	
//从上位机得到命令
		if(WIFI.ReceiveBufferSize()>50)
		{
			u8 ch=0;
		    
			//test
		
			while(WIFI.ReceiveBufferSize()>8)
			{
				WIFI.GetReceivedData(&ch,1);
				if(ch==0xff)//判断包头
				{
					WIFI.GetReceivedData(&ch,1);
					if(ch==0xdd)//判断包头
					{
					  //得到指令
					   WIFI.GetReceivedData(command,6);
					   WIFI.ClearReceiveBuffer();
						
					   u8 temp_cmd;
					   u32 temp;
					   temp_cmd=command[4];  //得到命令位				
					   temp=command[0]+command[1]+command[2]+command[3];//得到设备号
						//命令处理
						if(temp_cmd==0XAA)//如果命令是注册
						{
							//读取RFID 将结果给上位机***********************************
							DATA_COM<<"注册!!!!!\n";
							double in_time;
							in_time=tskmgr.Time();
							rfid.PCDReset();
							while(1)
							{
//								if(tskmgr.Time()-in_time>=20)  //20秒读取等待
//								{
//									//返回结束
//									break;
//								}
//								if(rfid.findCard(MFRC522_PICC_REQALL,tagInfo))//寻到卡
								if(rfid.PcdRequest(MFRC522_PICC_REQALL,tagInfo))//寻到卡
								{
										if(((u16)tagInfo[0]<<8|tagInfo[1])==MFRC522_PICC_MIFARE_ONE_S50)
										DATA_COM<<"\nMIFARE-M1-S50 card detected!\n";
//										if(rfid.PcdAntiColl(tagInfo))
//										{
//											DATA_COM<<"Card ID:\t";
//											for(int i=0;i<4;++i)
//												DATA_COM<<tagInfo[i]<<"\t";
//												DATA_COM<<"\n";
//										}
								}
								else
								{
									DATA_COM<<"find failed!\n";
								}
								
							}
						}
						else if(temp_cmd==0XDD)//如果命令是删除
						{
							for(int n=0;n<10;n++)
							{
								if(temp==equipment[n])
									equipment[n]=0;  //删除这个设备
							}
						}
						else
							DATA_COM.SendData(packgroup.CmmandTomodule(temp_cmd,temp),8); //将其发送给下位机

						
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
