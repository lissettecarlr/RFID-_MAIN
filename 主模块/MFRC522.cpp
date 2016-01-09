#include "MFRC522.h"

MFRC522::MFRC522(USART *com):usart(com)
{
 //构造函数
}

// 写入数据	
bool MFRC522::write(u8 address,u8 data)
{
	u8 temp;
	usart->ClearReceiveBuffer(); 
	address = address&0x7F;//最高位为读写位
	usart->SendData(&address,1);
	tskmgr.DelayMs(1); //延时1ms
	
	if(usart->ReceiveBufferSize()>0)
	{
		usart->GetReceivedData(&temp,1);
		if(temp==address) //如果返回正确
		{
			usart->SendData(&data,1);
			return true;
		}
		else
			return false;
	}
	else
	{
		return false;
	}
	
}


//读出数据
u8 MFRC522::read(u8 address)
{
	u8 data;
	usart->ClearReceiveBuffer();
	address = (address&0x7F)|0x80;
	usart->SendData(&address,1);
	tskmgr.DelayMs(1);
	if(usart->ReceiveBufferSize()>0)
	{
		usart->GetReceivedData(&data,1);//得到数据
		return data;
	}
	else
		return 0;
}

void MFRC522::SetBit(u8 reg,u8 bit)
{
	u8 data=0;
	data=read(reg);
	write(reg,data|bit);
}

void MFRC522::ClearBit(u8 reg,u8 bit) 
{
	u8 data=0;
	data=read(reg);
	write(reg,data & ~bit);
}

void MFRC522::RST()//复位
{
	//软件复位
	write(MFRC522_CommandReg,MFRC522_PCD_RESETPHASE);
	//硬件复位
	
}



void MFRC522::StartAntenna()  //开启天线
{
	SetBit(MFRC522_TxControlReg, 0x03); //最低两位置1
}

void MFRC522::CloseAntenna()
{
	ClearBit(MFRC522_TxControlReg,0x03);
}

void MFRC522::init()
{
	RST();
	tskmgr.DelayUs(1);
	write(MFRC522_ModeReg,0x3D);   //定义发送和接收的模式，和Mifare卡通讯，CRC初始值0x6363
    write(MFRC522_TReloadRegL,30); //定时器重装值
    write(MFRC522_TReloadRegH,0);  //定时器重装值
    write(MFRC522_TModeReg,0x8D);  //定时器模式设置
    write(MFRC522_TPrescalerReg,0x3E);//预分频设置
    write(MFRC522_TxAskReg,0x40);     //传输调制设置：强制100％ASK调制独立的ModGsPReg的寄存器设置
	
	CloseAntenna();
	tskmgr.DelayMs(1);
	StartAntenna();
	tskmgr.DelayMs(1);
}


//bool MFRC522::PcdComPicc(u8 Command,u8 *pInData,u8 InLenByte,u8 *pOutData,u32  *pOutLenBit)
//{
//   u8 irqEn   = 0x00;
//   u8 waitFor = 0x00;
//   bool status = false;
//	
//   u32 i;
//	
//	    switch (Command)
//		{
//       case MFRC522_PCD_AUTHENT:  //如果执行读卡器的MIFARE标准认证
//          irqEn   = 0x12;
//          waitFor = 0x10;
//          break;
//       case MFRC522_PCD_TRANSCEIVE: //发送并接收数据（
//          irqEn   = 0x77;
//          waitFor = 0x30;
//          break;
//         default:
//         break;
//		}
//		
//	write(MFRC522_ComIEnReg,irqEn|0x80);//管脚IRQ上的信号与寄存器StatusReg的IRQ位的值相反
//    ClearBit(MFRC522_ComIrqReg,0x80);    //中断标志，定义CommIRqReg寄存器中的屏蔽位置位
//    write(MFRC522_CommandReg,MFRC522_PCD_IDLE);//设置为IDLE状态（模拟电路开启，MFR522唤醒，取消当前命令的执行，命令为0）
//    SetBit(MFRC522_FIFOLevelReg,0x80);          //清FIFO状态标志和错误标志，FIFO计数清零
//		
//	for (i=0; i<InLenByte; i++)
//		write(MFRC522_FIFODataReg, pInData[i]);
//		write(MFRC522_CommandReg, Command);
//   
//    
//    if (Command == MFRC522_PCD_TRANSCEIVE)
//       SetBit(MFRC522_BitFramingReg,0x80);//启动数据发送
//	
//	i = 0;//又作为超时标识位
//	double timeOut=tskmgr.Time();
//	u8 irq_flag;
//    do 
//    {
//		if(tskmgr.Time()-timeOut > 0.027)
//		{
//			i=1;//超时
//			break;
//		}
//		irq_flag = read(MFRC522_ComIrqReg);//获取中断标志寄存器
//		}while (!(irq_flag&0x01) && !(irq_flag&waitFor));//等待传输完成，并且定时器的值递减到零时退出
//       ClearBit(MFRC522_BitFramingReg,0x80);//清掉启动数据发送位
//		
//	if (i==0)//没有超时
//    {    
//		u8 temp=(read(MFRC522_ErrorReg));
//		if(!(temp&0x1B)) //未冲突未出错
//         {
//             status = true;
//             if (irq_flag & irqEn & 0x01) 
//                status = false;
//             if (Command == MFRC522_PCD_TRANSCEIVE)
//             {
//				u8 datalenth = read(MFRC522_FIFOLevelReg);//获取FIFO中数据的长度
//				u8 lastBits = read(MFRC522_ControlReg) & 0x07;//最后接收到的字节的有效位数，为零时整个字节有效

//				if (lastBits)
//					*pOutLenBit = (datalenth-1)*8 + lastBits;
//				else
//					*pOutLenBit = datalenth*8;

//				if (datalenth == 0)//队列中没有数据
//					datalenth = 1;

//				if (datalenth > MFRC522_MaxReceiveLen)//长度超过了最大长度
//					datalenth = MFRC522_MaxReceiveLen;

//				for (i=0; i<datalenth; i++)
//					pOutData[i] = read(MFRC522_FIFODataReg);
//            }
//         }
//         else//出错
//            status = false; 
//   }	
//	
//   SetBit(MFRC522_ControlReg,0x80);           // stop timer now
//   write(MFRC522_CommandReg,MFRC522_PCD_IDLE); //取消当前命令的执行
//   return status;
//}




bool MFRC522::findCard(u8 FindWay,u8 *CardType)
{
	bool status=false;unsigned int  unLen=0;      //存放接收到的数据的长度
	unsigned char ucComMF522Buf[MFRC522_MaxReceiveLen];  //存放接收到的数据,这里不直接使用pTagType是保险，防止用户传入长度太短的数组，导致数组越界程序崩溃

	ClearBit(MFRC522_Status2Reg,0x08);//清MIFAREe认证标志
	write(MFRC522_BitFramingReg,0x07);//面向位的帧调节，置位最后一个字节的位数，当此标志位0时，代表数据发送完毕
	SetBit(MFRC522_TxControlReg,0x03);//使能管脚TX1和TX2经载波后发送


	status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,&FindWay,1,ucComMF522Buf,&unLen);

	if ((status == true) && (unLen == 0x10))
	{    
	   *CardType     = ucComMF522Buf[0];
	   *(CardType+1) = ucComMF522Buf[1];
	}
	else
	{   status = false;   }

	return status;
}

bool MFRC522::FindCardID(unsigned char *str)
{
		bool status;
    unsigned char i,snr_check=0;
    unsigned int  dataLen;
    unsigned char OutDataBuf[MFRC522_MaxReceiveLen];  
    

    ClearBit(MFRC522_Status2Reg,0x08);
    write(MFRC522_BitFramingReg,0x00);
    ClearBit(MFRC522_CollReg,0x80);
 
    OutDataBuf[0] = MFRC522_PICC_ANTICOLL1;
    OutDataBuf[1] = 0x20;

    status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,OutDataBuf,2,OutDataBuf,&dataLen);

    if (status)
    {
    	 for (i=0; i<4; i++) //卡输出5个字节 4个字节卡号 1个字节校验
         {   
             *(str+i)  = OutDataBuf[i];
             snr_check ^= OutDataBuf[i]; //校验位
         }
         if (snr_check != OutDataBuf[i])
         {   status = false;    }
    }
    SetBit(MFRC522_CollReg,0x80);
    return status;
}





/****************************************************************************************/



bool MFRC522::WriteRawRC(unsigned char address, unsigned char value)
{
	double timeOut=tskmgr.Time();
	u8 reteunval;	

		usart->ClearReceiveBuffer();
		address = address&0x7F;//使用USART时使用
		usart->SendData(&address,1);
		while(usart->ReceiveBufferSize()<=0)
		{
			if(tskmgr.Time()-timeOut>0.010)//大于10ms判定是超时，时间可改
				return false;
		}
		usart->GetReceivedData(&reteunval,1);
		if(reteunval!=address)//返回的地址与发的不相同，错误
			return false;
		usart->SendData(&value,1);
	
	return true;
}

unsigned char MFRC522::ReadRawRC(unsigned char address)
{
	double timeOut=tskmgr.Time();
	unsigned char temp;
	
		usart->ClearReceiveBuffer();
		address = (address&0x7F)|0x80;
		usart->SendData(&address,1);
		while(usart->ReceiveBufferSize()<=0)
		{
			if(tskmgr.Time()-timeOut>0.010)//大于10ms判定是超时，时间可改
				break;
		}
		
		usart->GetReceivedData(&temp,1);



	return temp;
}

void MFRC522::ClearBitMask(unsigned char reg, unsigned char mask)
{
	char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}
void MFRC522::SetBitMask(unsigned char reg,unsigned char mask)
{
	char tmp = 0x00;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}



bool MFRC522::PcdRequest(unsigned char whichTag,unsigned char *pTagType)
{
	bool status=false;unsigned int  unLen=0;      //存放接收到的数据的长度
	unsigned char ucComMF522Buf[MFRC522_MaxReceiveLen];  //存放接收到的数据,这里不直接使用pTagType是保险，防止用户传入长度太短的数组，导致数组越界程序崩溃

	ClearBitMask(MFRC522_Status2Reg,0x08);//清MIFAREe认证标志
	WriteRawRC(MFRC522_BitFramingReg,0x07);//面向位的帧调节，置位最后一个字节的位数，当此标志位0时，代表数据发送完毕
	SetBitMask(MFRC522_TxControlReg,0x03);//使能管脚TX1和TX2经载波后发送


	status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,&whichTag,1,ucComMF522Buf,&unLen);

	if ((status == true) && (unLen == 0x10))
	{    
	   *pTagType     = ucComMF522Buf[0];
	   *(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   status = false;   }

	return status;
}


bool MFRC522::PcdComPicc(unsigned char Command,unsigned char *pDataToPicc,unsigned char toPiccLength,
	                                       unsigned char *pDataInPcd, unsigned int  *pInBitLength)
{
    bool status = false;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
	char temp;
    switch (Command)
    {
       case MFRC522_PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case MFRC522_PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    WriteRawRC(MFRC522_ComIEnReg,irqEn|0x80);//管脚IRQ上的信号与寄存器StatusReg的IRQ位的值相反
    ClearBitMask(MFRC522_ComIrqReg,0x80);    //中断标志，定义CommIRqReg寄存器中的屏蔽位置位
    WriteRawRC(MFRC522_CommandReg,MFRC522_PCD_IDLE);//设置为IDLE状态（模拟电路开启，MFR522唤醒，取消当前命令的执行，命令为0）
    SetBitMask(MFRC522_FIFOLevelReg,0x80);          //清FIFO状态标志和错误标志，FIFO计数清零
    
    for (i=0; i<toPiccLength; i++)
    	WriteRawRC(MFRC522_FIFODataReg, pDataToPicc[i]);
		WriteRawRC(MFRC522_CommandReg, Command);
   
    
    if (Command == MFRC522_PCD_TRANSCEIVE)
       SetBitMask(MFRC522_BitFramingReg,0x80);//启动数据发送
    
    i = 0;//操作M1卡最大等待时间25ms
	double timeOut=tskmgr.Time();
    do 
    {
		if(tskmgr.Time()-timeOut > 0.027)
		{
			i=1;//超时
			break;
		}
		n = ReadRawRC(MFRC522_ComIrqReg);//获取中断标志寄存器
		}while (!(n&0x01) && !(n&waitFor));//等待传输完成，并且定时器的值递减到零时退出
    ClearBitMask(MFRC522_BitFramingReg,0x80);//清掉启动数据发送位
	      
    if (i==0)//没有超时
    {    
		temp=(ReadRawRC(MFRC522_ErrorReg));
         if(!(temp&0x1B))
         {
             status = true;
             if (n & irqEn & 0x01)
                status = false;
             if (Command == MFRC522_PCD_TRANSCEIVE)
             {
				n = ReadRawRC(MFRC522_FIFOLevelReg);//获取FIFO中数据的长度
				lastBits = ReadRawRC(MFRC522_ControlReg) & 0x07;//最后接收到的字节的有效位数，为零时整个字节有效

				if (lastBits)
					*pInBitLength = (n-1)*8 + lastBits;
				else
					*pInBitLength = n*8;

				if (n == 0)//队列中没有数据
					n = 1;

				if (n > MFRC522_MaxReceiveLen)//长度超过了最大长度
					n = MFRC522_MaxReceiveLen;

				for (i=0; i<n; i++)
					pDataInPcd[i] = ReadRawRC(MFRC522_FIFODataReg);
            }
         }
         else//出错
            status = false;
   }
   

   SetBitMask(MFRC522_ControlReg,0x80);           // stop timer now
   WriteRawRC(MFRC522_CommandReg,MFRC522_PCD_IDLE); //取消当前命令的执行
   
   if(!status)
   {
	 if(temp&0x08 /*|| (n & irqEn & 0x01)*/)//检测到碰撞(或者超时)
	 {
		 char collReg = ReadRawRC(MFRC522_CollReg);//读出碰撞检测的寄存器中的值
		 collReg&=0x1F;//获取发生碰撞的位置
		
	 }	   
   }
   

   return status;
}


