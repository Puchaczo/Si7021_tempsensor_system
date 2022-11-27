/*
 * Si7021_service.h
 *
 *  Created on: Nov 26, 2022
 *      Author: kurvinox
 */

#ifndef SI7021_SERVICE_H_
#define SI7021_SERVICE_H_

/*
 * Si7021_service.c
 *
 *  Created on: Nov 27, 2022
 *      Author: kurvinox
 */

#include "Si7021.h"

typedef enum Si7021_service_state
{
	SI7021_SERVICE_READY,
	SI7021_SERVICE_OPERATING,
	SI7021_SERVICE_COMMAND,
	SI7021_SERVICE_ERROR
}Si7021_service_state;

typedef enum Si7021_data_state
{
	SI7021_DO_NOT_ASK,
	SI7021_READY_TOASK,
	SI7021_DATA_ASKED,
	SI7021_WAITING_FOR_CONV,
	SI7021_CONV_READY,
	SI7021_DATA_READY,

}Si7021_data_state;

typedef enum Si7021_user_command
{
	SI7021_NULL_COMMAND,
	SI7021_RESET_COMMAND,
	SI7021_READ_TEMP,
	SI7021_READ_HUMID,
	SI7021_READ_HEATER_VAL,
	SI7021_SET_HEATER_VAL,
	SI7021_READ_HEATER_STATE,
	SI7021_SET_HEATER_STATE,
	SI7021_READ_ID,

}Si7021_user_command;


typedef enum Si7021_data_ready
{
	SI7021_DATA_ISREADY,
	SI7021_DATA_ISNOTREADY

}Si7021_data_ready;


typedef struct
{
	Si7021_sensor* sensor;

	Si7021_service_state state;
	Si7021_data_state data_state;
	Si7021_user_command command;

	Si7021_data_ready temp_ready_flag;
	Si7021_data_ready humid_ready_flag;

	uint32_t tick;

}Si7021_service;


Si7021_sensor_state Si7021_service_init(Si7021_service* service, Si7021_sensor* sensor, I2C_HandleTypeDef *I2C_hand);

//funkcje co umieszczenia w przerwaniach po zakończeniu konwersji I2c
void Si7021_TX_callback_service(Si7021_service *service);
void Si7021_RX_callback_service(Si7021_service *service);

//główne funkcje sterujące operacjami/stanem czujnika
void Si7021_data_read_cycle(Si7021_service *service);
static void Si7021_command_pick(Si7021_service *service);
void Si7021_main_service(Si7021_service *service);

//Funkcje użytkownika do kontroli/odczytu czujnika
void Si7021_read_temp(Si7021_service *service);
void Si7021_read_humid(Si7021_service *service);
void Si7021_read_heater_val(Si7021_service *service);
void Si7021_set_heater_val(Si7021_service *service, uint8_t value);
void Si7021_read_heater_state(Si7021_service *service);
void Si7021_set_heater_state(Si7021_service *service, Si7021_heater_state value);

//funkcje odczytu/zmiany stanu
void Si7021_set_sensor_ready(Si7021_service *service);

void Si7021_set_data_state(Si7021_service* service, Si7021_data_state state);
Si7021_data_state Si7021_get_data_state(Si7021_service* service);

void Si7021_set_service_state(Si7021_service* service, Si7021_service_state state);
Si7021_service_state Si7021_get_service_state(Si7021_service* service);

void Si7021_set_command(Si7021_service* service, Si7021_user_command command);
Si7021_user_command Si7021_get_command(Si7021_service* service);

Si7021_data_ready Si7021_is_temp_ready(Si7021_service *service);
Si7021_data_ready Si7021_is_humid_ready(Si7021_service *service);

void Si7021_set_temp_ready(Si7021_service *service, Si7021_data_ready state);
void Si7021_set_humid_ready(Si7021_service *service, Si7021_data_ready state);


#endif /* SI7021_SERVICE_H_ */
