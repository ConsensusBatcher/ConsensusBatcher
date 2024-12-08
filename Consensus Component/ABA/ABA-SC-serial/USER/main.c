/************************************************
 ALIENTEK ������STM32F767������ FreeRTOSʵ��2-1
 FreeRTOS��ֲʵ��-HAL��汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
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

u8 run_round_flag = 1;
u8 run_round = 7;

u32 MY_TIME = 0;
u32 MY_Consensus_TIME=0;
/****************ABA_Shared***************************************/
u8 cur_aba_id;
u8 state_aba;
u32 TIM5_Exceed_Times;
u32 TIM2_Exceed_Times;
u8 ABA_share[MAX_ABA][MAX_round][MAX_Nodes][Share_Size] = {0};//��x�ֵ��ĸ��ڵ��share
u8 ACK_ABA_share[MAX_ABA][MAX_round][MAX_Nodes] = {0};//��x�ֵ��ĸ��ڵ��share�յ���
u8 ABA_share_Number[MAX_ABA][MAX_round];//��x��share����
u8 Shared_Coin[MAX_ABA][MAX_round];//��x�ֵ�coin //FF->NULL
u8 COIN[Share_Size];
u8 est[MAX_ABA][MAX_round];//��x�ֵĹ���ֵ
u8 bin_values[MAX_ABA][MAX_round][2];

//BVAL
u8 BVAL[MAX_ABA][MAX_round][2];//������BVAL
u8 ACK_BVAL[MAX_ABA][MAX_round][MAX_Nodes][2];//��x���ĸ��ڵ��0/1�յ���
u8 ACK_BVAL_Number[MAX_ABA][MAX_round][2];//��x�ֵ�0/1���յ��˶��ٸ�

//AUX
u8 AUX[MAX_ABA][MAX_round][2];//������AUX
u8 vals[MAX_ABA][MAX_round][2];//�յ���AUX
u8 ACK_AUX[MAX_ABA][MAX_round][MAX_Nodes][2];//��x���ĸ��ڵ��0/1�յ���
u8 ACK_AUX_Number[MAX_ABA][MAX_round][2];//��x�ֵ�0/1���յ��˶��ٸ�

//���ͼ���Round ��STATE��
u8 STATE_Set[MAX_Nodes];
u8 STATE_Set_Number;
//���ͼ���Round ��NACK��
u8 NACK_Set[MAX_Nodes];
u8 NACK_Set_Number;

u8 ABA_decided[MAX_ABA];//FF->NULL
u32 ABA_time[MAX_ABA];
u8 ABA_round[MAX_ABA];
u8 ABA_R[MAX_ABA];
u32 ABA_round_time[5][8];

u8 ABA_id = 0;
u8 aba_id_flag = 0;

/********************************************************************/


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



/***********************PRBC_Sig*************************************/
u8 ACK_PRBC_share_Number[MAX_Nodes];
u8 ACK_PRBC_share[MAX_Nodes][MAX_Nodes];
u8 PRBC_share[MAX_Nodes][MAX_Nodes][Share_Size];//�ڼ���RBC���ڼ����ڵ��share
u8 PRBC_sig[MAX_Nodes][Share_Size];
u8 PRBC_done_Number;

u8 need_EVENT_RBC_ER = 0;


/****************************************************************/
u8 tmp_block[Block_Size];
u8 tmp_hash[Hash_Size];

/*********************CBC-commit***************************************/

u8 CBC_v[MAX_Nodes][CBC_v_SIZE];
u8 ACK_CBC_Echo[MAX_Nodes];// �Լ���v��Echo
u8 CBC_Echo[MAX_Nodes];	//�������ڵ��Echo
u8 CBC_Echo_Share[MAX_Nodes][Share_Size];
u8 My_CBC_Echo_Share[MAX_Nodes][Share_Size];
u8 My_CBC_Echo_Number;

u8 ACK_CBC_Finish[MAX_Nodes];
u8 CBC_Finish_Share[MAX_Nodes][Share_Size];
u8 CBC_done_Number;

/*******************CBC-value**************************************/
u8 VAL_CBC_v[MAX_Nodes][MAX_Nodes][Share_Size];
u8 ACK_VAL_CBC[MAX_Nodes][MAX_Nodes];
u8 ACK_VAL_CBC_Number[MAX_Nodes];

u8 ACK_VAL_CBC_Echo[MAX_Nodes];// �Լ���v��Echo
u8 VAL_CBC_Echo[MAX_Nodes];	//�������ڵ��Echo
u8 VAL_CBC_Echo_Share[MAX_Nodes][Share_Size];
u8 My_VAL_CBC_Echo_Share[MAX_Nodes][Share_Size];
u8 My_VAL_CBC_Echo_Number;

u8 ACK_VAL_CBC_Finish[MAX_Nodes];
u8 VAL_CBC_Finish_Share[MAX_Nodes][Share_Size];
u8 VAL_CBC_done_Number;

u8 W_arr[2];
/*******************************************************************/

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
u8 RBC_init_Number = 0;
u8 CBC_v_number = 0;
/****************************************************************/

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
#define EVENTSETBIT_STK_SIZE 	256
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

u8 buff[3000];

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


u8 decidd_flag = 1;

void ABA_init();
void ABA_STATE_handler();
//void ABA_NACK_handler();

void RBC_init();
void RBC_INIT_handler();
void RBC_ER_handler();
void PRBC_SIG_handler();

void VAL_CBC_init();
void VAL_CBC_STATE_handler();

void CBC_init();
void CBC_STATE_handler();

void PACKET_handler();

typedef enum
{
	//TypeNACK = 0,
	TypeRBC_INIT,
	TypeRBC_ER,
	TypePRBC_SIG,
	TypeCBC_STATE,
	TypeVAL_CBC_STATE,
	TypeSTATE,
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
	init_thres_enc();
	ABA_init();
	init_thres_sig();
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
//						(uint32_t		)EVENT_CBC_STATE,			//Ҫ���µ�bit
//						(eNotifyAction	)eSetBits);				//����ָ����bit
			
			vTaskDelay(2000);
//			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//��������֪ͨ��������
//						(uint32_t		)EVENT_NACK,			//Ҫ���µ�bit
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
				//sprintf((char*)Send_Data_buff,"STATE: Get EVENT 0.");LoRa_SendData(Send_Data_buff);
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
			else if((NotifyValue & EVENT_STATE)!=0)	//STATE����	
			{
				ABA_STATE_handler();
			}
			else if((NotifyValue&EVENT_PACKET)!=0)	//PACKET	
			{
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

void VAL_CBC_init(){}

void CBC_init(){}

void RBC_init(){}

void ABA_init(){
	cur_aba_id = 1;
	for(int a=0;a<MAX_ABA;a++){
		for(int i=0;i<MAX_round;i++){
			ABA_share_Number[a][i] = 0x00;
			Shared_Coin[a][i] = 0xFF;
			for(int j=0;j<MAX_Nodes;j++){
				for(int k=0;k<Share_Size;k++)ABA_share[a][i][j][k] = 0x00;
				ACK_ABA_share[a][i][j] = 0x00;
			}
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
	
	MYDMA_Config_Rece(DMA1_Stream1,DMA_CHANNEL_4);//��ʼ��DMA
	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);
	
	for(int i=0;i<MAX_ABA;i++)ABA_R[i] = 1;
	
	for(int i=0;i<MAX_ABA;i++)
		for(int j=0;j<MAX_round;j++)
			est[i][j] = 0xFF;
	
	for(int i=0;i<MAX_ABA;i++){
		u8 vote = RNG_Get_RandomNum() % 2;
		est[i][1] = vote;
		BVAL[i][1][vote] = 1;
		ACK_BVAL_Number[i][1][vote] = 1;
	}
		
	for(int i=0;i<MAX_Nodes;i++)STATE_Set[i]=0;
	STATE_Set_Number = 1;
	
	for(int i=0;i<MAX_ABA;i++)
		ABA_decided[i] = 0xFF;
	
	if(ID == 1){
		ABA_id = RNG_Get_RandomNum();
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
	u8 aba_id = state_aba;
	if(Nodes_N<=8){// && ABA_decided[1]==0xFF && ABA_decided[2]==0xFF && ABA_decided[3]==0xFF && ABA_decided[4]==0xFF){
		Send_Data_buff[0] = ID;
		Send_Data_buff[1] = TypeSTATE;
		u8 padding = 50;
		Send_Data_buff[2] = padding;
		for(int i=0;i<padding;i++)Send_Data_buff[3+i] = 0xFF;
		
		if(ID == 1)Send_Data_buff[3] = ABA_id;
		if(aba_id_flag == 1)Send_Data_buff[4] = 2;
		
		u8 packet_idx = 3 + padding;
		Send_Data_buff[packet_idx] = STATE_Set_Number;
		for(int o=0;o<STATE_Set_Number;o++){
			packet_idx++;
			Send_Data_buff[packet_idx] = aba_id;//aba number
			packet_idx++;
			Send_Data_buff[packet_idx] = STATE_Set[o];//round number
			packet_idx++;
			u8 round = STATE_Set[o];
			
			//my BVAL
			if(BVAL[aba_id][round][0])
				Send_Data_buff[packet_idx + 0] |= (1<<(ID-1));
			if(BVAL[aba_id][round][1])
				Send_Data_buff[packet_idx + 1] |= (1<<(ID-1));
			//my AUX
			if(AUX[aba_id][round][0])
				Send_Data_buff[packet_idx + 2] |= (1<<(ID-1));
			if(AUX[aba_id][round][1])
				Send_Data_buff[packet_idx + 3] |= (1<<(ID-1));
			
			//nack BVAL AUX
			for(int k=1;k<=Nodes_N;k++){
				if(k == ID)continue;
				//BVAL0
				if(ACK_BVAL[aba_id][round][k][0])
					Send_Data_buff[packet_idx + 0] |= (1<<(k-1));
				//BVAL1
				if(ACK_BVAL[aba_id][round][k][1])
					Send_Data_buff[packet_idx + 1] |= (1<<(k-1));
				//AUX0
				if(ACK_AUX[aba_id][round][k][0])
					Send_Data_buff[packet_idx + 2] |= (1<<(k-1));
				//AUX1
				if(ACK_AUX[aba_id][round][k][1])
					Send_Data_buff[packet_idx + 3] |= (1<<(k-1));
			}
			packet_idx += 4;
			
			//share_nack
			for(int k=1;k<=Nodes_N;k++)
				if(ACK_ABA_share[aba_id][round][k])Send_Data_buff[packet_idx] |= (1<<(k-1));
			packet_idx++;
			
			//share
			//share �Լ���share ��û�м���?
			if(ACK_ABA_share[aba_id][round][ID] == 0 && aba_id_flag){
				Coin_SK_sign(aba_id,round);
			}
			for(int i=0;i<Share_Size;i++){
				Send_Data_buff[packet_idx + i] = ABA_share[aba_id][round][ID][i];
			}
			packet_idx += Share_Size;
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

void RBC_INIT_handler(){}

void RBC_ER_handler(){}

void PRBC_SIG_handler(){}

void CBC_STATE_handler(){}

void VAL_CBC_STATE_handler(){}

void ENC_handler(){}

void PACKET_handler(){
	ttt = ttt + 1;
	delay_ms(20);
//	ttt = ttt + 1;
//	HAL_UART_DMAStop(&uart3_handler);
//	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//����DMA����
//	return;
//	if(ReceBuff[0] == 0x00||ReceBuff[0] > Nodes_N || ReceBuff[1] > 20 || ReceBuff[2] > Nodes_N){
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
	
	if(packet_type == TypeSTATE){
		u8 padding = buff[2];
		
		if(s_id == 1 && !aba_id_flag){
			ABA_id = buff[3];
			aba_id_flag = 1;
		}
		if(!aba_id_flag)return;
		if(buff[4] != 2)return;
		
		
		u8 packet_idx = 3+padding;
		u8 round_number = buff[packet_idx];packet_idx++;
		//NACK
		u8 round_flag = 0;
		for(int o=0;o<round_number;o++){
			round_flag = 0;
			u8 aba_id = buff[packet_idx];packet_idx++;
			u8 round = buff[packet_idx];packet_idx++;
			u8 bval0 = buff[packet_idx + 0];
			u8 bval1 = buff[packet_idx + 1];
			u8 aux0 = buff[packet_idx + 2];
			u8 aux1 = buff[packet_idx + 3];

			//BVAL[0]
			if(((bval0 & (1<<(ID-1))) == 0) && BVAL[aba_id][round][0] == 1){
				round_flag = 1;
				//break;
			}
			//BVAL[1]
			if(((bval1 & (1<<(ID-1))) == 0) && BVAL[aba_id][round][1] == 1){
				round_flag = 1;
				//break;
			}
			
			//AUX[0]
			if(((aux0 & (1<<(ID-1))) == 0) && AUX[aba_id][round][0] == 1){
				round_flag = 1;
				//break;
			}
			//AUX[1]
			if(((aux1 & (1<<(ID-1))) == 0) && AUX[aba_id][round][1] == 1){
				round_flag = 1;
				//break;
			}

			//share_nack
			packet_idx += 4;
			if((buff[packet_idx] & (1<<(ID-1))) == 0 && ACK_ABA_share[aba_id][round][ID] == 1)round_flag = 1;	

			packet_idx++;
			if(round_flag){
				state_aba = aba_id;
				STATE_Set[0] = round;
				STATE_Set_Number=1;
				break;
			}
		}
		
		if(STATE_Set_Number)
			STATE_Set_Number = 1;
		
		if(STATE_Set_Number!=0)
			xTaskNotify((TaskHandle_t	)EventGroupTask_Handler,//��������֪ͨ��������
						(uint32_t		)EVENT_STATE,			//Ҫ���µ�bit
						(eNotifyAction	)eSetBits);				//����ָ����bit
		
		//STATE
		packet_idx = 4+padding;
		for(int o=0;o<round_number;o++){
			round_flag = 0;
			u8 aba_id = buff[packet_idx];packet_idx++;
			u8 round = buff[packet_idx];packet_idx++;
			u8 bval0 = buff[packet_idx + 0];
			u8 bval1 = buff[packet_idx + 1];
			u8 aux0 = buff[packet_idx + 2];
			u8 aux1 = buff[packet_idx + 3];
			
			if(ABA_R[aba_id] > round) continue;
			
			packet_idx += 4;
			packet_idx ++;
			
			if(!ACK_ABA_share[aba_id][round][s_id]){
				//��֤share ���� ��ʱ��
				u8 share[70];
				for(int i=0;i<Share_Size;i++)share[i] = 0x00;
				for(int i=0;i<Share_Size;i++)share[i] = buff[packet_idx+i];
				if(Coin_PK_verify_sig_share(s_id,aba_id,round,share)){
					for(int i=0;i<Share_Size;i++)ABA_share[aba_id][round][s_id][i] = buff[packet_idx+i];
					ABA_share_Number[aba_id][round]++;
					ACK_ABA_share[aba_id][round][s_id] = 1;
					if(ABA_share_Number[aba_id][round] >= Nodes_N){
						//�յ�f+1��coin�����кϳ�
						if(Shared_Coin[aba_id][round]==0xFF)
							Coin_PK_sig_combine_share(aba_id, round);//�ڲ�����֤
					}
				}else{
					while(1){
						sprintf((char*)Send_Data_buff,"Node %d verify sig error",s_id);
						LoRa_SendData(Send_Data_buff);
					}
				}
			}
			packet_idx += Share_Size;
			
			
			if((bval0 & (1<<(s_id-1))) && !ACK_BVAL[aba_id][round][s_id][0]){ACK_BVAL[aba_id][round][s_id][0] = 1;ACK_BVAL_Number[aba_id][round][0]++;}
			if((bval1 & (1<<(s_id-1))) && !ACK_BVAL[aba_id][round][s_id][1]){ACK_BVAL[aba_id][round][s_id][1] = 1;ACK_BVAL_Number[aba_id][round][1]++;}
			if((aux0 & (1<<(s_id-1))) && !ACK_AUX[aba_id][round][s_id][0]){ACK_AUX[aba_id][round][s_id][0] = 1;ACK_AUX_Number[aba_id][round][0]++;}
			if((aux1 & (1<<(s_id-1))) && !ACK_AUX[aba_id][round][s_id][1]){ACK_AUX[aba_id][round][s_id][1] = 1;ACK_AUX_Number[aba_id][round][1]++;}
			
			//�յ�f+1��BVAL(b)
			if(ACK_BVAL_Number[aba_id][round][0] >= Nodes_f + 1 && !BVAL[aba_id][round][0]){ACK_BVAL_Number[aba_id][round][0]++;BVAL[aba_id][round][0] = 1;}
			if(ACK_BVAL_Number[aba_id][round][1] >= Nodes_f + 1 && !BVAL[aba_id][round][1]){ACK_BVAL_Number[aba_id][round][1]++;BVAL[aba_id][round][1] = 1;}
			//�յ�2f+1��BVAL(b)
			if(ACK_BVAL_Number[aba_id][round][0] >= 2 * Nodes_f + 1){bin_values[aba_id][round][0] = 1;}
			if(ACK_BVAL_Number[aba_id][round][1] >= 2 * Nodes_f + 1){bin_values[aba_id][round][1] = 1;}
			
			//bin_values_R�ǿ�  �Լ�����AUX����¼number
			if(bin_values[aba_id][round][0] && !AUX[aba_id][round][0]){ACK_AUX_Number[aba_id][round][0]++;AUX[aba_id][round][0] = 1;}
			if(bin_values[aba_id][round][1] && !AUX[aba_id][round][1]){ACK_AUX_Number[aba_id][round][1]++;AUX[aba_id][round][1] = 1;}
			
			//�յ�N-f��AUX_r(b)   �㲥BVAL��
			if(ACK_AUX_Number[aba_id][round][0] >= (Nodes_N - Nodes_f) && !BVAL[aba_id][round][0]){ACK_BVAL_Number[aba_id][round][0]++;BVAL[aba_id][round][0] = 1;}
			if(ACK_AUX_Number[aba_id][round][1] >= (Nodes_N - Nodes_f) && !BVAL[aba_id][round][1]){ACK_BVAL_Number[aba_id][round][1]++;BVAL[aba_id][round][1] = 1;}
			//�յ�N-f��AUX_r(b)   �ռ���vals��	
			if(ACK_AUX_Number[aba_id][round][0] >= (Nodes_N - Nodes_f))vals[aba_id][round][0] = 1;
			if(ACK_AUX_Number[aba_id][round][1] >= (Nodes_N - Nodes_f))vals[aba_id][round][1] = 1;
			
			if(ACK_AUX_Number[aba_id][round][0] > 0)vals[aba_id][round][0] = 1;
			if(ACK_AUX_Number[aba_id][round][1] > 0)vals[aba_id][round][1] = 1;	
			
			u8 aux_flag = 0;
			if(ACK_AUX_Number[aba_id][round][0] + ACK_AUX_Number[aba_id][round][1] >= (Nodes_N - Nodes_f))aux_flag = 1;
				
			//vals��bin_values���Ӽ�
			if(aux_flag && subset_or_not(aba_id,round) && ABA_R[aba_id] == round){
				//shared coin
				//������һ��������N-f��share����Ϊ�ǰ󶨵�һ���
				//��ȡround��coin
				//�ݶ�Ϊ1����Ҫ�ϳ�
				if(ABA_share_Number[aba_id][round] < Nodes_f + 1){
					while(1){
						LoRa_CFG.chn = Msg_channel;LoRa_Set();
						sprintf((char*)Send_Data_buff,"Node %d error combine coin",ID);
						LoRa_SendData(Send_Data_buff);
					}
				}
				if(Shared_Coin[aba_id][round]==0xFF)
					Coin_PK_sig_combine_share(aba_id,round);
				u8 s = 0;
				s = Shared_Coin[aba_id][round];
				//if(round >= 5) s = Shared_Coin[aba_id][round];;

				
				if(vals[aba_id][round][0] == 1 && vals[aba_id][round][1] == 0){
					est[aba_id][round+1] = 0;
					BVAL[aba_id][round+1][0] = 1;
					ACK_BVAL_Number[aba_id][round+1][0]++;
					ABA_R[aba_id]++;
					if(s == 0){
						//ABA j done output 0
						if(ABA_decided[aba_id] == 0xFF){
							ABA_decided[aba_id] = 0;
							
							if(cur_aba_id == aba_id){
								ABA_time[cur_aba_id] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
								cur_aba_id++;
							}
							if(cur_aba_id == Nodes_N + 1){
								if(run_round_flag && ID == 1){
									LoRa_CFG.chn = 66;
									LoRa_Set();	
									for(int r=1 ;r <= Nodes_N;r++){
										MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
										sprintf((char*)Send_Data_buff,"%d",ABA_time[r]);
										LoRa_SendData(Send_Data_buff);
										delay_ms(500);
									}
									sprintf((char*)Send_Data_buff,"...");
									LoRa_SendData(Send_Data_buff);
									
									while(1);
								}
							}
							
						}
						else if (ABA_decided[aba_id] == 1){
							//error
							LED0(1);LED1(1);LED2(1);
							while(1);
						}	
					}
				}
				if(vals[aba_id][round][0] == 0 && vals[aba_id][round][1] == 1){
					est[aba_id][round+1] = 1;
					BVAL[aba_id][round+1][1] = 1;
					ACK_BVAL_Number[aba_id][round+1][1]++;
					ABA_R[aba_id]++;
					ABA_round_time[aba_id][round] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
					if(s == 1){
						//ABA j done output 1
						ABA_time[cur_aba_id] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
						if(!run_round_flag){
							if(ABA_decided[aba_id] == 0xFF){
								ABA_decided[aba_id] = 1;
								//ABA_time[aba_id] = TIM2->CNT + 9000 * TIM2_Exceed_Times;
								ABA_round[aba_id] = round;
								MY_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
								//if(cur_aba_id == aba_id)cur_aba_id++;
								if(ID == 1 && MY_Consensus_TIME==0){
									LoRa_CFG.chn = 77;
									LoRa_Set();	
									
									MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
									sprintf((char*)Send_Data_buff,"%d",MY_Consensus_TIME);
									LoRa_SendData(Send_Data_buff);
									while(1);
								}
								
							}
							else if (ABA_decided[aba_id] == 0){
								//error
								LED0(1);LED1(1);LED2(1);
								while(1);
							}
						}else cur_aba_id++;
						if(cur_aba_id == Nodes_N + 1){
							if(run_round_flag && ID == 1){
								LoRa_CFG.chn = 66;
								LoRa_Set();	
								for(int r=1 ;r <= Nodes_N;r++){
									MY_Consensus_TIME = TIM2->CNT + 9000 * TIM2_Exceed_Times;
									sprintf((char*)Send_Data_buff,"%d",ABA_time[r]);
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
				if(vals[aba_id][round][0] == 1 && vals[aba_id][round][1] == 1){
					est[aba_id][round+1] = s;
					BVAL[aba_id][round+1][s] = 1;
					ABA_R[aba_id]++;
				}
			}

//			packet_idx += 4;
//			packet_idx ++;
//			
//			if(!ACK_ABA_share[aba_id][round][s_id]){
//				//��֤share ���� ��ʱ��
//				u8 share[70];
//				for(int i=0;i<Share_Size;i++)share[i] = 0x00;
//				for(int i=0;i<Share_Size;i++)share[i] = buff[packet_idx+i];
//				if(Coin_PK_verify_sig_share(s_id,aba_id,round,share)){
//					for(int i=0;i<Share_Size;i++)ABA_share[aba_id][round][s_id][i] = buff[packet_idx+i];
//					ABA_share_Number[aba_id][round]++;
//					ACK_ABA_share[aba_id][round][s_id] = 1;
//					if(ABA_share_Number[aba_id][round] >= Nodes_N){
//						//�յ�f+1��coin�����кϳ�
//						if(Shared_Coin[aba_id][round]==0xFF)
//							Coin_PK_sig_combine_share(aba_id, round);//�ڲ�����֤
//					}
//				}else{
//					while(1){
//						sprintf((char*)Send_Data_buff,"Node %d verify sig error",s_id);
//						LoRa_SendData(Send_Data_buff);
//					}
//				}
//			}
//			packet_idx += Share_Size;
		}
	}
	
	////send 
//	for(int i=0;i<300;i++)ReceBuff[i] = 0x00;
	HAL_UART_DMAStop(&uart3_handler);
////	HAL_DMA_Init(&UART3RxDMA_Handler);
	HAL_UART_Receive_DMA(&uart3_handler,ReceBuff,DMA_buff_Size);//����DMA����
}









