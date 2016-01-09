#include "stm32f10x.h"
#include "USART.h"
#include "TaskManager.h"

//串口默认为9600


class MFRC522{
	private:
			
		bool write(u8 address,u8 data);
		u8 read(u8 address);
		USART *usart;

		///PCD和PICC通信
//		bool PcdComPicc(u8 Command,u8 *pInData,u8 InLenByte,u8 *pOutData, u32  *pOutLenBit);


	public:
		MFRC522(USART *com); //选择工作串口
	
	 	void SetBit(u8 reg,u8 bit);
		void ClearBit(u8 reg,u8 bit);
	
		void RST(); //复位
		void StartAntenna();  //开启天线
		void CloseAntenna();//关闭天线
		void init();
	
	  //方式  MFRC522_PICC_REQIDL 寻天线区内未进入休眠状态  MFRC522_PICC_REQALL 寻天线区内全部卡
		bool findCard(u8 FindWay,u8 *CardType); 
	
	  //寻卡号
	  bool FindCardID(unsigned char *str);
	
	
	
	/*******************************************/
	bool WriteRawRC(unsigned char address, unsigned char value);
	unsigned char ReadRawRC(unsigned char address);
	void ClearBitMask(unsigned char reg, unsigned char mask);
	void SetBitMask(unsigned char reg,unsigned char mask);
	bool PcdComPicc(unsigned char Command,unsigned char *pDataToPicc,unsigned char toPiccLength,
	                                       unsigned char *pDataInPcd, unsigned int  *pInBitLength);
	bool PcdRequest(unsigned char whichTag,unsigned char *pTagType);
};


#define MFRC522_MaxReceiveLen 18   //用来当缓冲区和临时变量使用的

/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define MFRC522_PCD_IDLE              0x00               //取消当前命令
#define MFRC522_PCD_CALCCRC           0x03               //激活CRC协处理器或执行自测试
#define MFRC522_PCD_TRANSMIT          0x04               //发送FIFO缓冲区的命令
#define MFRC522_PCD_NOCMDCHANGE       0x07               //无命令改变，用来修改命令寄存器的不同位，但有不触及其它命令，如掉电
#define MFRC522_PCD_RECEIVE           0x08               //接收数据（激活接收电路）
#define MFRC522_PCD_TRANSCEIVE        0x0C               //发送并接收数据（如果ControlReg的Initiator位被置为1，将FIFO缓冲区数据发送到天线并在完成后自动激活接收器电路，
                                                         //                                      Initiator为0, 则激活接收天线）
#define MFRC522_PCD_AUTHENT           0x0E               //执行读卡器的MIFARE标准认证
#define MFRC522_PCD_RESETPHASE        0x0F               //复位MFRC522

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define MFRC522_PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define MFRC522_PICC_REQALL           0x52               //寻天线区内全部卡
#define MFRC522_PICC_ANTICOLL1        0x93               //防冲撞 代表 UID CL1（对于ID字节数大于等于1个字节的）
#define MFRC522_PICC_ANTICOLL2        0x95               //防冲撞 代表 UID CL2（对于ID字节数大于4个字节的）
#define MFRC522_PICC_ANTICOLL3        0x97               //防冲撞 代表 UID CL3（对于ID字节数大于6个字节的）
#define MFRC522_PICC_AUTHENT1A        0x60               //验证A密钥
#define MFRC522_PICC_AUTHENT1B        0x61               //验证B密钥
#define MFRC522_PICC_READ             0x30               //读块
#define MFRC522_PICC_WRITE            0xA0               //写块
#define MFRC522_PICC_DECREMENT        0xC0               //扣款（减法）
#define MFRC522_PICC_INCREMENT        0xC1               //充值（加法）
#define MFRC522_PICC_RESTORE          0xC2               //调块数据到缓冲区
#define MFRC522_PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define MFRC522_PICC_HALT             0x50               //休眠

//////////////////////////////////////////////
///Mifare_One卡类型枚举
//////////////////////////////////////////////
#define MFRC522_PICC_MIFARE_ULTRALIGHT       0x4400
#define MFRC522_PICC_MIFARE_ONE_S50          0x0400
#define MFRC522_PICC_MIFARE_ONE_S70          0x0200
#define MFRC522_PICC_MIFARE_PRO_X            0x0800
#define MFRC522_PICC_MIFARE_DESFIRE          0x4403


/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义  RFU寄存器保留，不能更改其中的值，RFT保留用于产品测试，不能更改其中的值
/////////////////////////////////////////////////////////////////////
// PAGE 0 命令和状态
#define     MFRC522_RFU00                 0x00    
#define     MFRC522_CommandReg            0x01    
#define     MFRC522_ComIEnReg             0x02    
#define     MFRC522_DivlEnReg             0x03    
#define     MFRC522_ComIrqReg             0x04    
#define     MFRC522_DivIrqReg             0x05
#define     MFRC522_ErrorReg              0x06    
#define     MFRC522_Status1Reg            0x07    
#define     MFRC522_Status2Reg            0x08    
#define     MFRC522_FIFODataReg           0x09
#define     MFRC522_FIFOLevelReg          0x0A
#define     MFRC522_WaterLevelReg         0x0B
#define     MFRC522_ControlReg            0x0C
#define     MFRC522_BitFramingReg         0x0D
#define     MFRC522_CollReg               0x0E
#define     MFRC522_RFU0F                 0x0F
// PAGE 1     命令
#define     MFRC522_RFU10                 0x10
#define     MFRC522_ModeReg               0x11
#define     MFRC522_TxModeReg             0x12
#define     MFRC522_RxModeReg             0x13
#define     MFRC522_TxControlReg          0x14
#define     MFRC522_TxAskReg              0x15
#define     MFRC522_TxSelReg              0x16
#define     MFRC522_RxSelReg              0x17
#define     MFRC522_RxThresholdReg        0x18
#define     MFRC522_DemodReg              0x19
#define     MFRC522_RFU1A                 0x1A
#define     MFRC522_RFU1B                 0x1B
#define     MFRC522_MifareReg             0x1C
#define     MFRC522_RFU1D                 0x1D
#define     MFRC522_RFU1E                 0x1E
#define     MFRC522_SerialSpeedReg        0x1F
// PAGE 2     CFG（设置）
#define     MFRC522_RFU20                 0x20  
#define     MFRC522_CRCResultRegM         0x21
#define     MFRC522_CRCResultRegL         0x22
#define     MFRC522_RFU23                 0x23
#define     MFRC522_ModWidthReg           0x24
#define     MFRC522_RFU25                 0x25
#define     MFRC522_RFCfgReg              0x26
#define     MFRC522_GsNReg                0x27
#define     MFRC522_CWGsCfgReg            0x28
#define     MFRC522_ModGsCfgReg           0x29
#define     MFRC522_TModeReg              0x2A
#define     MFRC522_TPrescalerReg         0x2B
#define     MFRC522_TReloadRegH           0x2C
#define     MFRC522_TReloadRegL           0x2D
#define     MFRC522_TCounterValueRegH     0x2E
#define     MFRC522_TCounterValueRegL     0x2F
// PAGE 3      测试
#define     MFRC522_RFU30                 0x30
#define     MFRC522_TestSel1Reg           0x31
#define     MFRC522_TestSel2Reg           0x32
#define     MFRC522_TestPinEnReg          0x33
#define     MFRC522_TestPinValueReg       0x34
#define     MFRC522_TestBusReg            0x35
#define     MFRC522_AutoTestReg           0x36
#define     MFRC522_VersionReg            0x37
#define     MFRC522_AnalogTestReg         0x38
#define     MFRC522_TestDAC1Reg           0x39  
#define     MFRC522_TestDAC2Reg           0x3A   
#define     MFRC522_TestADCReg            0x3B   
#define     MFRC522_RFU3C                 0x3C   
#define     MFRC522_RFU3D                 0x3D   
#define     MFRC522_RFU3E                 0x3E   
#define     MFRC522_RFU3F		          0x3F

