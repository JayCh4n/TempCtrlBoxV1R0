/*
* timer.c
*
* Created: 2018-04-03 09:57:22
*  Author: chenlong
*/
#include "timer.h"

uint8_t timer2_ovf = 0;
// uint8_t ctrl_command = READ_DATA_ALL; //命令
uint8_t ctrl_command[20];
uint8_t ctrl_index = 0;
uint8_t all_set_flag = 0;
uint8_t all_set_cnt = 0;
// uint8_t global;						  //是否为全局设定
/*uint8_t all_senser_sta;*/
uint8_t read_setting_data_mask = 0;
uint8_t read_setting_data_cnt = 0;
uint8_t usart1_tx_timecnt = 0;
uint8_t usart1_tx_overtime_mask = 0;
uint8_t init_complete = 0;
uint8_t alarm_monitor_overtime_mask = 0;

ISR(TIMER0_OVF_vect)
{
	static uint8_t usart1_cnt = 0;		  //USART1接收时间计数
	static uint8_t pre_usart1_rx_cnt = 0; //USART1上次接收计数

	static uint8_t usart0_cnt = 0;		  //USART0接收时间计数
	static uint8_t pre_usart0_rx_cnt = 0; //USART0上次接收计数
	
/*	static uint8_t time_cnt = 0;  //时间计数*/
/*	static uint8_t slave_num = 1; //控制板号码*/
	static uint16_t update_run_temp_cnt = 0;
	static uint16_t alarm_monitor_cnt = 0;

	EN_INTERRUPT;
	TCNT0 = 0xB2;

	if (++read_setting_data_cnt >= 60)
	{
		read_setting_data_cnt = 0;
		read_setting_data_mask = 1;
	}

	if (init_complete)
	{
		if (++usart1_tx_timecnt >= 100)
		{
			usart1_tx_timecnt = 0;
			usart1_tx_overtime_mask = 1;
		} //每5百毫秒请求一个主控板发送所有采集数据  1500毫秒完成3个主控板采集数据的发送

		if(++alarm_monitor_cnt >= 300)
		{
			alarm_monitor_cnt = 0;
			alarm_monitor_overtime_mask = 1;
		}
	}

	if (update_run_temp_flag) //如果在单独设定界面，1s更新一次运行温度
	{
		update_run_temp_cnt++;

		if (update_run_temp_cnt >= 200)
		{
			update_run_temp_cnt = 0;

			if (run_temp_page)
			{
				send_variables(SINGLE_RUNTEMP_ADDR, run_temp[set_num] + temp_unit * ((run_temp[set_num] * 8 / 10) + 32)); //单独设定界面更新运行温度
			}
			else
			{
				send_variables(CURVE_PAGE_RUNTEMP_ADDR, run_temp[curve_page_num] +
				temp_unit * (run_temp[curve_page_num] * 8 / 10 + 32)); //曲线界面更新运行温度

				send_variables(CURVE_PAGE_OUTRATE_ADDR, output_rate[curve_page_num]); //曲线界面更新输出比例
			}
		}
	}

	if (usart1_rx_cnt != pre_usart1_rx_cnt)
	{
		pre_usart1_rx_cnt = usart1_rx_cnt;
		usart1_cnt = 0;
	} //如果usart1当前接收计数与上次计数不一样 表示有数据接收  清除时间计数
	else
	{
		if (usart1_rx_cnt != 0) //如果usart1接收计数为0 认为没有开始接收数据  则不进行超时计数
		{
			usart1_cnt++;

			if (usart1_cnt >= 4)
			{
				usart1_rx_lenth = usart1_rx_cnt;
				usart1_rx_end = 1;

				usart1_rx_cnt = 0;
				pre_usart1_rx_cnt = 0;

				usart1_cnt = 0;
			}
		}
	} //如果大于21毫秒没有数据接收  认为完成一次数据的接收      清除时间计数

	if (usart0_rx_cnt != pre_usart0_rx_cnt)
	{
		pre_usart0_rx_cnt = usart0_rx_cnt;
		usart0_cnt = 0;
	} //如果usart0当前接收计数与上次计数不一样 表示有数据接收  清除时间计数
	else
	{
		if (usart0_rx_cnt != 0) //如果usart0接收计数为0   认为没有开始接收数据  则不进行超时计数
		{
			usart0_cnt++;

			if (usart0_cnt >= 4)
			{
				usart0_rx_lenth = usart0_rx_cnt;
				usart0_rx_end = 1;

				usart0_rx_cnt = 0;
				pre_usart0_rx_cnt = 0;

				usart0_cnt = 0;
			}
		}
	} //如果大于21毫秒没有数据接收  认为完成一次数据的接收      清除时间计数
}

/*射胶阀时间控制相关程序 
**现把此功能独立成单独电路板
*/
ISR(TIMER1_COMPA_vect)
{
// 	EN_INTERRUPT;
// 	
// 	if(init_complete)
// 	{
// 		if(++screen_protection_time_cnt >= 30000)
// 		{
// 			screen_protection_over_time_mask = 1;
// 		}
// 	}
// 	
	/*
	uint8_t i = 0;

	static uint16_t time_cnt[8] = {0};
	static uint8_t sta[8] = {0};
	static uint8_t pre_sta[8] = {0};
	static uint8_t curve_cnt = 0;
	
	EN_INTERRUPT;

	for(i=0; i<8; i++)
	{
	if(first_start_iqr[i] == 1)
	{
	first_start_iqr[i] = 0;
	time_cnt[i] = t1[i];
	
	sta[i] = 0;
	pre_sta[i] = 0;
	clear_curve_buff(CHANNEL0_BUFF);
	}
	else if(pre_sta[i] != sta[i])
	{
	switch(sta[i])
	{
	case 0: time_cnt[i] = t1[i]; break;
	case 1:	time_cnt[i] = t2[i]; break;
	case 2: time_cnt[i] = t3[i]; break;
	case 3: time_cnt[i] = t4[i]; break;
	default: break;
	}
	
	pre_sta[i] = sta[i];
	}
	
	if(start_iqr[i] == 1)
	{
	time_cnt[i]--;
	if(time_cnt[i] <= 0)
	{
	if((sta[i] == 0) || (sta[i] == 2))
	{
	PORTC &= ~(1<<i);	//开启通道
	}
	else if((sta[i] == 1) || (sta[i] == 3))
	{
	PORTC |= (1<<i);	//关闭通道
	}
	
	sta[i] = sta[i] + 1;
	if(sta[i] >= 4)
	{
	sta[i] = 0;
	}
	}
	
	switch(i)
	{
	case 0: LED4_ON; break;
	case 1: LED3_ON; break;
	case 2: LED2_ON; break;
	case 3: LED1_ON; break;
	}
	}
	else if(start_iqr[i] == 0)
	{
	PORTC |= (1<<i);
	
	switch(i)
	{
	case 0: LED4_OFF; break;
	case 1: LED3_OFF; break;
	case 2: LED2_OFF; break;
	case 3: LED1_OFF; break;
	}

	}
	}
	
	curve_cnt++;
	if(curve_cnt >= 25)
	{
	curve_cnt = 0;
	
	if(in_time_ctrl_page == 1)
	{
	if(start_iqr[pre_iqr] == 1)
	{
	if((sta[pre_iqr] == 0) || (sta[pre_iqr] == 2))
	{
	send_to_channel(CHANNEL0, 0x11);
	}
	else if((sta[pre_iqr] == 1) || (sta[pre_iqr] == 3))
	{
	send_to_channel(CHANNEL0, 0x33);
	}
	}
	}
	}
	*/
}

/*更新温度曲线 在曲线显示界面时才会更新*/
ISR(TIMER2_OVF_vect)
{
	static uint8_t time_cnt = 0;
	int16_t temp_differ = 0;

	EN_INTERRUPT;
	TCNT2 = 0xB2;

	// temp_differ = (run_temp[curve_page_num] - set_temp[curve_page_num]) + 100;

	/*温度转换 计算不同单位下的温度数值  run_temp[] set_temp[] 存储的是摄氏度 */
	temp_differ = (run_temp[curve_page_num]
				+ temp_unit * (run_temp[curve_page_num] * 8 / 10 + 32))
				- (set_temp[curve_page_num] + temp_unit * (set_temp[curve_page_num] * 8 / 10 + 32)) + 100;

	time_cnt++;
	if (time_cnt >= timer2_ovf)
	{
		time_cnt = 0;
		send_to_channel(CHANNEL0, temp_differ);
	}
}

/*IO口模拟usart发送*/
ISR(TIMER3_COMPA_vect)
{
	static uint8_t cnt = 0;
	static uint8_t pos = 0; //当前发送数据BIt 的位置

	cnt++;

	if (usart2_sta == USART2_IN_TX)
	{
		if (cnt <= 8)
		{
			if (((usart2_buff & (1 << pos)) >> pos) == 1)
			{
				USART2_TX_PIN_SET;
			}
			else
			{
				USART2_TX_PIN_RESET;
			}
			pos++;
		}

		if (cnt == 9)
		{
			pos = 0;		   //数据发送完成，发送位置恢复1
			USART2_TX_PIN_SET; //停止位
		}

		if (cnt == 10)
		{
			cnt = 0;
			usart2_sta = USART2_TX_END; //更改USART2状态为发送完成
			TCCR3B &= 0xF8;				//关闭关闭定时器
		}
	}
}

void timer0_init()
{
	TCCR0 &= 0xF8; //关闭定时器0

	TCNT0 = 0xB2; //定时5ms

	TIMSK &= 0xFC;
	TIMSK |= 0x01; //使能定时器0溢出中断

	TCCR0 |= 0x07; //开启定时器0 正常模式TOP:0xFF, OC0不输出信号, clk/1024
}

void timer1_init()
{
	TCCR1B |= 0x08; //WGM12位置1
	TCCR1B &= 0xF8; //关闭定时器（清除时钟选择位 CS10 CS11 CS12）

	OCR1AH = 0x09; //定时10ms
	OCR1AL = 0xC4;

	TIMSK |= 0x10; //定时器1 A输出比较中断使能

	TCCR1B |= 0x03; //开启定时器1 CTC模式 top:OCR1A, OC1A OC1B OC1C DISCONNECT,CLK/64
}

void timer2_init()
{
	TCCR2 &= 0xF8; //关闭定时器

	TCNT2 = 0xB2; //定时5ms

	TIMSK &= 0x3F;
	TIMSK |= 0x40; //使能定时器2溢出中断

	TCCR2 |= 0x05; //开启定时器2 正常模式TOP:0xFF, OC0不输出信号, clk/1024
	TCCR2 &= 0xF8;
}

/* 移到串口初始化里
void timer3_init()
{
TCCR3B |= 0x80;			//WGM32位置1
TCCR3B &= 0xF8;			//关闭定时器（清除时钟选择位 CS30 CS31 CS32）

ETIMSK |= 0x10;			//定时器3 A输出比较中断使能
}
*/
