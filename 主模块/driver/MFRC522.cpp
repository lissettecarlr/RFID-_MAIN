
# include "MFRC522.h"

MFRC522::MFRC522(USART *usart):mUsart(usart)
{
}

void MFRC522::PCDInit()
{

	WriteRawRC(MFRC522_CommandReg,MFRC522_PCD_RESETPHASE);
	tskmgr.DelayMs(1);
	WriteRawRC(MFRC522_ModeReg,0x3D);   //定义发送和接收的模式，和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(MFRC522_TReloadRegL,30); //定时器重装值
    WriteRawRC(MFRC522_TReloadRegH,0);  //定时器重装值
    WriteRawRC(MFRC522_TModeReg,0x8D);  //定时器模式设置
    WriteRawRC(MFRC522_TPrescalerReg,0x3E);//预分频设置
    WriteRawRC(MFRC522_TxAskReg,0x40);     //传输调制设置：强制100％ASK调制独立的ModGsPReg的寄存器设置
	
	PcdAntennaOff();//关载波
	tskmgr.DelayMs(1);
	PcdAntennaOn();//开载波
	tskmgr.DelayMs(1);
}


bool MFRC522::WriteRawRC(unsigned char address, unsigned char value)
{
	double timeOut=tskmgr.Time();
	u8 reteunval;	
		mUsart->ClearReceiveBuffer();
		address = address&0x7F;//使用USART时使用
		mUsart->SendData(&address,1);
		while(mUsart->ReceiveBufferSize()<=0)
		{
			if(tskmgr.Time()-timeOut>0.010)//大于10ms判定是超时，时间可改
				return false;
		}
		mUsart->GetReceivedData(&reteunval,1);
		if(reteunval!=address)//返回的地址与发的不相同，错误
			return false;
		mUsart->SendData(&value,1);

	return true;
}

unsigned char MFRC522::ReadRawRC(unsigned char address)
{
	double timeOut=tskmgr.Time();
	unsigned char temp;
	
		mUsart->ClearReceiveBuffer();
		address = (address&0x7F)|0x80;
		mUsart->SendData(&address,1);
		while(mUsart->ReceiveBufferSize()<=0)
		{
			if(tskmgr.Time()-timeOut>0.010)//大于10ms判定是超时，时间可改
				break;
		}
		
		mUsart->GetReceivedData(&temp,1);
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
void MFRC522::PcdAntennaOn()
{
	unsigned char i;
    i = ReadRawRC(MFRC522_TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(MFRC522_TxControlReg, 0x03);
    }
}

void MFRC522::PcdAntennaOff()
{
	ClearBitMask(MFRC522_TxControlReg,0x03);//停止发送13.56MHz的载波
}


bool MFRC522::FindCard(unsigned char whichTag,unsigned char *pTagType)
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
  
   return status;
}

///////////////////
///防冲撞寻卡号
///@param pSnr 返回的卡片序列号 4字节
///@retval 是否成功
///////////////////
bool MFRC522::PcdAntiColl(unsigned char *pSnr)
{
	bool status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MFRC522_MaxReceiveLen]; 
    

    ClearBitMask(MFRC522_Status2Reg,0x08);
    WriteRawRC(MFRC522_BitFramingReg,0x00);
    ClearBitMask(MFRC522_CollReg,0x80);
 
    ucComMF522Buf[0] = MFRC522_PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = false;    }
    }
    SetBitMask(MFRC522_CollReg,0x80);
    return status;
}


///////////////////
///选择PICC
///@param pSnr IC卡的序列号 4个字节
///@retval 选择成功与否
///////////////////
bool MFRC522::PcdSelect(unsigned char *pSnr)
{
	char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MFRC522_MaxReceiveLen]; 
    
    ucComMF522Buf[0] = MFRC522_PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC16(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(MFRC522_Status2Reg,0x08);

    status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if (status && (unLen == 0x18))
    {   status = true;  }
    else
    {   status = false;    }

    return status;
}
////////////////////////
///CRC16校验计算
////////////////////////
void MFRC522::CalulateCRC16(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
	unsigned char i,n;
    ClearBitMask(MFRC522_DivIrqReg,0x04);
    WriteRawRC(MFRC522_CommandReg,MFRC522_PCD_IDLE);
    SetBitMask(MFRC522_FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
       WriteRawRC(MFRC522_FIFODataReg, *(pIndata+i));
	
    WriteRawRC(MFRC522_CommandReg, MFRC522_PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(MFRC522_DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(MFRC522_CRCResultRegL);
    pOutData[1] = ReadRawRC(MFRC522_CRCResultRegM);
}

////////////////////////
///验证卡片密码
///@param auth_mode 验证密码模式 取值：
///                                MFRC522_PICC_AUTHENT1A 验证A密匙
///                                MFRC522_PICC_AUTHENT1B 验证B密匙
///@param addr 块地址
///@param pKey 密码，6个字节
///@param pSnr 卡片序列号，4字节
////////////////////////
bool MFRC522::PcdAuthState(unsigned char auth_mode,unsigned char addr,
				  unsigned char *pKey,unsigned char *pSnr)
{
	char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MFRC522_MaxReceiveLen]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
 //   memcpy(&ucComMF522Buf[2], pKey, 6); 
 //   memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = PcdComPicc(MFRC522_PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if (!status || (!(ReadRawRC(MFRC522_Status2Reg) & 0x08)))
    {   status = false;   }
    
    return status;
}

//////////////////////////
///写数据到PICC的某块中
///@param addr 块地址
///@param pData 要写入的数据（16个字节）
///@retval 是否写成功
/////////////////////////
bool MFRC522::PcdWrite(unsigned char addr,unsigned char *pData)
{
	char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MFRC522_MaxReceiveLen]; 
    
    ucComMF522Buf[0] = MFRC522_PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC16(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if (!status || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = false;   }
        
    if (status)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    ucComMF522Buf[i] = *(pData+i);   }
        CalulateCRC16(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if (!status || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = false;   }
    }
    
    return status;
}

//////////////////////////
///读取PICC中的一块的数据
///@param addr 块地址
///@param pData 存入块中的数据16个字节
///@retval 存数据是否成功
//////////////////////////
bool MFRC522::PcdRead(unsigned char addr,unsigned char *pData)
{
	bool status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MFRC522_MaxReceiveLen]; 

    ucComMF522Buf[0] = MFRC522_PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC16(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComPicc(MFRC522_PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status) && (unLen == 0x90))
 //     memcpy(pData, ucComMF522Buf, 16);
    {
        for (i=0; i<16; i++)
            *(pData+i) = ucComMF522Buf[i];
    }
    else
       status = false;
    
    return status;
}

////////////////////////
///命令卡片进入睡眠状态
///@retval 睡眠是否成功
////////////////////////
bool MFRC522::CardHalt(void)
{
   /* bool status;*/
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MFRC522_MaxReceiveLen]; 

    ucComMF522Buf[0] = MFRC522_PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC16(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    /*status =*/ PcdComPicc(MFRC522_PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return true;
}
