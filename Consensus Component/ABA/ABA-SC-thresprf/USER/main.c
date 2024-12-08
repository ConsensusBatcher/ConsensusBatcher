#include "sys.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "utility.h"
#include "mw1268_app.h"
#include "tendermint2.h"
#include "uECC.h"
#include "rng.h"
#include "ABA_Shared.h"
#include "btim.h"
#include "usart3.h"
#include "core.h"
#include "bls_BN158.h"
#include "dma.h"

#include "mbedtls/aes.h"

#define ORIGINAL_DATA   "ABCDEFGHIJKLMNOP"
#define PASSWORD        "AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDD"
#define PLAINSTRING     "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"

#define USE_STM32F7XX_NUCLEO_144

u32 Total_low_time = 0;
u8 low_flag = 0;
u32 low_time[400] = {0};
u32 wfi_time[400] = {0};
u32 low_IDX = 0;

u32 MY_TIME=0;
u32 MY_Consensus_TIME=0;
u8 need_er = 0;


u8 run_round_flag = 0;
u8 run_round = 700;
u8 parallel_D = 4;


/****************ABA_Shared***************************************/
u8 start_ABA = 0;

u32 TIM5_Exceed_Times;
u32 TIM2_Exceed_Times;
u8 ABA_share[MAX_round][MAX_Nodes][Share_Size] = {0};//第x轮的哪个节点的share
BIG_160_28 ABA_Z[MAX_round][MAX_Nodes];
u8 ABA_Z_share[MAX_round][MAX_Nodes][32];
BIG_160_28 ABA_C[MAX_round][MAX_Nodes];
u8 ABA_C_share[MAX_round][MAX_Nodes][32];
u8 ACK_ABA_share[MAX_round][MAX_Nodes] = {0};//第x轮的哪个节点的share收到了
u8 ABA_share_Number[MAX_round];//第x轮share数量
u8 Shared_Coin[MAX_round];//第x轮的coin //FF->NULL
u8 COIN[Share_Size];
u8 ABA_sig[MAX_round][Share_Size];

u8 est[MAX_ABA][MAX_round];//第x轮的估计值
u8 bin_values[MAX_ABA][MAX_round][2];
//BVAL
u8 BVAL[MAX_ABA][MAX_round][2];//发过的BVAL
u8 ACK_BVAL[MAX_ABA][MAX_round][MAX_Nodes][2];//第x轮哪个节点的0/1收到了
u8 ACK_BVAL_Number[MAX_ABA][MAX_round][2];//第x轮的0/1各收到了多少个

//AUX
u8 AUX[MAX_ABA][MAX_round][2];//发过的AUX
u8 vals[MAX_ABA][MAX_round][2];//收到的AUX
u8 ACK_AUX[MAX_ABA][MAX_round][MAX_Nodes][2];//第x轮哪个节点的0/1收到了
u8 ACK_AUX_Number[MAX_ABA][MAX_round][2];//第x轮的0/1各收到了多少个

//发送几个Round 的STATE。
u8 STATE_Set[MAX_Nodes];
u8 STATE_Set_Number;
//发送几个Round 的NACK。
u8 NACK_Set[MAX_Nodes];
u8 NACK_Set_Number;

u8 ABA_decided[MAX_ABA];//FF->NULL
u8 ABA_done_Number;
u32 ABA_time[MAX_ABA];
u8 ABA_round[MAX_ABA];
u8 ABA_R[MAX_ABA];
u32 ABA_round_time[5][8];

u8 aba_id = 0;
u8 aba_id_flag = 0;
/*******************************************************/


/****************RBC_Bracha***************************************/
u8 Block[MAX_Nodes][Block_Num][Block_Size];
u8 Block_each_Size[MAX_Nodes][Block_Num];
u8 hash_v[MAX_Nodes][Hash_Size];
u8 RBC_Block[MAX_Nodes][Block_Num];
u8 RBC_Block_Number[MAX_Nodes];
u8 RBC_Init[MAX_Nodes];

u8 ACK_RBC_Echo[MAX_Nodes][MAX_Nodes];
u8 RBC_Echo[MAX_Nodes];
u8 RBC_Echo_Number[MAX_Nodes];
u8 ACK_RBC_Ready[MAX_Nodes][MAX_Nodes];
u8 RBC_Ready[MAX_Nodes];
u8 RBC_Ready_Number[MAX_Nodes];

u8 RBC_done[MAX_Nodes];
u8 RBC_done_Number;
u8 RBC_init_Number=0;
/****************************************************************/



/********************Threshole Encryption*************************************/
u8 Transaction[MAX_Nodes][Proposal_Size];
u8 tx_passwd[PD_Size];

u32 Proposal_Packet_Size;

u8 enc_tx_passwd[MAX_Nodes][PD_Size];
u8 dec_tx_passwd[MAX_Nodes][PD_Size];
u8 tmp_tx_buff[MAX_PROPOSAL];
u8 tmp_enc_tx_buff[MAX_PROPOSAL];
u8 tmp_dec_tx_buff[MAX_PROPOSAL];
u8 ACK_Share[MAX_Nodes][MAX_Nodes];
u8 Tx_Share[MAX_Nodes][MAX_Nodes][Share_Size];
ECP_BN158 Tx_U[MAX_Nodes];
u8 Tx_U_str[MAX_Nodes][Share_Size];

ECP2_BN158 Tx_W[MAX_Nodes],Tx_H[MAX_Nodes];
ECP_BN158 Tx_shares[MAX_Nodes][MAX_SK];
u8 Tx_shares_str[MAX_Nodes][MAX_SK][Share_Size];
u8 Tx_shares_Number[MAX_Nodes];
u8 dec_tx_flag[MAX_Nodes];
u8 dec_tx_number;

u8 dec_done_number = 0;
/****************************************************************/


/************************************************
 ALIENTEK 阿波罗STM32F767开发板 FreeRTOS实验2-1
 FreeRTOS移植实验-HAL库版本
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		10240
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define CSMACA_PBFT_TASK_PRIO		2
//任务堆栈大小	
#define CSMACA_PBFT_STK_SIZE 		256*2  
//任务句柄
TaskHandle_t CSMACA_PBFT_Task_Handler;
//任务函数
void CSMACA_PBFT(void *pvParameters);

//任务优先级
#define EVENTSETBIT_TASK_PRIO	3
//任务堆栈大小	
#define EVENTSETBIT_STK_SIZE 	512*2
//任务句柄
TaskHandle_t EventSetBit_Handler;
//任务函数
void eventsetbit_task(void *pvParameters);

//任务优先级
#define EVENTGROUP_TASK_PRIO	3
//任务堆栈大小	
#define EVENTGROUP_STK_SIZE 	1024*5
//任务句柄
TaskHandle_t EventGroupTask_Handler;
//任务函数
void eventgroup_task(void *pvParameters);

u8 buff[300];

void init(){
	while(LoRa_Init())//?????LORA???
	{
		delay_ms(300);
	}
	LoRa_CFG.chn = 10;
	LoRa_CFG.tmode = LORA_TMODE_PT;
	LoRa_CFG.addr = 0;
	LoRa_CFG.power = LORA_TPW_9DBM;
	LoRa_CFG.wlrate = LORA_RATE_62K5;
	LoRa_CFG.lbt = LORA_LBT_ENABLE;
	LoRa_Set();
}

void RBC_init();

u8 decidd_flag = 1;
void ABA_init();
void ABA_STATE_handler();
void ABA_NACK_handler();
void RBC_INIT_handler();
void RBC_ER_handler();
void PACKET_handler();
void Tx_init();
void ENC_handler();
typedef enum
{
	TypeNACK = 0,
	TypeSTATE,
	TypeRBC_INIT,
	TypeRBC_ER,
	TypeTX_ENC
} PacketType;
int main(void)
{
    /* Configure the MPU attributes */
	MPU_Config();
	//CPU_CACHE_Enable();                 //打开L1-Cache  DMA
    HAL_Init();				        //初始化HAL库
	SystemClock_Config();
	
    delay_init(216);                //延时初始化
    LED_Init();                     //初始化LED
	
    //创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

int PAD(u8 *str, int len){
	//int _n = strlen(str);
	int n = 16 - (len % 16);
	for(int i=0;i<n;i++){
		str[len+i] = n;
	}
	return n + len;
}

void UNPAD(u8 *str, int len){
//	int _n = strlen(str);
	int n = str[len-1];
	for(int i=0;i<n;i++){
		str[len-i-1] = 0x00;
	}
}




u8 tmp_buff[3000];
u8 tx_buff[300];
u8 cipher_buff[300];
u8 decrypt_buff[300];
u8 PASSWD[PD_Size];

int Tx_encrypt(u8 *key, u8 *raw, int raw_len, u8 *enc){
	int pad_len = raw_len;// = PAD(raw, raw_len);
	u8 iv[16];
	u8 iv2[16];
	for(u8 i=0;i<16;i++){
		iv[i] = i+1;//RNG_Get_RandomNum();
		iv2[i] = iv[i];
	}
	
	mbedtls_aes_context ctx;
	
	// 加密
	mbedtls_aes_setkey_enc(&ctx, (unsigned char *)key, 128);
	mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, pad_len, iv, raw, enc);	
	
	for(int i=0;i<pad_len;i++)tmp_buff[i] = enc[i];
	for(int i=0;i<16;i++)enc[i] = iv2[i];
	for(int i=0;i<pad_len;i++)enc[16+i] = tmp_buff[i];
	
	return pad_len+16;
}
	
void Tx_decrypt(u8 *key, u8 *enc, int enc_len, u8 *dec){
	u8 iv[16];
	for(int i=0;i<16;i++)iv[i] = enc[i];
	//int enc_len = strlen(enc);
	for(int i=0;i<enc_len + 16;i++)tmp_buff[i] = enc[i];
	for(int i=0;i<enc_len;i++)enc[i] = 0x00;
	for(int i=0;i<enc_len;i++)enc[i] = tmp_buff[16 + i];
	
	mbedtls_aes_context ctx;
	// 解密
	mbedtls_aes_setkey_dec(&ctx, (unsigned char *)key, 128);
	mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, enc_len-16, iv, enc, dec);
}



//开始任务任务函数
void start_task(void *pvParameters)
{

//	for(int i=0;i<300;i++){
//		tx_buff[i] = 0x00;
//		tmp_buff[i] = 0x00;
//		cipher_buff[i] = 0x00;
//		decrypt_buff[i] = 0x00;
//	}
//	for(int i=0;i<198;i++)tx_buff[i] = 'a';
//	for(int i=0;i<16;i++)PASSWD[i] = 'a'+i;
//	int len = Tx_encrypt(PASSWD, tx_buff, 198, cipher_buff);
//	Tx_decrypt(PASSWD,cipher_buff,len,decrypt_buff);
//	UNPAD(decrypt_buff,len);
	
	init();
	init_public_key();
	init_thres_enc();
	ABA_init();
	Tx_init();
	RBC_init();

	//test_thres();
	TIM5_Exceed_Times = 0;
	TIM2_Exceed_Times = 0;
	btim_tim5_int_init(9000-1,10800-1);//108Mhz/10800 = 10000hz  900ms
	btim_tim2_int_init(9000-1,10800-1);
	
    taskENTER_CRITICAL();           //进入临界区
	
//    //创建LED0任务
//    xTaskCreate((TaskFunction_t )CSMACA_PBFT,     	
//                (const char*    )"csmaca_pbft",   	
//                (uint16_t       )CSMACA_PBFT_STK_SIZE, 
//                (void*          )NULL,					
//                (UBaseType_t    )CSMACA_PBFT_TASK_PRIO,	
//                (TaskHandle_t*  )&CSMACA_PBFT_Task_Handler);   
    
	//创建设置事件位的任务
    xTaskCreate((TaskFunction_t )eventsetbit_task,             
                (const char*    )"eventsetbit_task",           
                (uint16_t       )EVENTSETBIT_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EVENTSETBIT_TASK_PRIO,        
                (TaskHandle_t*  )&EventSetBit_Handler);   	
    //创建事件标志组处理任务
    xTaskCreate((TaskFunction_t )eventgroup_task,             
                (const char*    )"eventgroup_task",           
                (uint16_t       )EVENTGROUP_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EVENTGROUP_TASK_PRIO,        
                (TaskHandle_t*  )&EventGroupTask_Handler);     
    vTaskDelete(StartTask_Handler); //删除开始任务
	
	
	btim_tim5_enable(1);			
    taskEXIT_CRITICAL();            //退出临界区
}

//LED0任务函数
void CSMACA_PBFT(void *pvParameters)
{
	//btim_tim5_int_init(9000-1,10800-1);//108Mhz/10800 = 10000hz  900ms
	//btim_tim2_int_init(9000-1,10800-1);
	//btim_tim9_int_init(9000-1,21600-1);//216*1000,000hz/21600 = 10000hz  500ms
	//tendermint2();
	vTaskSuspend(CSMACA_PBFT_Task_Handler);
}   


//设置事件位的任务
void eventsetbit_task(void *pvParameters)
{
	
//	btim_timx_enable(DISABLE);
//	HAL_NVIC_DisableIRQ(BTIM_TIMX_INT_IRQn);
//	HAL_NVIC_DisableIRQ(USART3_IRQn);
	
	while(1)
	{
//		STATE_Set_Number = 1;
//		STATE_Set[0] = 1;
//		NACK_Set_Number = 1;
//		NACK_Set[0] = 1;
		if(EventGroupTask_Handler!=NULL)
		{
			vTaskDelay(1000);
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//接收任务通知的任务句柄
//						(uint32_t		)EVENTBIT_1,			//要更新的bit
//						(eNotifyAction	)eSetBits);				//更新指定的bit
			
			vTaskDelay(2000);
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//接收任务通知的任务句柄
//						(uint32_t		)EVENT_ABA_NACK,			//要更新的bit
//						(eNotifyAction	)eSetBits);				//更新指定的bit
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//接收任务通知的任务句柄
//						(uint32_t		)EVENTBIT_2,			//要更新的bit
//						(eNotifyAction	)eSetBits);				//更新指定的bit
			vTaskDelay(1000);	
		}
	
	}
}

//事件标志组处理任务
void eventgroup_task(void *pvParameters)
{
	u8 num=0,enevtvalue;
	static u8 event0flag,event1flag,event2flag;
	uint32_t NotifyValue;
	BaseType_t err;
	
	while(1)
	{
		//获取任务通知值
		err=xTaskNotifyWait((uint32_t	)0x00,				//进入函数的时候不清除任务bit
							(uint32_t	)ULONG_MAX,			//退出函数的时候清除所有的bit
							(uint32_t*	)&NotifyValue,		//保存任务通知值
							(TickType_t	)portMAX_DELAY);	//阻塞时间
		
		if(err==pdPASS)	   //任务通知获取成功
		{
			if((NotifyValue&EVENTBIT_0)!=0)			//事件0发生	
			{
				for(int i=0;i<300;i++)ReceBuff[i] = 0x00;
			}				
			else if((NotifyValue&EVENTBIT_1)!=0)	//事件1发生	
			{
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//接收任务通知的任务句柄
//						(uint32_t		)EVENT_PACKET,			//要更新的bit
//						(eNotifyAction	)eSetBits);				//更新指定的bit
//				delay_ms(300);
//				btim_tim2_enable(1);
//				for(int i=0;i<100;i++)buff[i] = OkReceBuff[OkIdx][i];
//				sprintf((char*)Send_Data_buff,"STATE: Get EVENT 1.");LoRa_SendData(Send_Data_buff);
			}
			else if((NotifyValue&EVENTBIT_2)!=0)	//事件2发生	
			{
				for(int i=0;i<300;i++)ReceBuff[i] = 0x00;
				//sprintf((char*)Send_Data_buff,"STATE: Get EVENT 2.");LoRa_SendData(Send_Data_buff);	
			}
			else if((NotifyValue&EVENT_ABA_STATE)!=0)	//STATE发生	
			{
				ABA_STATE_handler();
			}
			else if((NotifyValue&EVENT_PACKET)!=0)	//PACKET	
			{
				//sprintf((char*)Send_Data_buff,"STATE: Get EVENT_PACKET.");LoRa_SendData(Send_Data_buff);
				PACKET_handler();
			}

		}
		else
		{
			sprintf((char*)Send_Data_buff,"ERROR: Get mission.");
			LoRa_SendData(Send_Data_buff);	
		}
	}
}


void Tx_init(){}

void RBC_init(){}
	
void ABA_init(){
	start_ABA = 0;
	ABA_done_Number = 0;
	for(int i=0;i<MAX_round;i++){
		ABA_share_Number[i] = 0x00;
		Shared_Coin[i] = 0xFF;
		for(int j=0;j<Share_Size;j++){
			ABA_sig[i][j] = 0x00;
		}
		for(int j=0;j<MAX_Nodes;j++){
			for(int k=0;k<Share_Size;k++)ABA_share[i][j][k] = 0x00;
			ACK_ABA_share[i][j] = 0x00;
		}
	}
	for(int a=0;a<MAX_ABA;a++)
	for(int i=0;i<MAX_round;i++){
		est[a][i] = 0x00;
		bin_values[a][i][0] = 0x00;bin_values[a][i][1] = 0x00;
		ACK_BVAL_Number[a][i][0] = 0x00;ACK_BVAL_Number[a][i][1] = 0x00;
		for(int j=0;j<MAX_Nodes;j++){
			ACK_BVAL[a][i][j][0] = 0x00;ACK_BVAL[a][i][j][1] = 0x00;
		}
	}
	for(int a=0;a<MAX_ABA;a++)
	for(int i=0;i<MAX_round;i++){
		vals[a][i][0] = 0x00;vals[a][i][1] = 0x00;
		ACK_AUX_Number[a][i][0] = 0x00;ACK_AUX_Number[a][i][1] = 0x00;
		for(int j=0;j<MAX_Nodes;j++){
			ACK_AUX[a][i][j][0] = 0x00;ACK_AUX[a][i][j][1] = 0x00;
		}
	}
	
	MYDMA_Config_Rece(DMA1_Stream1,DMA_CHANNEL_4);//初始化DMA
	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);

	init_thres_sig();
	
	for(int i=0;i<MAX_ABA;i++)ABA_R[i] = 1;
	
	for(int i=0;i<MAX_ABA;i++)
		for(int j=0;j<MAX_round;j++)
			est[i][j] = 0xFF;
	
	for(int i=0;i<MAX_ABA;i++){
		u8 vote = RNG_Get_RandomNum() % 2;
		if(run_round_flag) vote = 1;
		est[i][1] = vote;
		BVAL[i][1][vote] = 1;
		ACK_BVAL_Number[i][1][vote] = 1;
	}
		
	for(int i=0;i<MAX_Nodes;i++)STATE_Set[i]=0;
	STATE_Set_Number = 1;
	
	for(int i=0;i<MAX_ABA;i++)
		ABA_decided[i] = 0xFF;

	if(ID == 1){
		aba_id = RNG_Get_RandomNum();
		aba_id_flag = 1;
	}
}
u8 ttt = 0;
u8 xxx = 0;
u8 mmm = 0;
u8 nnn = 0;
int subset_or_not(u8 aba, u8 round){
	if(bin_values[aba][round][0] == 0 && bin_values[aba][round][1] == 0)return 0;
	if(bin_values[aba][round][0] == 1 && bin_values[aba][round][1] == 0){
		if(vals[aba][round][0] == 1 && vals[aba][round][1] == 0)return 1;
		return 0;
	}
	if(bin_values[aba][round][0] == 0 && bin_values[aba][round][1] == 1){
		if(vals[aba][round][0] == 0 && vals[aba][round][1] == 1)return 1;
		return 0;
	}
	if(bin_values[aba][round][0] == 1 && bin_values[aba][round][1] == 1){
		if(vals[aba][round][0] == 0 && vals[aba][round][1] == 0)return 0;
		return 1;
	}
}


void ABA_STATE_handler(){

	for(int i=0;i<300;i++)Send_Data_buff[i] = 0x00;
	if(Nodes_N<=8){// && ABA_decided[1]==0xFF && ABA_decided[2]==0xFF && ABA_decided[3]==0xFF && ABA_decided[4]==0xFF){
		Send_Data_buff[0] = ID;
		Send_Data_buff[1] = TypeSTATE;
		u8 padding = 20;
		Send_Data_buff[2] = padding;
		for(int i=0;i<padding;i++)Send_Data_buff[3+i] = 0xFF;
		
		if(ID == 1)Send_Data_buff[3] = aba_id;
		if(aba_id_flag == 1)Send_Data_buff[4] = 2;
		
		u8 packet_idx = 3 + padding;
		Send_Data_buff[packet_idx] = STATE_Set_Number;
		for(int o=0;o<STATE_Set_Number;o++){
			packet_idx++;
			Send_Data_buff[packet_idx] = STATE_Set[o];//round number
			u8 round = STATE_Set[o];
			//BVAL
			packet_idx++;
			for(int j=1;j<=Nodes_N;j++){//nodes
				if(ID == j){
					for(int k=1;k<=parallel_D;k++){//ABA id
						if(BVAL[k][round][0])Send_Data_buff[packet_idx+(j-1)*2] |= (1<<(k-1));
						if(BVAL[k][round][1])Send_Data_buff[packet_idx+1+(j-1)*2] |= (1<<(k-1));
					}
					continue;
				}
				for(int k=1;k<=parallel_D;k++){//ABA id
					if(ACK_BVAL[k][round][j][0])Send_Data_buff[packet_idx+(j-1)*2] |= (1<<(k-1));
					if(ACK_BVAL[k][round][j][1])Send_Data_buff[packet_idx+1+(j-1)*2] |= (1<<(k-1));
				}
			}
			packet_idx += 2 * Nodes_N;
			
			//AUX
			for(int j=1;j<=Nodes_N;j++){//nodes
				if(ID == j){
					for(int k=1;k<=parallel_D;k++){//ABA id
						if(AUX[k][round][0])Send_Data_buff[packet_idx+(j-1)*2] |= (1<<(k-1));
						if(AUX[k][round][1])Send_Data_buff[packet_idx+1+(j-1)*2] |= (1<<(k-1));
					}
					continue;
				}
				for(int k=1;k<=parallel_D;k++){//ABA id
					if(ACK_AUX[k][round][j][0])Send_Data_buff[packet_idx+(j-1)*2] |= (1<<(k-1));
					if(ACK_AUX[k][round][j][1])Send_Data_buff[packet_idx+1+(j-1)*2] |= (1<<(k-1));
				}
			}
			packet_idx += 2 * Nodes_N;
			
			//share_nack
			for(int k=1;k<=Nodes_N;k++)
				if(ACK_ABA_share[round][k])Send_Data_buff[packet_idx] |= (1<<(k-1));
			packet_idx++;
			
			//share
			//share 自己的share 有没有计算?
			if(ACK_ABA_share[round][ID] == 0 && aba_id_flag){
				Coin_SK_sign(round);
			}
			//share
			for(int i=0;i<Share_Size;i++){
				Send_Data_buff[packet_idx + i] = ABA_share[round][ID][i];
			}
			packet_idx += Share_Size;
			//verify share z
			for(int i=0;i<BIG_Size;i++){
				Send_Data_buff[packet_idx + i] = ABA_Z_share[round][ID][i];
			}
			packet_idx += BIG_Size;
			//verify share c
			for(int i=0;i<BIG_Size;i++){
				Send_Data_buff[packet_idx + i] = ABA_C_share[round][ID][i];
			}
			packet_idx += BIG_Size;
		}
		
		//Sign
		uint8_t hash[32] = {0};
		uint8_t sig[64] = {0};
		//sha2(Send_Data_buff,packet_idx,hash,0);
		hash256 sh256;
		HASH256_init(&sh256);
		for (int i=0;i<packet_idx;i++) HASH256_process(&sh256,Send_Data_buff[i]);
		HASH256_hash(&sh256,hash);
		
		sign_data(hash,sig);
		for(int i=0;i<Sig_Size;i++)Send_Data_buff[packet_idx+i] = sig[i];
		
//		HAL_UART_DMAStop(&uart3_handler);
//		HAL_UART_Transmit_DMA(&uart3_handler,Send_Data_buff,packet_idx+Sig_Size);
		HAL_UART_Transmit(&uart3_handler, (uint8_t *)Send_Data_buff, packet_idx+Sig_Size, 500);
		
	}else if(Nodes_N>8 && Nodes_N<=16){
		while(1);
	}
	
	
}

void ABA_NACK_handler(){

}

void RBC_INIT_handler(){}

void RBC_ER_handler(){}

void ENC_handler(){}

void PACKET_handler(){
	ttt = ttt + 1;
	delay_ms(20);
//	ttt = ttt + 1;
//	HAL_UART_DMAStop(&uart3_handler);
//	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//开启DMA传输
//	return;
//	if(ReceBuff[0] == 0x00||ReceBuff[0] > Nodes_N || ReceBuff[1] > 20 || ReceBuff[2] > Nodes_N){
	if(ReceBuff[0] == 0x00||ReceBuff[0] > Nodes_N || ReceBuff[1] > 20){
		mmm++;
		HAL_UART_DMAStop(&uart3_handler);
		HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//开启DMA传输
		return;
	}
	nnn++;
	if(Nodes_N > 8)while(1);
	
	//现在是一个一个处理的
	//之后修改为接收多个，
	
	for(int i=0;i<300;i++)buff[i] = ReceBuff[i];
	//验证签名
	u8 len = DMA_buff_Size - UART3RxDMA_Handler.Instance->NDTR;
	if(len != 64 && len != 89) xxx++;
	u8 s_id = buff[0];
	u8 packet_type = buff[1];
	uint8_t hash[32] = {0};
	uint8_t sig[64] = {0};
	u8 verify_flag = 1;
	
	//sha2(buff,len-Sig_Size,hash,0);
	hash256 sh256;
	HASH256_init(&sh256);
	for (int i=0;i<len-Sig_Size;i++) HASH256_process(&sh256,buff[i]);
	HASH256_hash(&sh256,hash);
	
	int tmp_idx=0;
	for(int i=len-Sig_Size;i<len;i++){
		sig[tmp_idx] = buff[i];
		tmp_idx = tmp_idx + 1;
	}
	if (!uECC_verify(public_key[s_id], hash, 32, sig, uECC_secp160r1())) {
		verify_flag = 0;
	}
	
	if(packet_type == TypeSTATE && verify_flag){
		u8 padding = buff[2];
		
		if(s_id == 1 && !aba_id_flag){
			aba_id = buff[3];
			aba_id_flag = 1;
		}
		if(!aba_id_flag)return;
		if(buff[4] != 2)return;
		
		u8 packet_idx = 3+padding;
		u8 round_number = buff[packet_idx];
		packet_idx++;
		//NACK
		u8 round_flag = 0;
		for(int o=0;o<round_number;o++){
			round_flag = 0;
			u8 round = buff[packet_idx];packet_idx++;
			for(int k=1;k<=parallel_D;k++){
				
				//BVAL[0]
				if(((buff[packet_idx + (ID - 1) * 2] & (1<<(k-1))) == 0) && BVAL[k][round][0]==1){
					round_flag = 1;
					break;
				}
				//BVAL[1]
				if(((buff[packet_idx + (ID - 1) * 2 + 1] & (1<<(k-1))) == 0) && BVAL[k][round][1]==1){
					round_flag = 1;
					break;
				}
				
				//AUX[0]
				if(((buff[packet_idx + (ID - 1) * 2 + 2 * Nodes_N] & (1<<(k-1))) == 0) && AUX[k][round][0]==1){
					round_flag = 1;
					break;
				}
				//AUX[1]
				if(((buff[packet_idx + (ID - 1) * 2 + 1 + 2 * Nodes_N] & (1<<(k-1))) == 0) && AUX[k][round][1]==1){
					round_flag = 1;
					break;
				}
			}
			//share_nack
			packet_idx += 4 * Nodes_N;
			if((buff[packet_idx] & (1<<(ID-1))) == 0 && ACK_ABA_share[round][ID] == 1)round_flag = 1;	
			
			packet_idx++;
			if(round_flag){
				STATE_Set[0] = round;
				STATE_Set_Number=1;
				break;
			}
		}
		
		if(STATE_Set_Number)
			STATE_Set_Number = 1;
		
		if(STATE_Set_Number!=0)
			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//接收任务通知的任务句柄
						(uint32_t		)EVENT_ABA_STATE,			//要更新的bit
						(eNotifyAction	)eSetBits);				//更新指定的bit
		
		//STATE
		packet_idx = 4+padding;
		for(int o=0;o<round_number;o++){
			round_flag = 0;
			u8 round = buff[packet_idx];packet_idx++;
//		    for(int i=0;i<round_number;i++){ 
			//u8 round = buff[packet_idx+1];
			
			u32 tmpp_packet_idx = packet_idx;
			
			packet_idx += 4 * Nodes_N;
			packet_idx ++;
			
			if(!ACK_ABA_share[round][s_id]){
				//验证share 复杂 耗时大
				u8 share[70];
				int tmp_packet_idx = packet_idx;
				for(int i=0;i<Share_Size;i++)share[i] = 0x00;
				//share
				for(int i=0;i<Share_Size;i++)share[i] = buff[tmp_packet_idx+i];
				tmp_packet_idx+=Share_Size;
				//Z
				for(int i=0;i<BIG_Size;i++)ABA_Z_share[round][s_id][i] = buff[tmp_packet_idx+i];
				BIG_160_28_fromBytes(ABA_Z[round][s_id],ABA_Z_share[round][s_id]);
				tmp_packet_idx+=BIG_Size;
				//C
				for(int i=0;i<BIG_Size;i++)ABA_C_share[round][s_id][i] = buff[tmp_packet_idx+i];
				BIG_160_28_fromBytes(ABA_C[round][s_id],ABA_C_share[round][s_id]);
				if(Coin_PK_verify_sig_share(s_id,round,share)){
					for(int i=0;i<Share_Size;i++)ABA_share[round][s_id][i] = buff[packet_idx+i];
					ABA_share_Number[round]++;
					ACK_ABA_share[round][s_id] = 1;
					if(ABA_share_Number[round] >= Nodes_N){
						//收到f+1个coin，进行合成
						if(Shared_Coin[round]==0xFF)
							Coin_PK_sig_combine_share(round);//内部会验证
					}
				}else{
					while(1){
						sprintf((char*)Send_Data_buff,"Node %d verify sig error",s_id);
						LoRa_SendData(Send_Data_buff);
					}
				}
			}
			packet_idx += Share_Size + BIG_Size*2;
			
			for(int k=1;k<=parallel_D;k++){//ABA
				
				if(ABA_R[k]>round) continue;
				
				if((buff[tmpp_packet_idx + (s_id - 1) * 2] & (1<<(k-1))) && !ACK_BVAL[k][round][s_id][0]){ACK_BVAL[k][round][s_id][0] = 1;ACK_BVAL_Number[k][round][0]++;}
				if((buff[tmpp_packet_idx + (s_id - 1) * 2 + 1] & (1<<(k-1))) && !ACK_BVAL[k][round][s_id][1]){ACK_BVAL[k][round][s_id][1] = 1;ACK_BVAL_Number[k][round][1]++;}
				if((buff[tmpp_packet_idx + (s_id - 1) * 2 + 2 * Nodes_N] & (1<<(k-1))) && !ACK_AUX[k][round][s_id][0]){ACK_AUX[k][round][s_id][0] = 1;ACK_AUX_Number[k][round][0]++;}
				if((buff[tmpp_packet_idx + (s_id - 1) * 2 + 1 + 2 * Nodes_N] & (1<<(k-1))) && !ACK_AUX[k][round][s_id][1]){ACK_AUX[k][round][s_id][1] = 1;ACK_AUX_Number[k][round][1]++;}
				
				//收到f+1个BVAL(b)
				if(ACK_BVAL_Number[k][round][0] >= Nodes_f + 1 && !BVAL[k][round][0]){ACK_BVAL_Number[k][round][0]++;BVAL[k][round][0] = 1;}
				if(ACK_BVAL_Number[k][round][1] >= Nodes_f + 1 && !BVAL[k][round][1]){ACK_BVAL_Number[k][round][1]++;BVAL[k][round][1] = 1;}
				//收到2f+1个BVAL(b)
				if(ACK_BVAL_Number[k][round][0] >= 2 * Nodes_f + 1){bin_values[k][round][0] = 1;}
				if(ACK_BVAL_Number[k][round][1] >= 2 * Nodes_f + 1){bin_values[k][round][1] = 1;}
				
				//bin_values_R非空  自己发送AUX，记录number
				if(bin_values[k][round][0] && !AUX[k][round][0]){ACK_AUX_Number[k][round][0]++;AUX[k][round][0] = 1;}
				if(bin_values[k][round][1] && !AUX[k][round][1]){ACK_AUX_Number[k][round][1]++;AUX[k][round][1] = 1;}
				
				//收到N-f个AUX_r(b)   广播BVAL内
				if(ACK_AUX_Number[k][round][0] >= (Nodes_N - Nodes_f) && !BVAL[k][round][0]){ACK_BVAL_Number[k][round][0]++;BVAL[k][round][0] = 1;}
				if(ACK_AUX_Number[k][round][1] >= (Nodes_N - Nodes_f) && !BVAL[k][round][1]){ACK_BVAL_Number[k][round][1]++;BVAL[k][round][1] = 1;}
				//收到N-f个AUX_r(b)   收集到vals内	
				if(ACK_AUX_Number[k][round][0] >= (Nodes_N - Nodes_f))vals[k][round][0] = 1;
				if(ACK_AUX_Number[k][round][1] >= (Nodes_N - Nodes_f))vals[k][round][1] = 1;
				
				if(ACK_AUX_Number[k][round][0] > 0)vals[k][round][0] = 1;
				if(ACK_AUX_Number[k][round][1] > 0)vals[k][round][1] = 1;	
				
				u8 aux_flag = 0;
				if(ACK_AUX_Number[k][round][0] + ACK_AUX_Number[k][round][1] >= (Nodes_N - Nodes_f))aux_flag = 1;
					
				//vals是bin_values的子集
				if(aux_flag && subset_or_not(k,round) && ABA_R[k] == round){
					//shared coin
					//到这里一定会获得了N-f的share，因为是绑定到一起的
					//获取round的coin
					//暂定为1，需要合成
					if(ABA_share_Number[round] < Nodes_f + 1){
						while(1);
					}
					u8 s = 0;
					if(Shared_Coin[round] == 0xFF)
						Coin_PK_sig_combine_share(round);
					s = Shared_Coin[round];
					
					if(run_round_flag)s = 0;
					if(vals[k][round][0] == 1 && vals[k][round][1] == 0){
						est[k][round+1] = 0;
						BVAL[k][round+1][0] = 1;
						ACK_BVAL_Number[k][round+1][0]++;
						ABA_round_time[k][round] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
						ABA_R[k]++;
						if(s == 0){
							//ABA j done output 0
							if(ABA_decided[k] == 0xFF){
								ABA_decided[k] = 0;
								ABA_done_Number++;
								ABA_time[k] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
								ABA_round[k] = round;
								if(ABA_done_Number == parallel_D){
									if(ABA_done_Number == parallel_D){
										if(ID == 1 && ABA_done_Number == parallel_D && MY_Consensus_TIME==0){
											LoRa_CFG.chn = 77;
											LoRa_Set();	
											
											MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
											sprintf((char*)Send_Data_buff,"ABA-thres-%d-%d",parallel_D,MY_Consensus_TIME);
											LoRa_SendData(Send_Data_buff);
											while(1);
										}
									}
								}
							}
							else if (ABA_decided[k] == 1){
								//error
								LED0(1);LED1(1);LED2(1);
//								LoRa_CFG.chn = Msg_channel;LoRa_Set();
//								sprintf((char*)Send_Data_buff,"Node %d error",ID);
//								LoRa_SendData(Send_Data_buff);
								while(1);
							}	
						}
					}
					if(vals[k][round][0] == 0 && vals[k][round][1] == 1){
						est[k][round+1] = 1;
						BVAL[k][round+1][1] = 1;
						ACK_BVAL_Number[k][round+1][1]++;
						ABA_R[k]++;
						ABA_round_time[k][round] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
						if(run_round_flag && ID == 1 && k == Nodes_N && round + 1 == run_round){
							LoRa_CFG.chn = 66;
							LoRa_Set();	
							for(int r=1 ;r <= round;r++){
								MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
								sprintf((char*)Send_Data_buff,"%d,%d,%d,%d",ABA_round_time[1][r],ABA_round_time[2][r],ABA_round_time[3][r],ABA_round_time[4][r]);
								LoRa_SendData(Send_Data_buff);
								delay_ms(500);
							}
							sprintf((char*)Send_Data_buff,"...");
							LoRa_SendData(Send_Data_buff);
							
							while(1);
						}
						if(s == 1){
							//ABA j done output 1
							if(ABA_decided[k] == 0xFF){
								ABA_decided[k] = 1;
								ABA_time[k] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
								ABA_round[k] = round;
								ABA_done_Number++;
								if(ABA_done_Number == parallel_D){
									if(ID == 1 && ABA_done_Number == parallel_D && MY_Consensus_TIME==0){
										LoRa_CFG.chn = 77;
										LoRa_Set();	
										
										MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
										sprintf((char*)Send_Data_buff,"ABA-thres-%d-%d",parallel_D,MY_Consensus_TIME);
										LoRa_SendData(Send_Data_buff);
										while(1);
									}
								}
							}
							else if (ABA_decided[k] == 0){
								//error
								LED0(1);LED1(1);LED2(1);
//								LoRa_CFG.chn = Msg_channel;LoRa_Set();
//								sprintf((char*)Send_Data_buff,"Node %d error",ID);
//								LoRa_SendData(Send_Data_buff);
								while(1);
							}
						}
					}	
					if(vals[k][round][0] == 1 && vals[k][round][1] == 1){
						est[k][round+1] = s;
						BVAL[k][round+1][s] = 1;
						ABA_R[k]++;
					}
				}
			}
//			packet_idx += 4 * Nodes_N;
//			packet_idx ++;
//			
//			if(!ACK_ABA_share[round][s_id]){
//				//验证share 复杂 耗时大
//				u8 share[70];
//				int tmp_packet_idx = packet_idx;
//				for(int i=0;i<Share_Size;i++)share[i] = 0x00;
//				//share
//				for(int i=0;i<Share_Size;i++)share[i] = buff[tmp_packet_idx+i];
//				tmp_packet_idx+=Share_Size;
//				//Z
//				for(int i=0;i<BIG_Size;i++)ABA_Z_share[round][s_id][i] = buff[tmp_packet_idx+i];
//				BIG_160_28_fromBytes(ABA_Z[round][s_id],ABA_Z_share[round][s_id]);
//				tmp_packet_idx+=BIG_Size;
//				//C
//				for(int i=0;i<BIG_Size;i++)ABA_C_share[round][s_id][i] = buff[tmp_packet_idx+i];
//				BIG_160_28_fromBytes(ABA_C[round][s_id],ABA_C_share[round][s_id]);
//				if(Coin_PK_verify_sig_share(s_id,round,share)){
//					for(int i=0;i<Share_Size;i++)ABA_share[round][s_id][i] = buff[packet_idx+i];
//					ABA_share_Number[round]++;
//					ACK_ABA_share[round][s_id] = 1;
//					if(ABA_share_Number[round] >= Nodes_N){
//						//收到f+1个coin，进行合成
//						if(Shared_Coin[round]==0xFF)
//							Coin_PK_sig_combine_share(round);//内部会验证
//					}
//				}else{
//					while(1){
//						sprintf((char*)Send_Data_buff,"Node %d verify sig error",s_id);
//						LoRa_SendData(Send_Data_buff);
//					}
//				}
//			}
//			packet_idx += Share_Size + BIG_Size*2;
//		}                              
		}
	}
	
	
//	for(int i=0;i<300;i++)ReceBuff[i] = 0x00;
	HAL_UART_DMAStop(&uart3_handler);
////	HAL_DMA_Init(&UART3RxDMA_Handler);
	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//开启DMA传输
	
	
}










///




