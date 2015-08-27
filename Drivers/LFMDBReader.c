/****************************************Copyright (c)*************************************************
**                      Fujian Junpeng Communicaiton Technology Co.,Ltd.
**                               http://www.easivend.com.cn
**--------------File Info------------------------------------------------------------------------------
** File name:           LFMDBReader
** Last modified Date:  2013-03-04
** Last Version:         
** Descriptions:        �ʷ�MDBˢ��������������غ�������                     
**------------------------------------------------------------------------------------------------------
** Created by:          gzz
** Created date:        2013-03-04
** Version:             V2.0
** Descriptions:        The original version
**------------------------------------------------------------------------------------------------------
** Modified by:         
** Modified date:       
** Version:             
** Descriptions:        
********************************************************************************************************/
#include "..\config.h"
#include "LFMDBReader.h"

//����ֽ����ͨ��Ӧ�����ݰ���ʱ����
//extern unsigned int MDBCONVERSATIONWAITACKTIMEOUT;
extern void LFReaderUpdateConfigData(uint8_t * ReaderRdBuff);
extern void LFReaderSessionComplete(void);
extern void LFReaderCashSale(uint16_t ItemPrice);

/*********************************************************************************************************
** Function name:       LFReaderDevProcess
** Descriptions:        �������ֵ
** input parameters:    
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderUpdateRecMsg(uint8_t * ReaderRdBuff)
{

	switch(ReaderRdBuff[0])
	{
		case RTV_JUST_RESET://3���ڸ�λ״̬		
			TraceReader("\r\nDrvReader2JustReset...");
			//do nothing ???
		break;
		
		case RTV_READER_CONFIG_INFO:
			TraceReader("\r\nDrvReader4ReaderUpdateConfigData...");		   			
		break;
		
		case RTV_DISPLAY_REQUEST:
			TraceReader("\r\nDrvReaderDisplayRequest...");
			//for future use...
		break;
		
		case RTV_BEGIN_SESSION://7.poll���忨��Ϣ�����뽻��session			
			TraceReader("\r\nDrvReader7BeginSession...");
			TraceReader("\r\nDrvReader 2.=%d,%d\r\n",ReaderRdBuff[1],ReaderRdBuff[2]);
			//Reader_BeginSession(buf,len);//�õ����ڽ��		
			//ReaderPara.cReaderSta = READER_STA_SESSION_IDLE;
			//Reader_PostToVmc(buf[0]);//display balance	����������䷢�͸�vmc�������̣���ʼ���н���				
		break;
		
		case RTV_SESSION_CANCEL_REQUEST://7���һ�ο���ȡ������session
			TraceReader("\r\nDrvReader7SessionCancelRequest...");
			//ReaderPara.cSessionCancel = 1;//��1������vmcȡ������session
		break;
		
		case RTV_VEND_APPROVED://8�����������㹻��������������
			TraceReader("\r\nDrvReader9VendApproved...");
			//Reader_VendApproved(buf, len);
			//Reader_PostToVmc(buf[0]);//allow dispense����vmc���Գ���
		break;
		
		case RTV_VEND_DENIED:
			TraceReader("\r\nDrvReaderVendDenied...");
			//Reader_PostToVmc(buf[0]);//belance not enough
		break;
		
		case RTV_END_SESSION://11������session,���ؿ���ʹ��״̬	
			TraceReader("\r\nDrvReader7.2EndSession...");
			//ReaderPara.cReaderSta = READER_STA_ENABLED;//����ʹ��״̬
			//Reader_PostToVmc(buf[0]);//display goodbye
		break;

		case RTV_CANCELLED:
			TraceReader("\r\nDrvReaderCancelled...");
			//reader will reset
		break;
		
		case RTV_PERIPHERAL_ID:
			TraceReader("\r\nDrvReaderPeripheralID...");
		break;
		
		case RTV_MALFUNCTION_ERROR:
			TraceReader("\r\nDrvReaderMalfunctionError...");
			//Reader_MalfunctionError(buf, len);
			//Reader_PostToVmc(buf[0]);//reader error
		break;
		
		case RTV_CMD_OUT_OF_SEQUENCE:
			TraceReader("\r\nDrvReaderCmdOutOfSequence");
			//Reader_PostToVmc(buf[0]);
		break;

		//level 1+...		
		
		//case 0x0D://Revalue Approved
		//break;
		//case 0x0E://Revalue Denied
		//break;
		//case 0x0F://Revalue Limit Amount
		//break;
		//case 0x10://User File Data
		//break;
		//case 0x11://Time/Data Request
		//break;
		//case 0x12://Data Entry Request
		//break;	
		
		default://...
			TraceReader("\r\nDrvReaderLevel 1+ ... buf[0] = %02x", ReaderRdBuff[0]);
		break;
	}
}

/*********************************************************************************************************
** Function name:       LFReaderDevInit
** Descriptions:        ˢ�����豸��ʼ��
** input parameters:    ��
** output parameters:   ��
** Returned value:      1ʹ��ʧ��,0ʹ�ܳɹ�
*********************************************************************************************************/
uint8_t LFReaderDevInit()
{
	unsigned char ReaderRdBuff[36],ReaderRdLen,ComStatus,VMCValue[11]={0};	
	unsigned char j,i;
	//uint32_t Bvalue = 0;
	//uint8_t ReaderType = 0;
	uint8_t retvalue = 0;
			
	//1Reset
	for(ComStatus = 0, j = 0; j < 30 && ComStatus != 1; j++)
	{
		ComStatus = MdbConversation(0x10,NULL,0x00,&ReaderRdBuff[0],&ReaderRdLen);
	}
	OSTimeDly(OS_TICKS_PER_SEC/100);	
	TraceReader("\r\n1DrvReader>>%#02x",0x10);
	LCDNumberFontPrintf(40,LINE15,2,"READER-1");
	//poll
	//LFReaderDevProcess(&Bvalue,0,0,&ReaderType);
	//poll,����ͨ����һ��ָ���ٵõ���һ�������ݻظ�	
	{
		ComStatus = MdbConversation(0x12,NULL,0x00,&ReaderRdBuff[0],&ReaderRdLen);
	}
	if(ComStatus == 1)
	{	
		TraceReader("\r\nDrvReaderPollRec<< [%02d]-",ReaderRdLen);
		for(i=0;i<ReaderRdLen;i++)
		{
			TraceReader(" %#02x ",ReaderRdBuff[i]);
		}
		TraceReader("\r\n");
	}
	OSTimeDly(OS_TICKS_PER_SEC/100);
	//OSTimeDly(OS_TICKS_PER_SEC/100);	
	//2Setup		
	VMCValue[0] = 0x00;//Configuration data
	VMCValue[1] = 0x01;//VMC Feature Level
	VMCValue[2] = 0x00;//Columns on Display
	VMCValue[3] = 0x00;//Rows on Display
	VMCValue[4] = 0x00;//Display Information	
	for(ComStatus = 0, j = 0; j < 10 && ComStatus != 1; j++)
	{
		ComStatus = MdbConversation(0x11,VMCValue,5,&ReaderRdBuff[0],&ReaderRdLen);
	}
	TraceReader("\r\n 2DrvReader>>%#02x,%#02x,%#02x,%#02x,%#02x,%#02x",0x11,VMCValue[0],VMCValue[1],VMCValue[2],VMCValue[3],VMCValue[4]);
	if(ComStatus == 1)
	{
		/*
		//Trace("\r\n 2");
		stDevValue.ReaderLevel = ReaderRdBuff[1];
		stDevValue.ReaderDecimal = 100;
		for(i = 0; i < ReaderRdBuff[5]; i++) 
		{
		   stDevValue.ReaderDecimal /= 10;
		}
		stDevValue.ReaderScale = ReaderRdBuff[4] * stDevValue.ReaderDecimal;
		stDevValue.ReaderRecTime = ReaderRdBuff[6];
		stDevValue.ReaderMiscelOpt = ReaderRdBuff[7];
		
		Trace("\r\nlen = %d",ReaderRdLen);
		for(p=0;p<ReaderRdLen;p++)
		{
			Trace(" %02x",ReaderRdBuff[p]);
		}
		*/
		TraceReader("\r\nDrvReaderRec<< [%02d]-",ReaderRdLen);
		for(i=0;i<ReaderRdLen;i++)
		{
			TraceReader(" %#02x ",ReaderRdBuff[i]);
		}
		TraceReader("\r\n");	
		LCDNumberFontPrintf(40,LINE15,2,"READER-2");
		//LFReaderDevProcess(&Bvalue,0,0,&ReaderType);//poll,����ͨ����һ��ָ���ٵõ���һ�������ݻظ�	
		LFReaderUpdateConfigData(ReaderRdBuff);//3����setup����Ϣ������level,�������ӵ�
	}
	
	//3
	//LFReaderDevProcess(&Bvalue,&ReaderType);
	//3setupprice���������С���
	VMCValue[0] = 0x01;//Max / Min prices
	VMCValue[1] = 0xff;//Maximum Price �C scaled
	VMCValue[2] = 0xff;
	VMCValue[3] = 0xff;
	VMCValue[4] = 0xff;
	VMCValue[5] = 0x00;//Minimum Price �C scaled
	VMCValue[6] = 0x00; 
	VMCValue[7] = 0x00; 
	VMCValue[8] = 0x00; 
	VMCValue[9] = 0x11;
	VMCValue[10] = 0x56;
	ComStatus = MdbConversation(0x11,VMCValue,11,&ReaderRdBuff[0],&ReaderRdLen);
	TraceReader("\r\n 3DrvReader>>%#02x,%#02x,%#02x,%#02x,%#02x,%#02x",0x11,VMCValue[0],VMCValue[1],VMCValue[2],VMCValue[3],VMCValue[4]);
	TraceReader("\r\n3DrvReaderSetupMax/Min Prices");
	//4
	//LFReaderDevProcess(&Bvalue,0,0,&ReaderType);//poll,����ͨ����һ��ָ���ٵõ���һ�������ݻظ�		
	//TraceReader("\r\nDrvReader 5.RTV SETUPPRICE");
	TraceReader("\r\nDrvReaderSetup=%d,%d,%ld,%d,%d,%d\r\n",stDevValue.ReaderLevel,stDevValue.ReaderScale,stDevValue.ReaderDecimal,stDevValue.ReaderRecTime,stDevValue.ReaderMiscelOpt,stDevValue.ReaderCashSale);
	//4ʹ��
	LFReaderDevEnable();
	LCDNumberFontPrintf(40,LINE15,2,"READER-3");
	//OSTimeDly(40);
	
	//5
	//LFReaderDevProcess(&Bvalue,0,0,&ReaderType);
	//poll,����ͨ����һ��ָ���ٵõ���һ�������ݻظ�	
	{
		ComStatus = MdbConversation(0x12,NULL,0x00,&ReaderRdBuff[0],&ReaderRdLen);
	}
	if(ComStatus == 1)
	{	
		TraceReader("\r\nDrvReaderPollRec<< [%02d]-",ReaderRdLen);
		for(i=0;i<ReaderRdLen;i++)
		{
			TraceReader(" %#02x ",ReaderRdBuff[i]);
		}
		TraceReader("\r\n");
	}
	OSTimeDly(OS_TICKS_PER_SEC/100);
	retvalue = (ComStatus == 1)?0:1;
	return retvalue;
	/*//Trace("4.2=%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\r\n",nReadervalue[0],nReadervalue[1],nReadervalue[2],nReadervalue[3],nReadervalue[4],nReadervalue[5],nReadervalue[6],nReadervalue[7]);
	//OSTimeDly(OS_TICKS_PER_SEC*3);
	//LFReaderDevEnable();
	
	*/	
}



/*********************************************************************************************************
** Function name:       LFReaderDevEnable
** Descriptions:        ʹ��ˢ����
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderDevEnable(void)
{
	unsigned char ReaderRdBuff[36],ReaderRdLen;
	uint8_t Enable[1] = {0x01};
	uint8_t comStatus,i;
	//
	for(comStatus = 0, i = 0; i < 10 && comStatus != 1; i++)
	{
		comStatus = MdbConversation(0x14,Enable,1,&ReaderRdBuff[0],&ReaderRdLen);
	}	
	TraceReader("\r\n 4DrvReaderEnable>>%#02x,%#02x",0x14,Enable[0]);
	OSTimeDly(OS_TICKS_PER_SEC/100);
}

/*********************************************************************************************************
** Function name:       LFReaderDevDisable
** Descriptions:        ����ˢ����
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderDevDisable(void)
{
	unsigned char ReaderRdBuff[36],ReaderRdLen;
	uint8_t Enable[1] = {0x00};
	uint8_t comStatus,i;
	
	for(comStatus = 0, i = 0; i < 10 && comStatus != 1; i++)
	{
		comStatus = MdbConversation(0x14,Enable,1,&ReaderRdBuff[0],&ReaderRdLen);
	}
	TraceReader("\r\n 4DrvReaderDisable>>%#02x,%#02x",0x14,Enable[0]);
	OSTimeDly(OS_TICKS_PER_SEC/100);
}

/*********************************************************************************************************
** Function name:       LFReaderSessionVendRequest
** Descriptions:        ���׿ۿ�����
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderSessionVendRequest(uint16_t ItemPrice)
{	
	uint8_t Request[7],comStatus,i;
	unsigned char ReaderRdBuff[36],ReaderRdLen;
	
	Request[0] = 0x00;//VEND REQUEST
	Request[1] = 0x00;
	Request[2] = 0x00;
	Request[3] = HUINT16(ItemPrice);//Item Price - scaled���׳�������
	Request[4] = LUINT16(ItemPrice);
	Request[5] = 0x00;;//Item Number���׳�������
	Request[6] = 0x01;
	for(comStatus = 0, i = 0; i < 20 && comStatus != 1; i++)
	{
		comStatus = MdbConversation(0x13,Request,7,&ReaderRdBuff[0],&ReaderRdLen);
	}
	TraceReader("\r\n8DrvReadercostRequest>>%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x",0x13,Request[0],Request[1],Request[2],Request[3],Request[4],Request[5],Request[6]);
	
	//LFReaderUpdateRecMsg(ReaderRdBuff);
	//OSTimeDly(OS_TICKS_PER_SEC / 2);
}

/*********************************************************************************************************
** Function name:       LFReaderSessionVendSuccess
** Descriptions:        ���׳ɹ�
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderSessionVendSuccess()
{	
	uint8_t Success[3];
	unsigned char ReaderRdBuff[36],ReaderRdLen;
	uint8_t comStatus,i;
	
	Success[0] = 0x02;//VEND SUCCESS 	
	Success[1] = 0x00;;//Item Number���׳�������
	Success[2] = 0x01;
	for(comStatus = 0, i = 0; i < 20 && comStatus != 1; i++)
	{
		comStatus = MdbConversation(0x13,Success,3,&ReaderRdBuff[0],&ReaderRdLen);
	}
	TraceReader("\r\n10DrvReaderSUCC>>%#02x,%#02x,%#02x,%#02x",0x13,Success[0],Success[1],Success[2]);
	LFReaderSessionComplete();
}

/*********************************************************************************************************
** Function name:       LFReaderSessionVendFail
** Descriptions:        ����ʧ��
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderSessionVendFail()
{	
	uint8_t Failure[1] = {0x03};//VEND FAILURE
	unsigned char ReaderRdBuff[36],ReaderRdLen;
	uint8_t comStatus,i;
	
	for(comStatus = 0, i = 0; i < 20 && comStatus != 1; i++)
	{
		comStatus = MdbConversation(0x13,Failure,1,&ReaderRdBuff[0],&ReaderRdLen);
	}
	TraceReader("\r\n10DrvReaderFail>>%#02x,%#02x",0x13,Failure[0]);
	
}



/*********************************************************************************************************
** Function name:       LFReaderSessionComplete
** Descriptions:        ���ν��׽���
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderSessionComplete(void)
{	
	uint8_t SessionComplete[1] = {0x04};//SESSION COMPLETE
	unsigned char ReaderRdBuff[36],ReaderRdLen;
	uint8_t comStatus,i;
	//unsigned char p;
	
	for(comStatus = 0, i = 0; i < 20 && comStatus != 1; i++)
	{
		comStatus = MdbConversation(0x13,SessionComplete,1,&ReaderRdBuff[0],&ReaderRdLen);
	}
	TraceReader("\r\nDrvReader7.1complete>>%#02x,%#02x",0x13,SessionComplete[0]);
	//OSTimeDly(OS_TICKS_PER_SEC/100);		
	TraceReader("\r\nDrvReaderRec<< [%02d]-",ReaderRdLen);
	for(i=0;i<ReaderRdLen;i++)
	{
		TraceReader(" %#02x ",ReaderRdBuff[i]);
	}
	TraceReader("\r\n");	
	
	//LFReaderUpdateRecMsg(ReaderRdBuff);
}

/*********************************************************************************************************
** Function name:       LFReaderCashSale
** Descriptions:        �ֽ���ϸ���׼�¼���ύ��������
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderCashSale(uint16_t ItemPrice)
{	
	uint8_t Request[5],comStatus,i;
	unsigned char ReaderRdBuff[36],ReaderRdLen;
	
	Request[0] = 0x05;//Cash Sale
	Request[1] = HUINT16(ItemPrice);//Item Price - scaled���׳�������
	Request[2] = LUINT16(ItemPrice);
	Request[3] = 0x00;;//Item Number���׳�������
	Request[4] = 0x01;
	for(comStatus = 0, i = 0; i < 20 && comStatus != 1; i++)
	{
		comStatus = MdbConversation(0x13,Request,5,&ReaderRdBuff[0],&ReaderRdLen);
	}
	TraceReader("\r\n8DrvReaderCashSale>>%#02x,%#02x,%#02x,%#02x,%#02x,%#02x",0x13,Request[0],Request[1],Request[2],Request[3],Request[4]);
	
	//LFReaderUpdateRecMsg(ReaderRdBuff);
	//OSTimeDly(OS_TICKS_PER_SEC / 2);
}


/*********************************************************************************************************
** Function name:       LFReaderUpdateConfigData
** Descriptions:        ����ʱ��������
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderUpdateConfigData(uint8_t * ReaderRdBuff)
{
	uint8_t i;	

	stDevValue.ReaderLevel = ReaderRdBuff[1];
	stDevValue.ReaderDecimal = 100;
    for(i = 0; i < ReaderRdBuff[5]; i++) 
    {
	   stDevValue.ReaderDecimal /= 10;
    }
	stDevValue.ReaderScale = ReaderRdBuff[4] * stDevValue.ReaderDecimal;
	stDevValue.ReaderRecTime = ReaderRdBuff[6];
	stDevValue.ReaderMiscelOpt = ReaderRdBuff[7];
	if((uint8_t)(stDevValue.ReaderMiscelOpt>>3)&0x01)//�����ֽ���ϸ���׼�¼���ύ��������
	{
		stDevValue.ReaderCashSale = 1;
	}
}


/*********************************************************************************************************
** Function name:       LFReaderDevProcess
** Descriptions:        ˢ������ѯ
** input parameters:    RecvMoney���뿨���յ��Ľ��
						cmdOpt����vmc�·����������Ŀ����������ۿ������
						ReaderPrice����vmc�·����������Ŀۿ���
                        readerStatus�������ϱ���vmc��ǰ���ص�״̬����������������                        
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void LFReaderDevProcess(uint32_t *RecvMoney,uint8_t cmdOpt,uint16_t  ReaderPrice, uint8_t *readerStatus)
{
	unsigned char ReaderRdBuff[36],ReaderRdLen,ComStatus;
	unsigned char i;
	static uint8_t readrec=0;
	
	switch(cmdOpt)
	{
		case MBOX_READERENABLEDEV:
			TraceReader("\r\nDrvReaderenable Reader\r\n");
			LFReaderDevEnable();
			break;
		case MBOX_READERDISABLEDEV:
			TraceReader("\r\nDrvReaderdisable Reader\r\n");
			LFReaderDevDisable();
			break;	
		case MBOX_READERVENDREQUEST:
			TraceReader("\r\nDrvReader costRequest=%ld,%ld",ReaderPrice,(ReaderPrice/stDevValue.ReaderScale));
			LFReaderSessionVendRequest(ReaderPrice/stDevValue.ReaderScale);
			//LFReaderSessionVendRequest(0);
			//OSTimeDly(200);
			break;
		case MBOX_READERVENDSUCCESS:
			TraceReader("\r\nDrvReader costSUCC=%ld",(ReaderPrice/stDevValue.ReaderScale));
			LFReaderSessionVendSuccess();
			break;
		case MBOX_READERVENDFAIL:
			TraceReader("\r\nDrvReader costFail=%ld",(ReaderPrice/stDevValue.ReaderScale));
			LFReaderSessionVendFail();
			break;	
		case MBOX_READERVENDCOMP:
			TraceReader("\r\nDrvReader costFailComp=%ld",(ReaderPrice/stDevValue.ReaderScale));
			LFReaderSessionComplete();
			break;	
		case MBOX_READERCASHSALE:
			TraceReader("\r\nDrvReader cashsale=%ld,%ld",ReaderPrice,(ReaderPrice/stDevValue.ReaderScale));
			LFReaderCashSale(ReaderPrice/stDevValue.ReaderScale);			
			//OSTimeDly(200);
			break;	
		default:
			break;
	}
	OSTimeDly(OS_TICKS_PER_SEC / 10);

	//2.Poll
	//for(ComStatus = 0, i = 0; i < 20 && ComStatus != 1; i++)
	{
		ComStatus = MdbConversation(0x12,NULL,0x00,&ReaderRdBuff[0],&ReaderRdLen);
	}
	if(ComStatus == 1)
	{	
		CardStatePack.Error_Card=0;
		readrec=0;
		TraceReader("\r\nDrvReaderPollRec<< [%02d]-",ReaderRdLen);
		for(i=0;i<ReaderRdLen;i++)
		{
			TraceReader(" %#02x ",ReaderRdBuff[i]);
		}
		TraceReader("\r\n");
		for(i = 0; i < ReaderRdLen; i++) 
		{	
			*readerStatus = ReaderRdBuff[i];
			switch(ReaderRdBuff[i])
			{
				case RTV_JUST_RESET://���ڸ�λ״̬		
					TraceReader("\r\n1DrvReaderReset");	
					LFReaderDevInit();
					return;
				
				case RTV_READER_CONFIG_INFO:
					TraceReader("\r\n2DrvReaderSetup");
					LFReaderUpdateConfigData(ReaderRdBuff);//2����setup����Ϣ������level,�������ӵ�
					return;
				
				case RTV_DISPLAY_REQUEST:
					TraceReader("\r\nDrvReaderDisplayRequest...");
					//for future use...
					return;
				
				case RTV_BEGIN_SESSION://7.poll���忨��Ϣ�����뽻��session			
					*RecvMoney = ( ((((uint32_t)ReaderRdBuff[i+1]) << 24) |(((uint32_t)ReaderRdBuff[i+2]) << 16) |(((uint32_t)ReaderRdBuff[i+3]) << 8) | ReaderRdBuff[i+4]) * stDevValue.ReaderScale );
					TraceReader("\r\nDrvReader7BeginSession<<%#02x,%#02x,%ld\r\n",ReaderRdBuff[i+1],ReaderRdBuff[i+2],*RecvMoney);
					//Reader_BeginSession(buf,len);//�õ����ڽ��		
					//ReaderPara.cReaderSta = READER_STA_SESSION_IDLE;
					//Reader_PostToVmc(buf[0]);//display balance	����������䷢�͸�vmc�������̣���ʼ���н���						
					return;
				
				case RTV_SESSION_CANCEL_REQUEST://7���һ�ο���ȡ������session
					TraceReader("\r\nDrvReader7SessionCancelRequest...");
					LFReaderSessionComplete();
					*RecvMoney = 0;
					//ReaderPara.cSessionCancel = 1;//��1������vmcȡ������session
					return;
				
				case RTV_VEND_APPROVED://9�����������㹻��������������
					TraceReader("\r\nDrvReader9VendApproved...");	
					*readerStatus = RTV_VEND_APPROVED;
					//Reader_VendApproved(buf, len);
					//Reader_PostToVmc(buf[0]);//allow dispense����vmc���Գ���
					//OSTimeDly(200*2);
					//LFReaderSessionVendSuccess();
					return;
				
				case RTV_VEND_DENIED://9����������������ؾܾ�����
					TraceReader("\r\nDrvReader9VendDenied");
					*readerStatus = RTV_VEND_DENIED;
					//Reader_PostToVmc(buf[0]);//belance not enough
					return;
				
				case RTV_END_SESSION://7������session,���ؿ���ʹ��״̬	
					TraceReader("\r\nDrvReader7.2EndSession...");
					*RecvMoney = 0;
					//ReaderPara.cReaderSta = READER_STA_ENABLED;//����ʹ��״̬
					//Reader_PostToVmc(buf[0]);//display goodbye
					return;

				case RTV_CANCELLED:
					TraceReader("\r\nDrvReaderCancelled...");
					//reader will reset
					return;
				
				case RTV_PERIPHERAL_ID:
					TraceReader("\r\nDrvReaderPeripheralID...");
					return;
				
				case RTV_MALFUNCTION_ERROR:
					TraceReader("\r\nDrvReaderMalfunctionError...");
					//Reader_MalfunctionError(buf, len);
					//Reader_PostToVmc(buf[0]);//reader error
					return;
				
				case RTV_CMD_OUT_OF_SEQUENCE:
					TraceReader("\r\nDrvReaderCmdOutOfSequence");					
					//Reader_PostToVmc(buf[0]);
					return;

				//level 1+...		
				
				//case 0x0D://Revalue Approved
				//break;
				//case 0x0E://Revalue Denied
				//break;
				//case 0x0F://Revalue Limit Amount
				//break;
				//case 0x10://User File Data
				//break;
				//case 0x11://Time/Data Request
				//break;
				//case 0x12://Data Entry Request
				//break;	
				
				default://...
					TraceReader("\r\nDrvReaderLevel 1+ ... buf[0] = %02x", ReaderRdBuff[i]);
					return;
			}
		}
	}
	else
	{
		readrec++;
		TraceReader("\r\n Drvread commuFail,readrec=%d",readrec);
		if(readrec>=30)
		{
			TraceReader("\r\n Drvread commReject");
			CardStatePack.Error_Card=1;
			readrec=0;
		}	
		
	}
	OSTimeDly(OS_TICKS_PER_SEC/100);
}
