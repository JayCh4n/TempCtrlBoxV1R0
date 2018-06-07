/*
 * tempctrlboxmonitor_v1r0.c
 *
 * Created: 2018-04-03 09:05:18
 * Author : chenlong
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "usart.h"
#include "timer.h"
#include "dgus.h"
#include "gpio.h"
#include "eeprom.h"

void system_init(void);

int main(void)
{
	system_init();
	EN_INTERRUPT;
	read_eeprom_data();

	_delay_ms(2000);
	
	send_variables(MASTER_SWITCH, pre_system_sta);	//默认系统为关闭状态 调节屏幕状态图标为开启
	send_variables(ALL_SET_TEMP, all_temp);
	send_variables(SET_PREHEAT_TIME, preheat_time);
	send_variables(TEMP_UINT_ADDR, (CELSIUS+temp_unit*FAHRENHEIT));
	send_variables(ALL_SENSOR_TYPE_ADDR, TYPE_J+(all_sensor_type*TYPE_K));
	
	switch_language();
	
    /* Replace with your application code */
    while (1) 
    {
		if(usart0_rx_end)//屏相关数据接收完成
		{
			usart0_deal();
			usart0_rx_end = 0;
		}
		
		if(usart1_rx_end)//控制卡数据接收完成
		{
			usart1_deal();
			usart1_rx_end = 0;
		}
		
		if(in_main_page)//
		{
			update_main_page();
		}
    }
	
	return 0;
}

void system_init()
{
	gpio_init();
	usart0_init(MYUBRR(9600));
	usart1_init(MYUBRR(8928));
	timer0_init();
	timer2_init();
	timer1_init();
}
