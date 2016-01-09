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

//默认密码
const unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*ESP8266 指令*******************************************************************************************************/
u8 setmode[13]={0x41,0x54,0x2B,0x43,0x57,0x4D,0x4F,0x44,0x45,0x3D,0x32,0x0D,0x0A};//设置模式为2
//名字fzj  密码1234567890
u8 namepassword[33]={0x41,0x54,0x2B,0x43,0x57,0x53,0x41,0x50,0x3D,0x22,0x46,0x5A,0x4A,0x22,
					0x2C,0x22,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x22,0x2C,0x35,0x2C,0x33,0x0D,0x0A};
u8 Command1[13]={0x41,0x54,0x2B,0x43,0x49,0x50,0x4D,0x55,0x58,0x3D,0x31,0x0D,0x0A}; //设置多路连接
//设置IP地址为192.168.1.1
//u8 setIP[]={41 54 2B 43 49 50 41 50 3D 22 31 39 32 2E 31 36 38 2E 31 2E 31 22 0D 0A };
//开启服务器模式 端口 8080
u8 Command2[21]={0x41,0x54,0x2B,0x43,0x49,0x50,0x53,0x45,0x52,0x56,0x45,0x52,0x3D,0x31,0x2C,0x38,0x30,0x38,0x30,0x0D,0x0A};
//发送指令
u8 Command3[17]={0x41,0x54,0x2B,0x43,0x49,0x50,0x53,0x45,0x4E,0x44,0x3D,0x30,0x2C,0x32,0x30,0x0D,0x0A};
//取消回传
u8 CLOSS[6]={0x41,0x54,0x45,0x30,0x0D,0x0A};


u8 a[5]={0};

bool equipment_confirm(u8 *info);

//全局变量
u32 equipment[10];  //用于保存设备号

int main()
{
	u8 command[6];
	u8 datatemp[18];//数据暂存
	u8 tagInfo[18];
	u8 sensor[16];//保存传感器型号
	u8 basic_information[6];//基础信息
	u8 version[16];//版本
	
	tskmgr.DelayMs(1000);
	tskmgr.DelayMs(1000);
	
	
	//设置模式
	//设置名字密码
	//设置IP地址
	//复位
	WIFI.SendData(CLOSS,6);  //关闭回显
	tskmgr.DelayMs(1000);
	WIFI.SendData(Command1,13);  //开启多路连接
	tskmgr.DelayMs(1000);
	WIFI.SendData(Command2,21); //开启服务器模式
	tskmgr.DelayMs(1000);
	
	
	  equipment[0]=2;//测试用 将设备二加入
//	  DATA_COM.SendData(c,8);  //连续发送命令
	
	
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
			tskmgr.DelayMs(100); 
			  
			WIFI.ClearReceiveBuffer();//清除接收缓存
			WIFI.ClearSendBuffer();  //清除发送缓存
		  }
	  }
	  }
	 }
	
//从上位机得到命令()
		if(WIFI.ReceiveBufferSize()>40)
		{
			u8 ch=0;
			u8 temp_cmd;
			 u32 temp;
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
						
					   temp_cmd=command[4];  //得到命令位				
					   temp=command[0]+command[1]+command[2]+command[3];//得到设备号
						//命令处理
						if(temp_cmd==0XAA)//如果命令是注册
						{
							//读取RFID 将结果给上位机*********************************************
//						    WIFI.SendData(Command3,17);
//							tskmgr.DelayMs(100);
//							WIFI.ClearSendBuffer();
//							WIFI.SendData(a,5);
//							DATA_COM<<"11111";	
							double in_time;
							in_time=tskmgr.Time();
							rfid.PCDInit();
							while(1)
							{							
								tskmgr.DelayMs(500);
								if(tskmgr.Time()-in_time>=60)  //60秒读取等待
								{
									//返回结束
									break;
								}
								if(rfid.FindCard(MFRC522_PICC_REQALL,tagInfo))//寻到卡
								{
									if(rfid.PcdAntiColl(tagInfo))
									{																																
										if(rfid.PcdSelect(tagInfo))//选卡，卡号为前一步找到的卡号
										{																											
											if(rfid.PcdAuthState(MFRC522_PICC_AUTHENT1A,5,(unsigned char*)DefaultKey,tagInfo))//校验A密匙
											{
												rfid.PcdRead(5,basic_information);//第五块																		
												rfid.PcdRead(6,sensor);//第六块
													
											}
											
											if(rfid.PcdAuthState(MFRC522_PICC_AUTHENT1A,9,(unsigned char*)DefaultKey,tagInfo))//校验A密匙
											{
												rfid.PcdRead(9,version);//第9块
											}
								    		equipment_confirm(basic_information);//设备号存在判断，没有就存入
										    WIFI.SendData(Command3,17);
											tskmgr.DelayMs(100);
											WIFI.ClearSendBuffer();
											WIFI.SendData(packgroup.registered(basic_information,sensor,version),20);//打包发送	WIFI.SendData(a,6);
												
											tskmgr.DelayMs(500);//这里必须保证充足的延时才能发送下一半
											WIFI.SendData(Command3,17);
											tskmgr.DelayMs(100);
											WIFI.ClearSendBuffer();
											WIFI.SendData(packgroup.registered(basic_information,sensor,version)+20,20);//打包发送	WIFI.SendData(a,6);
											tskmgr.DelayMs(100);
											
										}
											
									}
									
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
						{
							DATA_COM.SendData(packgroup.CmmandTomodule(temp_cmd,temp),8); 	
						}

						
					}
				}
			}
			WIFI.ClearReceiveBuffer();
		}

	}
	
}


bool equipment_confirm(u8 *info)
{
	u32 moduleNO=(*info)+(*info+1)+(*info+2)+(*info+3);
	u8 flag;
	for(int i=0;i<10;i++)
	{
		if(equipment[i]==0) 
			flag=i;//记录那个下标下未存放
		if(moduleNO==equipment[i])
			return false;
	}
	equipment[flag]=moduleNO; //存放新设备号
	return true;
}






	
/*
主模块的功能：
接接收从模块的信息转发给上位机
读取RFID

*/

//u8 module[6]={0,0,0,0x02,0x03,0x00}; //模块信息
//u8 moduledata1[16]={0xB4,0xAB,0xB8,0xD0,0xC6,0xF7,0xC0,0xE0,0xD0,0xCD,0x4D,0x48,0x5A,0x31,0x34,0x20}; //MHZ14
//u8 moduledata2[16]={0xB9,0xCC,0xBC,0xFE,0xB0,0xE6,0xB1,0xBE,0xA3,0xBA,0x31,0x2E,0x30,0x30,0x20,0x20}; //固件版本
//u8 moduledata3[16]={0xB4,0xAB,0xB8,0xD0,0xC6,0xF7,0xC0,0xE0,0xD0,0xCD,0x44,0x48,0x32,0x32,0x20,0x20}; //HDH11
//														if(rfid.PcdWrite(9,(u8*)moduledata2))	

