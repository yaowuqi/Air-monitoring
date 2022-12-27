#include"stm32f10x.h"
#include"stm32f10x_it.h"
#include"stm32f10x_exti.h"
#include"FreeRTOS.h"
#include"task.h"
#include"System_Init.h"
#include"bsp_gpio.h"
#include"bsp_uart.h"
#include"bsp_iwdg.h"
//device
#include"delay.h"
#include"led.h"
#include"app_func.h"
#include"app_hmi.h"
intmain(void)
{
__set_PRIMASK(1);
//peripheralinit
System_Init();
//devicerinit
bsp_GpioInit();
usartInit();
//taskinit
app_FuncInit();
app_HmiInit();
bsp_IwdgInit();
/*Startthescheduler.*/
vTaskStartScheduler();
while(1);
}
[6]app_hmi.c//温湿度显示及模拟信号设置
#include"FREERTOS.h"
#include"task.h"
#include"bsp_uart.h"
#include"serialInterface.h"
#include"uart_api.h"
#include"stdarg.h"
#include"app_func.h"
#include"app_hmi.h"
#defineFrameHead
#defineFrameEnd
#defineCMD_Main
#defineCMD_Set1
#defineCMD_Set2
#defineEvent_MainIn
#defineEvent_MainOut
0xA5
0xBB
0xA0
0xA1
0xA2
0x00
0x01
//进入主页
//离开主页
//等待超时ms
#defineFrame_OverTime200
//动作设置数据
TACTDATAActData;
staticboolisMainPage=true;
staticuint8_tcount_1s=0;
staticvoidapp_hmiTask(void*data);
staticboolisShowRes=false;
staticuint16_tisShowResCount=0;
//串口发送缓存区
//主页标志
staticchartxBuf[128];
voidapp_HmiPrintf(char*fmt,...)
{
//发送缓冲,最大256字节
intlen=0;
va_listap;
va_start(ap,fmt);
memset(txBuf,0,128);
vsprintf(txBuf,fmt,ap);
va_end(ap);
len=strlen(txBuf);
//帧尾
txBuf[len]=0xff;
txBuf[len+1]=0xff;
txBuf[len+2]=0xff;
sysSerialSend(serialHMI,(uint8_t*)txBuf,len+3);
}
//显示测量结果
voidapp_HmiShowRes(void)
{
count_1s=0;
app_HmiPrintf("pageres");
vTaskDelay(20);
app_HmiPrintf("t10.txt=\"%d:%d:%d\"",12,23,34);
//检测过
程时长
app_HmiPrintf("t11.txt=\"%d\"",34);
//检测高度
//平均温度
//平均湿度
app_HmiPrintf("t12.txt=\"%d\"",24);
app_HmiPrintf("t13.txt=\"%d\"",64);
}
voidapp_HmiInit(void)
{
portBASE_TYPEsTatus;
sTatus=xTaskCreate(app_hmiTask,(char*)"apphmi",128,NULL,
(tskIDLE_PRIORITY+1),
NULL);
if(sTatus==pdTRUE)
serialPrintf("apphmiinitsucc.\r\n");
serialPrintf("apphmiinitfail.\r\n");
else
}
staticvoidapp_hmiTask(void*data)
{
uint16_trcv_len=0;
uint8_tbuf[128];
uint8_tvalue=0,value1=0;
uint8_twendu[]={30,29,28};
uint8_tshidu[]={58,60,57};
floatjiaquan[]={0.02,0.04,0.04,0.03,0.05,0.06,0.05,0.0
4,0.04,0.03};
uint8_tdb[]={62,64,64,64,66,76,75,68,74,66};
memset(&ActData,0,sizeof(TACTDATA));
for(;;){
vTaskDelay(20);
if(++count_1s>50){
	count_1s=0;
if(++value>2)
value=0;
if(++value1>=10)
value1=0;
//发送温湿度数据
app_HmiPrintf("t3.txt=\"%d\"",wendu[value]);
app_HmiPrintf("t4.txt=\"%d\"",shidu[value]);
app_HmiPrintf("t6.txt=\"%0.2f\"",jiaquan[value1]);
if(jiaquan[value1]<0.05){
app_HmiPrintf("t9.txt=\"正常\"");
app_HmiPrintf("t9.pco=1766");
}else{
app_HmiPrintf("t9.txt=\"超限\"");
app_HmiPrintf("t9.pco=RED");
}
app_HmiPrintf("t8.txt=\"%d\"",db[value1]);
}
}
}
[7]/***************led模式设置**************
*输入参数：
*
*
*
*
*
ledport
ledon.
led编号
led点亮时间s
led灭时间s
(50的倍数)为0时常灭
(50的倍数)为0时常亮
ledoff.
isConfirm.led快闪3次提示
false:不提示
true:提示
***************************************/
