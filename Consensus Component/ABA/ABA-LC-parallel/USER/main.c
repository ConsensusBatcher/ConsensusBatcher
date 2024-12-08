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
#define USE_STM32F7XX_NUCLEO_144

#include "mbedtls/aes.h"

u32 Total_low_time = 0;
u8 low_flag = 0;
u32 low_time[400] = {0};
u32 wfi_time[400] = {0};
u32 low_IDX = 0;

u32 MY_TIME=0;

u8 need_er = 0;
u8 start_ABA = 0;

u32 TIM5_Exceed_Times;
u32 TIM2_Exceed_Times;
u32 MY_Consensus_TIME = 0;

u8 run_round_flag = 0;
u8 run_round = 600;

u8 parallel_D = 4;

/****************ABA_Shared***************************************/

//u8 ABA_share[MAX_round][MAX_Nodes][Share_Size] = {0};//��x�ֵ��ĸ��ڵ��share
//u8 ACK_ABA_share[MAX_round][MAX_Nodes] = {0};//��x�ֵ��ĸ��ڵ��share�յ���
//u8 ABA_share_Number[MAX_round];//��x��share����
//u8 Shared_Coin[MAX_round];//��x�ֵ�coin //FF->NULL
//u8 COIN[Share_Size];
//u8 ABA_sig[MAX_round][Share_Size];

//u8 est[MAX_ABA][MAX_round];//��x�ֵĹ���ֵ
//u8 bin_values[MAX_ABA][MAX_round][2];
////BVAL
//u8 BVAL[MAX_ABA][MAX_round][2];//������BVAL
//u8 ACK_BVAL[MAX_ABA][MAX_round][MAX_Nodes][2];//��x���ĸ��ڵ��0/1�յ���
//u8 ACK_BVAL_Number[MAX_ABA][MAX_round][2];//��x�ֵ�0/1���յ��˶��ٸ�

////AUX
//u8 AUX[MAX_ABA][MAX_round][2];//������AUX
//u8 vals[MAX_ABA][MAX_round][2];//�յ���AUX
//u8 ACK_AUX[MAX_ABA][MAX_round][MAX_Nodes][2];//��x���ĸ��ڵ��0/1�յ���
//u8 ACK_AUX_Number[MAX_ABA][MAX_round][2];//��x�ֵ�0/1���յ��˶��ٸ�

/****************ABA_Local*********************************/
u32 TIM5_Exceed_Times;
u32 TIM2_Exceed_Times;


u8 Local_ABA_votes[MAX_ABA][MAX_round][Local_ABA_phase_Number][2];
u8 Local_ABA_votes_3_NULL[MAX_ABA][MAX_round];//�ڼ���ABA�ڼ��ֵ����׶ι��м�����ֵ
u8 ACK_Local_ABA_votes[MAX_ABA][MAX_round][Local_ABA_phase_Number];
//u8 ACK_Local_ABA_votes[MAX_ABA][MAX_round][Local_ABA_phase_Number][MAX_Nodes];

//RBC initial
u8 Local_ABA_RBC_Init[MAX_ABA][MAX_round][Local_ABA_phase_Number][MAX_Nodes];//ABA-��-�׶�-N��RBC:0/1/3(NULL)/ff��δ�յ���
//u8 ACK_Local_ABA_RBC_Init[MAX_ABA][MAX_round][Local_ABA_phase_Number][MAX_Nodes];

//RBC echo
u8 Local_ABA_RBC_Echo[MAX_ABA][MAX_round][Local_ABA_phase_Number];//ABA-��-�׶�-ÿ�����ر�ʾ�Ƿ񷢳�echo(1/0)
u8 Local_ABA_RBC_Echo_Number[MAX_ABA][MAX_round][Local_ABA_phase_Number][MAX_Nodes];//ABA-��-�׶�-N��RBC�ռ�����
u8 ACK_Local_ABA_RBC_Echo[MAX_ABA][MAX_round][Local_ABA_phase_Number][MAX_Nodes];//ABA-��-�׶�-N��RBC�����ÿ���ڵ㣩

//RBC ready
u8 Local_ABA_RBC_Ready[MAX_ABA][MAX_round][Local_ABA_phase_Number];//ABA-��-�׶�-ÿ�����ر�ʾ�Ƿ񷢳�ready(1/0)
u8 Local_ABA_RBC_Ready_Number[MAX_ABA][MAX_round][Local_ABA_phase_Number][MAX_Nodes];//ABA-��-�׶�-N��RBC�ռ�����
u8 ACK_Local_ABA_RBC_Ready[MAX_ABA][MAX_round][Local_ABA_phase_Number][MAX_Nodes];//ABA-��-�׶�-N��RBC�����ÿ���ڵ㣩

//est and vset
u8 Local_ABA_est[MAX_ABA][MAX_round][Local_ABA_phase_Number];
u8 Local_ABA_vset[MAX_ABA][MAX_round][Local_ABA_phase_Number][2];

u8 ABA_round[MAX_ABA];//decide round
u8 ABA_R[MAX_ABA];//current round
//u8 ABA_phase[MAX_ABA];


//���ͼ���Round ��STATE��
u8 STATE_Set[MAX_Nodes];
u8 STATE_Set_Number;
//���ͼ���Round ��NACK��
u8 NACK_Set[MAX_Nodes];
u8 NACK_Set_Number;

u8 ABA_decided[MAX_ABA];//FF->NULL
u8 ABA_done_Number;
u32 ABA_time[MAX_ABA];
u8 ABA_round[MAX_ABA];
u8 ABA_R[MAX_ABA];
u32 ABA_round_time[5][8];
//u8 RBC_init_Number = 0;
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
u8 tmp_buff[3000];
void ENC_handler();
void Tx_init();
/****************************************************************/

u8 tmp_block[Block_Size];
u8 tmp_hash[Hash_Size];
/************************************************
 ALIENTEK ������STM32F767������ FreeRTOSʵ��2-1
 FreeRTOS��ֲʵ��-HAL��汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		10240
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define CSMACA_PBFT_TASK_PRIO		2
//�����ջ��С	
#define CSMACA_PBFT_STK_SIZE 		256  
//������
TaskHandle_t CSMACA_PBFT_Task_Handler;
//������
void CSMACA_PBFT(void *pvParameters);

//�������ȼ�
#define EVENTSETBIT_TASK_PRIO	3
//�����ջ��С	
#define EVENTSETBIT_STK_SIZE 	512
//������
TaskHandle_t EventSetBit_Handler;
//������
void eventsetbit_task(void *pvParameters);

//�������ȼ�
#define EVENTGROUP_TASK_PRIO	3
//�����ջ��С	
#define EVENTGROUP_STK_SIZE 	1024*4
//������
TaskHandle_t EventGroupTask_Handler;
//������
void eventgroup_task(void *pvParameters);

u8 buff[300];

void init(){
	while(LoRa_Init())//?????LORA???
	{
		delay_ms(300);
	}
	LoRa_CFG.chn = 20;
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
	//CPU_CACHE_Enable();                 //��L1-Cache  DMA
    HAL_Init();				        //��ʼ��HAL��
	SystemClock_Config();
	
    delay_init(216);                //��ʱ��ʼ��
    LED_Init();                     //��ʼ��LED
	
    //������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
	init();
	init_public_key();
	ABA_init();
	//test_thres();
	TIM5_Exceed_Times = 0;
	TIM2_Exceed_Times = 0;
	btim_tim5_int_init(9000-1,10800-1);//108Mhz/10800 = 10000hz  900ms
	btim_tim2_int_init(9000-1,10800-1);
	
    taskENTER_CRITICAL();           //�����ٽ���
//    //����LED0����
//    xTaskCreate((TaskFunction_t )CSMACA_PBFT,     	
//                (const char*    )"csmaca_pbft",   	
//                (uint16_t       )CSMACA_PBFT_STK_SIZE, 
//                (void*          )NULL,					
//                (UBaseType_t    )CSMACA_PBFT_TASK_PRIO,	
//                (TaskHandle_t*  )&CSMACA_PBFT_Task_Handler);   
    
	//���������¼�λ������
    xTaskCreate((TaskFunction_t )eventsetbit_task,             
                (const char*    )"eventsetbit_task",           
                (uint16_t       )EVENTSETBIT_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EVENTSETBIT_TASK_PRIO,        
                (TaskHandle_t*  )&EventSetBit_Handler);   	
    //�����¼���־�鴦������
    xTaskCreate((TaskFunction_t )eventgroup_task,             
                (const char*    )"eventgroup_task",           
                (uint16_t       )EVENTGROUP_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EVENTGROUP_TASK_PRIO,        
                (TaskHandle_t*  )&EventGroupTask_Handler);     
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
	
	
	btim_tim5_enable(1);			
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//LED0������ 
void CSMACA_PBFT(void *pvParameters)
{
	//btim_tim5_int_init(9000-1,10800-1);//108Mhz/10800 = 10000hz  900ms
	//btim_tim2_int_init(9000-1,10800-1);
	//btim_tim9_int_init(9000-1,21600-1);//216*1000,000hz/21600 = 10000hz  500ms
	//tendermint2();
	vTaskSuspend(CSMACA_PBFT_Task_Handler);
}   


//�����¼�λ������
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
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//��������֪ͨ��������
//						(uint32_t		)EVENTBIT_1,			//Ҫ���µ�bit
//						(eNotifyAction	)eSetBits);				//����ָ����bit
			
			vTaskDelay(2000);
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//��������֪ͨ��������
//						(uint32_t		)EVENT_ABA_NACK,			//Ҫ���µ�bit
//						(eNotifyAction	)eSetBits);				//����ָ����bit
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//��������֪ͨ��������
//						(uint32_t		)EVENTBIT_2,			//Ҫ���µ�bit
//						(eNotifyAction	)eSetBits);				//����ָ����bit
			vTaskDelay(1000);	
		}

	}
}

//�¼���־�鴦������
void eventgroup_task(void *pvParameters)
{
	u8 num=0,enevtvalue;
	static u8 event0flag,event1flag,event2flag;
	uint32_t NotifyValue;
	BaseType_t err;
	
	while(1)
	{
		//��ȡ����ֵ֪ͨ
		err=xTaskNotifyWait((uint32_t	)0x00,				//���뺯����ʱ���������bit
							(uint32_t	)ULONG_MAX,			//�˳�������ʱ��������е�bit
							(uint32_t*	)&NotifyValue,		//��������ֵ֪ͨ
							(TickType_t	)portMAX_DELAY);	//����ʱ��
		
		if(err==pdPASS)	   //����֪ͨ��ȡ�ɹ�
		{
			if((NotifyValue&EVENTBIT_0)!=0)			//�¼�0����	
			{
				for(int i=0;i<300;i++)ReceBuff[i] = 0x00;
			}				
			else if((NotifyValue&EVENTBIT_1)!=0)	//�¼�1����	
			{
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//��������֪ͨ��������
//						(uint32_t		)EVENT_PACKET,			//Ҫ���µ�bit
//						(eNotifyAction	)eSetBits);				//����ָ����bit
//				delay_ms(300);
//				btim_tim2_enable(1);
//				for(int i=0;i<100;i++)buff[i] = OkReceBuff[OkIdx][i];
//				sprintf((char*)Send_Data_buff,"STATE: Get EVENT 1.");LoRa_SendData(Send_Data_buff);
			}
			else if((NotifyValue&EVENTBIT_2)!=0)	//�¼�2����	
			{
				for(int i=0;i<300;i++)ReceBuff[i] = 0x00;
				//sprintf((char*)Send_Data_buff,"STATE: Get EVENT 2.");LoRa_SendData(Send_Data_buff);	
			}
			else if((NotifyValue&EVENT_ABA_STATE)!=0)	//STATE����	
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

/********************************Multi-Hop************************************************/
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

int Tx_encrypt(u8 *key, u8 *raw, int raw_len, u8 *enc){
	int pad_len = raw_len;// = PAD(raw, raw_len);
	u8 iv[16];
	u8 iv2[16];
	for(u8 i=0;i<16;i++){
		iv[i] = i+1;//RNG_Get_RandomNum();
		iv2[i] = iv[i];
	}
	
	mbedtls_aes_context ctx;
	
	// ����
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
	// ����
	mbedtls_aes_setkey_dec(&ctx, (unsigned char *)key, 128);
	mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, enc_len-16, iv, enc, dec);
}
void Tx_init(){}
/*************************************************************************************/

void RBC_init(){}

void ABA_init(){
	start_ABA = 0;
	ABA_done_Number = 0;

		for(int i=0;i<MAX_ABA;i++){
		ABA_decided[i] = 0xFF;//0xFF->NULL or decided on 1/0

		for(int j=0;j<MAX_round;j++){
			Local_ABA_votes_3_NULL[i][j] = 0;
			for(int k=0;k<Local_ABA_phase_Number;k++){
				Local_ABA_est[i][j][k] = 0xFF;
				Local_ABA_vset[i][j][k][0] = 0;
				Local_ABA_vset[i][j][k][1] = 0;
				
				Local_ABA_votes[i][j][k][0] = 0;
				Local_ABA_votes[i][j][k][1] = 0;
				
				Local_ABA_RBC_Echo[i][j][k] = 0;
				Local_ABA_RBC_Ready[i][j][k] = 0;
				
				ACK_Local_ABA_votes[i][j][k] = 0;
				for(int m=0;m<MAX_Nodes;m++){
					
					
					Local_ABA_RBC_Init[i][j][k][m] = 0xFF;
					//ACK_Local_ABA_RBC_Init[i][j][k][m] = 0xFF;
					
					Local_ABA_RBC_Echo_Number[i][j][k][m] = 0;
					
					Local_ABA_RBC_Ready_Number[i][j][k][m] = 0;
					ACK_Local_ABA_RBC_Echo[i][j][k][m] = 0;
					ACK_Local_ABA_RBC_Ready[i][j][k][m] = 0;
					for(int n=0;n<MAX_Nodes;n++){
						
					}
				}
			}
		}
	}
	//for round 1
	for(int i=1;i<=Nodes_N;i++){
		
//		if(ID <= 1){
//			Local_ABA_RBC_Init[i][1][1][ID] = 0;
//			Local_ABA_est[i][1][1] = 0;
//		}else{
//			Local_ABA_RBC_Init[i][1][1][ID] = 1;
//			Local_ABA_est[i][1][1] = 1;
//		}
		u8 vote = RNG_Get_RandomNum() % 2;
		
		Local_ABA_RBC_Init[i][1][1][ID] = vote;
		Local_ABA_est[i][1][1] = vote;
		
		
		Local_ABA_RBC_Echo_Number[i][1][1][ID] = 1;
		Local_ABA_RBC_Echo[i][1][1] |= (1<<(ID-1));
		
		Local_ABA_vset[i][1][1][0] = 1;
		Local_ABA_vset[i][1][1][1] = 1;
	}
	for(int i=0;i<MAX_ABA;i++)ABA_R[i] = 1;

	for(int i=0;i<MAX_Nodes;i++)STATE_Set[i]=0;
	STATE_Set_Number = 1;
	
	for(int i=0;i<MAX_ABA;i++)
		ABA_decided[i] = 0xFF;

	TIM5_Exceed_Times = 0;
	TIM5->CNT = 0;
	
	btim_tim5_enable(1);
	
	
	MYDMA_Config_Rece(DMA1_Stream1,DMA_CHANNEL_4);//��ʼ��DMA

	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);
	//init_thres_sig();

	
}
u8 ttt = 0;
u8 xxx = 0;
u8 mmm = 0;
u8 nnn = 0;
int subset_or_not(u8 aba, u8 round){

}


void ABA_STATE_handler(){
	//DMA send STATE packet
	//ֻʵ����16�ڵ����µ��������������۵�
	
//	//�����ʱ��������Ϣ���͹���
//	u32 random = RNG_Get_RandomRange(0, 5);
//	random = random * 100;
//	delay_ms(random);
	
	for(int i=0;i<300;i++)Send_Data_buff[i] = 0x00;
	if(Nodes_N<=8){
		Send_Data_buff[0] = ID;
		Send_Data_buff[1] = TypeSTATE;
		u8 padding = 20;
		if(parallel_D < 4)padding += 20*(4-parallel_D);
		Send_Data_buff[2] = padding;
		for(int i=0;i<padding;i++)Send_Data_buff[3+i] = 0xFF;
		Send_Data_buff[3+padding] = STATE_Set_Number;
		u8 packet_idx = 3+padding+1;
		for(int i=0;i<STATE_Set_Number;i++){
			Send_Data_buff[packet_idx] = STATE_Set[i];//round 
			for(int a=1;a<=parallel_D;a++){//ABA number
				for(int j=1;j<=3;j++){//phase number
					//init
					for(int k=1;k<=Nodes_N;k++){//RBC number
						if(Local_ABA_RBC_Init[a][STATE_Set[i]][j][k] != 0xFF){
							Send_Data_buff[packet_idx+1+6*(j-1)] |= (1<<(k-1));	//ACK_init
							if(Local_ABA_RBC_Init[a][STATE_Set[i]][j][k])
								Send_Data_buff[packet_idx+2+6*(j-1)] |= (1<<(k-1)); 	//init
							if(j == 3 && Local_ABA_RBC_Init[a][STATE_Set[i]][j][k] == 3){
								Send_Data_buff[packet_idx+1+6*(j-1)] &= ~(1<<(k-1));//0
								Send_Data_buff[packet_idx+2+6*(j-1)] |= (1<<(k-1));//1
							}
						}
						
						if(Local_ABA_RBC_Echo_Number[a][STATE_Set[i]][j][k] >= Nodes_N-Nodes_f){
							Send_Data_buff[packet_idx+3+6*(j-1)] |= (1<<(k-1)); 	//ACK_echo
//							Send_Data_buff[packet_idx+5+6*(j-1)] |= (1<<(k-1)); 	//ACK_ready
						}
						
						if((Local_ABA_RBC_Echo[a][STATE_Set[i]][j] & (1<<(k-1))) != 0)
							Send_Data_buff[packet_idx+4+6*(j-1)] |= (1<<(k-1)); 	//echo
						
						if(Local_ABA_RBC_Ready_Number[a][STATE_Set[i]][j][k] >= 2 * Nodes_f+1){
//							Send_Data_buff[packet_idx+3+6*(j-1)] |= (1<<(k-1)); 	//ACK_echo
							Send_Data_buff[packet_idx+5+6*(j-1)] |= (1<<(k-1)); 	//ACK_ready
						}	
						
						if((Local_ABA_RBC_Ready[a][STATE_Set[i]][j] & (1<<(k-1))) != 0)	
							Send_Data_buff[packet_idx+6+6*(j-1)] |= (1<<(k-1)); 	//ready
					}
				}
				packet_idx = packet_idx + 3 * 6;
			}
		}
		packet_idx++;
		//Sign
		uint8_t hash[32] = {0};
		uint8_t sig[64] = {0};
		sha2(Send_Data_buff,packet_idx,hash,0);
		sign_data(hash,sig);
		for(int i=0;i<Sig_Size;i++)Send_Data_buff[packet_idx+i] = sig[i];
		
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
//	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//����DMA����
//	return;
	//if(ReceBuff[0] == 0x00||ReceBuff[0] > Nodes_N || ReceBuff[1] > 20 || ReceBuff[2] > Nodes_N){
	if(ReceBuff[0] == 0x00||ReceBuff[0] > Nodes_N || ReceBuff[1] > 20){
		mmm++;
		HAL_UART_DMAStop(&uart3_handler);
		HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//����DMA����
		return;
	}
	nnn++;
	if(Nodes_N > 8)while(1);
	
	//������һ��һ�������
	//֮���޸�Ϊ���ն����
	
	for(int i=0;i<300;i++)buff[i] = ReceBuff[i];
	//��֤ǩ��
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
	
	if(packet_type == TypeSTATE && verify_flag ){
		u8 padding = buff[2];
		u8 reply_flag = 0;
		u8 round_number = buff[3+padding];
		u8 packet_idx = 4 + padding;
		
		u8 min_round = 0xff;for(int i=1;i<=Nodes_N;i++){if(min_round > ABA_R[i])min_round = ABA_R[i];}
		
		for(int i=0;i<round_number;i++){
			u8 round = buff[packet_idx]; 
			if(min_round > round){STATE_Set_Number = 1;STATE_Set[0] = round;}
			for(int a=1;a<=parallel_D;a++){//ABA number
				for(int j=1;j<=3;j++){//phase number
					//init
					u8 init_ACK = buff[packet_idx + 1 + (j - 1) * 6];
					u8 init_VAL = buff[packet_idx + 2 + (j - 1) * 6];
					u8 echo_ACK = buff[packet_idx + 3 + (j - 1) * 6];
					u8 echo_VAL = buff[packet_idx + 4 + (j - 1) * 6];//s_id ��N��RBC�Ƿ񷢳�echo
					u8 ready_ACK = buff[packet_idx + 5 + (j - 1) * 6];
					u8 ready_VAL = buff[packet_idx + 6 + (j - 1) * 6];//s_id ��N��RBC�Ƿ񷢳�ready
					
					//��ʾĿǰ a round j 
					if(Local_ABA_vset[a][round][j][0]==0 && Local_ABA_vset[a][round][j][1]==0)continue;
					
					/**********ACK***********/
					if(j <= 2 && (init_ACK & (1<<(ID-1))) == 0 && Local_ABA_est[a][round][j] != 0xFF) reply_flag = 1;
					if(j == 3 && (init_ACK & (1<<(ID-1))) == 0 && (init_VAL & (1<<(ID-1))) == 0 && Local_ABA_est[a][round][j] != 0xFF) reply_flag = 1;
					for(int k=1;k<=Nodes_N;k++){//RBC number
						if((echo_ACK & (1<<(k-1)))==0 && (Local_ABA_RBC_Echo[a][round][j] & (1<<(k-1))) != 0)reply_flag = 1;	
						if((ready_ACK & (1<<(k-1)))==0 && (Local_ABA_RBC_Ready[a][round][j] & (1<<(k-1))) != 0)reply_flag = 1;
					}
					/**********VAL***********/
					//init
					if((init_ACK & (1<<(s_id-1))) != 0 && Local_ABA_RBC_Init[a][round][j][s_id] == 0xFF){
						if((init_VAL & (1<<(s_id-1))) != 0)
							Local_ABA_RBC_Init[a][round][j][s_id] = 1;
						else Local_ABA_RBC_Init[a][round][j][s_id] = 0;
						//echo
						Local_ABA_RBC_Echo[a][round][j] |= (1<<(s_id-1));
						Local_ABA_RBC_Echo_Number[a][round][j][s_id]++;
						if(Local_ABA_RBC_Echo_Number[a][round][j][s_id] >= Nodes_N - Nodes_f){
							Local_ABA_RBC_Echo[a][round][j] |= (1<<(s_id-1));
							Local_ABA_RBC_Ready[a][round][j] |= (1<<(s_id-1));
							//no need for RBC k Echo
							if(Local_ABA_vset[a][round][j][Local_ABA_RBC_Init[a][round][j][s_id]])
								ACK_Local_ABA_RBC_Echo[a][round][j][s_id] = 0xFF;
						}
					}else if(j == 3 && (init_ACK & (1<<(s_id-1))) == 0 && (init_VAL & (1<<(s_id-1))) != 0 && Local_ABA_RBC_Init[a][round][j][s_id] == 0xFF){
						Local_ABA_RBC_Init[a][round][j][s_id] = 3;
						//echo
						Local_ABA_RBC_Echo[a][round][j] |= (1<<(s_id-1));
						Local_ABA_RBC_Echo_Number[a][round][j][s_id]++;
						
						if(Local_ABA_RBC_Echo_Number[a][round][j][s_id] >= Nodes_N - Nodes_f){
							Local_ABA_RBC_Echo[a][round][j] |= (1<<(s_id-1));
							Local_ABA_RBC_Ready[a][round][j] |= (1<<(s_id-1));
							//no need for RBC k Echo
							if(Local_ABA_vset[a][round][j][0] && Local_ABA_vset[a][round][j][1])
								ACK_Local_ABA_RBC_Echo[a][round][j][s_id] = 0xFF;
						}
					}
					//echo
					for(int k=1;k<=Nodes_N;k++){//RBC number
						if((echo_VAL & (1<<(k-1))) != 0 && (ACK_Local_ABA_RBC_Echo[a][round][j][k] & (1<<(s_id-1))) == 0){
							Local_ABA_RBC_Echo_Number[a][round][j][k]++;
							ACK_Local_ABA_RBC_Echo[a][round][j][k] |= (1<<(s_id-1));
							
							if(Local_ABA_RBC_Echo_Number[a][round][j][k] >= Nodes_N - Nodes_f){
								if((init_VAL & (1<<(k-1))) != 0)
									Local_ABA_RBC_Init[a][round][j][k] = 1;
								else Local_ABA_RBC_Init[a][round][j][k] = 0;
								
								//if(j == 3 && (init_VAL & (1<<(k-1))) != 0)
								if(j == 3 && (init_VAL & (1<<(k-1))) != 0 && (init_ACK & (1<<(k-1))) == 0)
									Local_ABA_RBC_Init[a][round][j][k] = 3;
								
								Local_ABA_RBC_Echo[a][round][j] |= (1<<(k-1));
								Local_ABA_RBC_Ready[a][round][j] |= (1<<(k-1));
								
								//no need for RBC k Echo
								if(j <= 2){
									if(Local_ABA_vset[a][round][j][Local_ABA_RBC_Init[a][round][j][k]])
										ACK_Local_ABA_RBC_Echo[a][round][j][k] = 0xFF;
								}else if(j == 3){
									if(Local_ABA_vset[a][round][j][0] && Local_ABA_vset[a][round][j][1])
										ACK_Local_ABA_RBC_Echo[a][round][j][k] = 0xFF;
								}
								
							}
						
						}
					}

					//ready
					for(int k=1;k<=Nodes_N;k++){//RBC number
						if((ready_VAL & (1<<(k-1))) != 0 && (ACK_Local_ABA_RBC_Ready[a][round][j][k] & (1<<(s_id-1))) == 0){
							Local_ABA_RBC_Ready_Number[a][round][j][k]++;
							ACK_Local_ABA_RBC_Ready[a][round][j][k] |= (1<<(s_id-1));
							if(Local_ABA_RBC_Ready_Number[a][round][j][k] >= Nodes_f + 1){//ready_number >= f + 1
								if((init_VAL & (1<<(k-1))) != 0)
									Local_ABA_RBC_Init[a][round][j][k] = 1;
								else Local_ABA_RBC_Init[a][round][j][k] = 0;
								
								//if(j == 3 && (init_VAL & (1<<(k-1))) != 0)
								if(j == 3 && (init_VAL & (1<<(k-1))) != 0 && (init_ACK & (1<<(k-1))) == 0)
									Local_ABA_RBC_Init[a][round][j][k] = 3;
								
								Local_ABA_RBC_Echo[a][round][j] |= (1<<(k-1));
								Local_ABA_RBC_Ready[a][round][j] |= (1<<(k-1));
							}
							if(Local_ABA_RBC_Ready_Number[a][round][j][k] >= 2 * Nodes_f + 1){//ready_number >= 2*f + 1
								//accept go on phase? round?
								
								//no need for RBC k Ready
								//ACK_Local_ABA_RBC_Ready[a][round][j][k] = 0xFF;
								if(j <= 2){
									if(Local_ABA_vset[a][round][j][Local_ABA_RBC_Init[a][round][j][k]])
										ACK_Local_ABA_RBC_Ready[a][round][j][k] = 0xFF;
								}else if(j == 3){
									if(Local_ABA_vset[a][round][j][0] && Local_ABA_vset[a][round][j][1])
										ACK_Local_ABA_RBC_Ready[a][round][j][k] = 0xFF;
								}
								
								
								if((ACK_Local_ABA_votes[a][round][j] & (1<<(k-1))) == 0){
									ACK_Local_ABA_votes[a][round][j] |= (1<<(k-1));
									if(Local_ABA_RBC_Init[a][round][j][k] == 1){
										//check if in vset
										if(Local_ABA_vset[a][round][j][1])
											Local_ABA_votes[a][round][j][1]++;
									}
									else if(Local_ABA_RBC_Init[a][round][j][k] == 0){
										//check if in vset
										if(Local_ABA_vset[a][round][j][0])
											Local_ABA_votes[a][round][j][0]++;
										
										if(j == 3 && Local_ABA_RBC_Init[a][round][j][k] == 3)
											Local_ABA_votes_3_NULL[a][round]++;
									}
								}
								if(j <= 2 && (Local_ABA_votes[a][round][j][1] + Local_ABA_votes[a][round][j][0] == Nodes_N - Nodes_f)){
									//ͳ����votes�ˣ������յ���N-f
									u8 votes0 = Local_ABA_votes[a][round][j][0];
									u8 votes1 = Local_ABA_votes[a][round][j][1];
									if(j == 1){//phase 1
										if(votes0 == Nodes_N - Nodes_f || votes1 == Nodes_N - Nodes_f ){
											//correct
											u8 vote;
											if(votes0 == Nodes_N - Nodes_f)vote = 0;
											else vote = 1;
											
											//decide
											if(ABA_decided[a] == 0xFF){
												ABA_decided[a] = vote;
												if(vote == 1) { 
//													//decrypt the tx
//													Tx_SK_decrypt_share(a, ID);
//													if(ACK_Share[a][ID] == 0){
//														ACK_Share[a][ID] = 1;
//														Tx_shares_Number[a]++;
//													}
//													dec_done_number++;
												}
												u8 flag_a = 1;
												//for(int xx=1;xx<=Nodes_N;xx++)if(ABA_decided[xx]==0)flag_a=0;
												ABA_done_Number++;if(ABA_done_Number == parallel_D){
													if(ID == 1 && ABA_done_Number == parallel_D && MY_Consensus_TIME==0){
														LoRa_CFG.chn = 66;
														LoRa_Set();	
														
														MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
														sprintf((char*)Send_Data_buff,"aa-%d",MY_Consensus_TIME);
														LoRa_SendData(Send_Data_buff);
														while(1);
													}
												}
											}
											else if(ABA_decided[a] != vote){
												//error decide on different value
												while(1);
											}
											//iv_r+1 = v
											Local_ABA_est[a][round + 1][1] = vote;
											//vset = {v}
											Local_ABA_vset[a][round][2][vote] = 1;
											
											//for next phase
											Local_ABA_est[a][round][2] = vote;
											
											// to next phase RBC
											Local_ABA_RBC_Init[a][round][2][ID] = vote;
											Local_ABA_RBC_Echo_Number[a][round][2][ID] = 1;
											Local_ABA_RBC_Echo[a][round][2] |= (1<<(ID-1));
											
										}else{
											u8 vote;
											if(votes0 > votes1) vote = 0;
											else vote = 1;
											
											//vset = {0,1}
											Local_ABA_vset[a][round][2][0] = 1;
											Local_ABA_vset[a][round][2][1] = 1;
											//for next phase
											Local_ABA_est[a][round][2] = vote;
											
											// to next phase RBC
											Local_ABA_RBC_Init[a][round][2][ID] = vote;
											Local_ABA_RBC_Echo_Number[a][round][2][ID] = 1;
											Local_ABA_RBC_Echo[a][round][2] |= (1<<(ID-1));
										}
									}
									else if(j == 2){//phase 2
										if(votes0 >= (Nodes_N/2) || votes1 >= (Nodes_N/2) ){
											u8 vote;
											if(votes0 >= (Nodes_N/2))vote = 0;
											else vote = 1;
											
											//vset = {v}
											Local_ABA_vset[a][round][3][vote] = 1;
											
											//for next phase
											Local_ABA_est[a][round][3] = vote;
											
											// to next phase RBC
											Local_ABA_RBC_Init[a][round][3][ID] = vote;
											Local_ABA_RBC_Echo_Number[a][round][3][ID] = 1;
											Local_ABA_RBC_Echo[a][round][3] |= (1<<(ID-1));
										}else{
											//vset = {0,1}
											Local_ABA_vset[a][round][3][0] = 1;
											Local_ABA_vset[a][round][3][1] = 1;
											//for next phase
											Local_ABA_est[a][round][3] = 3;//NULL
											
											// to next phase RBC
											Local_ABA_RBC_Init[a][round][3][ID] = 3;
											Local_ABA_RBC_Echo_Number[a][round][3][ID] = 1;
											Local_ABA_RBC_Echo[a][round][3] |= (1<<(ID-1));
										}
									}
								}
								if(j == 3){
									if(Local_ABA_votes[a][round][j][0] + Local_ABA_votes[a][round][j][1] + Local_ABA_votes_3_NULL[i][j] == Nodes_N - Nodes_f){
										u8 votes0 = Local_ABA_votes[a][round][j][0];
										u8 votes1 = Local_ABA_votes[a][round][j][1];
										if(votes0 == 2 * Nodes_f + 1 || votes1 == 2 * Nodes_f + 1){
											u8 vote;
											if(votes0 == 2 * Nodes_f + 1)vote = 0;
											else vote = 1;
											//decide
											if(ABA_decided[a] == 0xFF){
												ABA_decided[a] = vote;
												if(vote == 1) {
//													//decrypt the tx
//													Tx_SK_decrypt_share(a, ID);
//													if(ACK_Share[a][ID] == 0){
//														ACK_Share[a][ID] = 1;
//														Tx_shares_Number[a]++;
//													}
//													dec_done_number++;
												}
												u8 flag_a = 1;
												ABA_done_Number++;if(ABA_done_Number == parallel_D){
													if(ID == 1 && ABA_done_Number == parallel_D && MY_Consensus_TIME==0){
														LoRa_CFG.chn = 66;
														LoRa_Set();	
														
														MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
														sprintf((char*)Send_Data_buff,"bb-%d",MY_Consensus_TIME);
														LoRa_SendData(Send_Data_buff);
														while(1);
													}
												}
											}
											else if(ABA_decided[a] != vote){
												//error decide on different value
												while(1);
											}
											
											//iv_r+1 = v
											Local_ABA_est[a][round + 1][1] = vote;
											//vset = {v}
											Local_ABA_vset[a][round + 1][1][vote] = 1;
											
											// to next round RBC
											Local_ABA_RBC_Init[a][round + 1][1][ID] = vote;
											Local_ABA_RBC_Echo_Number[a][round + 1][1][ID] = 1;
											Local_ABA_RBC_Echo[a][round + 1][1] |= (1<<(ID-1));
											
										}else if(votes0 == Nodes_f + 1 || votes1 == Nodes_f + 1){
											u8 vote;
											if(votes0 == Nodes_f + 1)vote = 0;
											else vote = 1;
											//iv_r+1 = v
											Local_ABA_est[a][round + 1][1] = vote;
											//vset = {0,1}
											Local_ABA_vset[a][round + 1][1][0] = 1;
											Local_ABA_vset[a][round + 1][1][1] = 1;
											
											// to next round RBC
											Local_ABA_RBC_Init[a][round + 1][1][ID] = vote;
											Local_ABA_RBC_Echo_Number[a][round + 1][1][ID] = 1;
											Local_ABA_RBC_Echo[a][round + 1][1] |= (1<<(ID-1));
										}else{
											//iv_r+1 = v
											Local_ABA_est[a][round + 1][1] = RNG_Get_RandomNum() % 2;
											//vset = {0,1}
											Local_ABA_vset[a][round + 1][1][0] = 1;
											Local_ABA_vset[a][round + 1][1][1] = 1;
											
											// to next round RBC
											Local_ABA_RBC_Init[a][round + 1][1][ID] = Local_ABA_est[a][round + 1][1];
											Local_ABA_RBC_Echo_Number[a][round + 1][1][ID] = 1;
											Local_ABA_RBC_Echo[a][round + 1][1] |= (1<<(ID-1));
										}
										if(round == ABA_R[a]){
											ABA_R[a]++;
											ABA_round_time[a][round] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
											if(run_round_flag && ID == 1 && a == Nodes_N && round + 1 == run_round){
												LoRa_CFG.chn = 66;
												LoRa_Set();	
												for(int r=1 ;r <= round;r++){
													MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
													sprintf((char*)Send_Data_buff,"%d-%d-%d-%d",ABA_round_time[1][r],ABA_round_time[2][r],ABA_round_time[3][r],ABA_round_time[4][r]);
													LoRa_SendData(Send_Data_buff);
													delay_ms(500);
												}
												sprintf((char*)Send_Data_buff,"...");
												LoRa_SendData(Send_Data_buff);
												
												while(1);
											}
										}
											
									}
									
								}
							}
						}
					}
				}
				packet_idx = packet_idx + 3 * 6;
			}
		}
		if(reply_flag){
		//if(reply_flag && min_round > STATE_Set[0]){
			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//��������֪ͨ��������
						(uint32_t		)EVENT_ABA_STATE,			//Ҫ���µ�bit
						(eNotifyAction	)eSetBits);				//����ָ����bit
		}
		
	}
	HAL_UART_DMAStop(&uart3_handler);
////	HAL_DMA_Init(&UART3RxDMA_Handler);
	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//����DMA����
	
	
}










///




