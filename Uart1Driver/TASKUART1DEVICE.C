/****************************************Copyright (c)*************************************************
**                      Fujian Junpeng Communicaiton Technology Co.,Ltd.
**                               http://www.easivend.com.cn
**--------------File Info------------------------------------------------------------------------------
** File name:           TASKDEVICE
** Last modified Date:  2013-03-04
** Last Version:         
** Descriptions:        串口1任务                     
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
#include "TASKUART1DEVICE.H"
#include "SelectKey.h"
//#include "Xmt.h"



/*
#define COINDEV_MDB			2				//当前系统识别到的硬币器类型为MDB
#define BILLDEV_MDB			2				//当前系统识别到的纸币器类型为MDB

#define READERDEV_MDB		2				//当前系统识别到的读卡器类型为MDB
 */

#define OFF_KEY			    0//普通键盘按键
#define SELECT_KEY			1//选货按键
#define GEZIGUI		    	1//开启格子柜






/*********************************************************************************************************
** Function name:       Uart1TaskDevice
** Descriptions:        设备任务:主要处理挂在UART2上的设备，如MDB协议,EVB协议等
** input parameters:    pvData: 没有使用
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void Uart1TaskDevice(void *pvData)
{
	MessageKEYPack *AccepterMsg;
	MessageXMTPack *XMTPack;
	//MessageFSBillRecyclerPack *FSBillRecyclerMsg;
	unsigned char ComStatus;
	MessagePack *GeziMsg;
	uint8_t key=0xff;
	
	//当前选货设备的类型
	uint8_t NowSelectDev = 0;
	//开启加热设备
	uint8_t TempDev = 0,temp=0;
	uint8_t TempCtr = 1;
	//开启格子柜
	uint8_t GeziCtr = 0;
	uint8_t rst=0;
	
	
	//检查选货按键控制
	AccepterMsg = OSMboxPend(g_KEYMail,OS_TICKS_PER_SEC,&ComStatus);
	if(ComStatus == OS_NO_ERR)
	{
		if((AccepterMsg->KeyCmd)==MBOX_SELECTKEYINIT)
		{	
			//OSTimeDly(1000);
			TraceSelection("\r\n Taskpend selectinit"); 
			LCDNumberFontPrintf(40,LINE15,2,"SelectAccepter-1");	
			memset(&selKey,0,sizeof(selKey));
			SelectKey_InitProc();	
			UpdateSelectionLed(AccepterMsg);
			NowSelectDev = SELECT_KEY;			
		}
		else
		{
			NowSelectDev = 0;
		}
	}	
	else
	{
		NowSelectDev = 0;
	}
	OSTimeDly(2);
	//检查温控器控制
	if(SystemPara.XMTTemp==1)
	{
		LCDNumberFontPrintf(40,LINE15,2,"XMT-1");	
		TempDev=TEMPERATURE;
		Timer.getTempTimer = 10;
		OSTimeDly(2);
	}
	//检查格子柜控制
	if(SystemPara.hefangGui==SERIAL_GEZI)
	{
		LCDNumberFontPrintf(40,LINE15,2,"GEZI-1");	
		GeziCtr = GEZIGUI;
		OSTimeDly(2);
	}
	//检查富士找零器
	if(SystemPara.BillRecyclerType == FS_BILLRECYCLER)
	{
		LCDNumberFontPrintf(40,LINE15,2,"FS-1");	
		OSTimeDly(200*2);
		FS_init();
	}
	
	
	while(1)
	{		
		if(NowSelectDev == SELECT_KEY)
		{
			//Trace("\r\n UART1TASK");
			//1.poll选货按键
			key=GetSelectKey();
			//有按键，上报按键值
			if(key != 0xff)
			{				
				MsgKEYBackPack.KeyBackCmd = MBOX_SELECTVALUE;	
				MsgKEYBackPack.selectInput = key;
				OSMboxPost(g_KEYBackMail,&MsgKEYBackPack);
				TraceSelection("\r\n Taskpkey=%d",key); 
				key=0xff;
				OSTimeDly(OS_TICKS_PER_SEC * 2);
			}
			//2.检查选货按键控制
			AccepterMsg = OSMboxPend(g_KEYMail,OS_TICKS_PER_SEC/100,&ComStatus);
			//TraceSelection("\r\n Taskkeymail1=%d",ComStatus); 
			if(ComStatus == OS_NO_ERR)
			{
				//TraceSelection("\r\n Taskkeymail2=%d",AccepterMsg->KeyCmd); 
				if((AccepterMsg->KeyCmd)==MBOX_SELECTLIGHT)
				{	
					UpdateSelectionLed(AccepterMsg);
				}
			}	
		}
		//温控器
		else if(TempDev==TEMPERATURE)
		{	
			if(Timer.getTempTimer==0)
			{
				Timer.getTempTimer = 10;
				temp=XMTMission_GetTemperature();
				Trace("\r\n temp=%d,GetPV=%d.%d,GetSV=%d.%d",temp,sysXMTMission.recPVTemp/10, sysXMTMission.recPVTemp%10,sysXMTMission.recSVTemp/10, sysXMTMission.recSVTemp%10);
			}
			OSTimeDly(2);
			//2.检查温控器控制
			XMTPack = OSMboxPend(g_XMTMail,OS_TICKS_PER_SEC/100,&ComStatus);
			//Trace("\r\n Taskkeymail1=%d",ComStatus); 
			if(ComStatus == OS_NO_ERR)
			{
				//Trace("\r\n Taskkeymail2=%d",XMTPack->KeyCmd); 
				if((XMTPack->KeyCmd)==MBOX_XMTSETTEMP)
				{	
					TempCtr = XMTMission_SetTemperatureS(XMTPack->temparater);
					if(TempCtr==0)//成功
					{
						MsgXMTPack.KeyBackCmd = MBOX_XMTTEMPOK;							
					}
					else//失败
					{
						MsgXMTPack.KeyBackCmd = MBOX_XMTTEMPFAIL;	
					}
					OSMboxPost(g_XMTBackMail,&MsgXMTPack);
					OSTimeDly(2);
				}
			}
		}
		//检查格子柜
		else if(GeziCtr == GEZIGUI)
		{
			//接收盒饭柜控制邮箱  Add by liya 2014-01-20
			GeziMsg = OSMboxPend(g_HeFanGuiMail,OS_TICKS_PER_SEC/100,&ComStatus);
			if(ComStatus == OS_NO_ERR)
			{
				switch(GeziMsg->HeFanGuiHandle)	
				{
					case HEFANGUI_KAIMEN:
					case HEFANGUI_CHAXUN:
					case HEFANGUI_JIAREKAI:
					case HEFANGUI_JIAREGUAN:
					case HEFANGUI_ZHILENGKAI:
					case HEFANGUI_ZHILENGGUAN:
					case HEFANGUI_ZHAOMINGKAI:
					case HEFANGUI_ZHAOMINGGUAN:
						TraceChannel("recv..CMD    lvel=%d\r\n",GeziMsg->HeFanGuiHandle);
						rst = HeFanGuiDriver(GeziMsg->Binnum,GeziMsg->HeFanGuiHandle,GeziMsg->HeFanGuiNum,MsgAccepterPack.HeFanGuiBuf);
						TraceChannel("Task_res==%d\r\n",rst);
						MsgAccepterPack.HeFanGuiHandle = GeziMsg->HeFanGuiHandle;
						MsgAccepterPack.HeFanGuiRst = rst;
						OSMboxPost(g_HeFanGuiBackMail,&MsgAccepterPack);
						break;
				}
			}
			OSTimeDly(2);	
		}
		//检查富士找零器
		else if(SystemPara.BillRecyclerType == FS_BILLRECYCLER)
		{
			FS_poll();
			msleep(10);
		}
		else	
		{
			OSTimeDly(2);
		}

		//检查纸币器控制
		/*FSBillRecyclerMsg = OSMboxPend(g_FSBillRecyclerMail,1,&ComStatus);
		if(ComStatus == OS_NO_ERR)
		{
			//纸币器按张数找币
			if(FSBillRecyclerMsg->BillBack == MBOX_FSBILLRECYPAYOUTNUM)
			{
				print_fs("\r\n TaskFSPay=%ld,Num=%d",FSBillRecyclerMsg->RecyPayoutMoney,FSBillRecyclerMsg->RecyPayoutNum);
                          PayoutMoney=(FSBillRecyclerMsg->RecyPayoutMoney)*(FSBillRecyclerMsg->RecyPayoutNum);
				//PayouBacktMoney=FS_dispense(PayoutMoney);
				MsgFSBillRecyclerPack.BillBackCmd = MBOX_FSBILLRECYPAYOUTSUCC;	
				MsgFSBillRecyclerPack.RecyPayoutMoneyBack=PayouBacktMoney;
				OSMboxPost(g_FSBillRecyclerBackMail,&MsgFSBillRecyclerPack);
			}
		}*/
		//OSTimeDly(50/5);
	}	
}
