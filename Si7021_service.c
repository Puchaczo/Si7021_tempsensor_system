/*
 * Si7021_service.c
 *
 *  Created on: Nov 26, 2022
 *      Author: kurvinox
 */

#include "Si7021.h"
#include "Si7021_service.h"


Si7021_sensor_state Si7021_service_init(Si7021_service *service, Si7021_sensor* sensor, I2C_HandleTypeDef *I2C_hand)
{
	if(HAL_I2C_IsDeviceReady(I2C_hand, SI7021_ADRESS,1,1) == HAL_OK)
		{
			sensor->I2C_handler = I2C_hand;
			sensor->state = SI7021_OK;

			SI7021_sensor_reset(sensor);


			sensor->humid_16bit = 0;
			sensor->temp_16bit = 0;

			sensor->heater.state = SI7021_HEATER_OFF;
			sensor->heater.value = 0;

			service->sensor = sensor;

			service->state = SI7021_SERVICE_READY;
			service->data_state = SI7021_DO_NOT_ASK;
			service->command = SI7021_NULL_COMMAND;

			service->temp_ready_flag = SI7021_DATA_ISNOTREADY;
			service->humid_ready_flag = SI7021_DATA_ISNOTREADY;

			service->tick = 0;
		}
	else
		sensor->state = SI7021_ERROR;

	return sensor->state;
}

/***************************************************************************************************
 * Serwisy przerwań
***************************************************************************************************/

void Si7021_TX_callback_service(Si7021_service *service)
{
	if(Si7021_get_data_state(service) == SI7021_DATA_ASKED)
	{
		if((Si7021_get_command(service) == SI7021_READ_TEMP)||(Si7021_get_command(service) == SI7021_READ_HUMID))
		{
			service->tick = HAL_GetTick();
			Si7021_set_data_state(service, SI7021_WAITING_FOR_CONV);
		}
		else
			Si7021_set_data_state(service, SI7021_CONV_READY);
	}

}


void Si7021_RX_callback_service(Si7021_service *service)
{
	if(Si7021_get_data_state(service) == SI7021_CONV_READY)
	{
		Si7021_set_data_state(service, SI7021_DATA_READY);
	}

}

/***************************************************************************************************
 * Funkcje sterujące
***************************************************************************************************/


void Si7021_data_read_cycle(Si7021_service *service)
{
	switch(service->data_state)
	{
		case SI7021_DATA_ASKED:
			break;

		case SI7021_WAITING_FOR_CONV:
		{
			if((HAL_GetTick() - service->tick) > SI7021_CONV_TIME_MS )
			{
				Si7021_set_data_state(service, SI7021_CONV_READY);
				break;
			}
		}
		case SI7021_CONV_READY:
		{
			Si7021_read_data(service->sensor, 2, nohold_mode_RX_wrap);
			break;
		}
		case SI7021_DATA_READY:
		{
			Si7021_set_service_state(service, SI7021_SERVICE_COMMAND);
		}

	}
}


static void Si7021_command_pick(Si7021_service *service)
{
	switch(Si7021_get_command(service))
	{
		case SI7021_READ_TEMP:
		{
			SI7021_temp_op(service->sensor);
			Si7021_set_sensor_ready(service);
			Si7021_set_temp_ready(service, SI7021_DATA_ISREADY);
			break;
		}

		case SI7021_READ_HUMID:
		{
			SI7021_humid_op(service->sensor);
			Si7021_set_sensor_ready(service);
			Si7021_set_humid_ready(service, SI7021_DATA_ISREADY);
			break;
		}
		case SI7021_READ_HEATER_VAL:
		{
			SI7021_heater_val_read_op(service->sensor);
			Si7021_set_sensor_ready(service);
			break;

		}
		case SI7021_SET_HEATER_VAL:
		{
			SI7021_heater_st_read_op(service->sensor);
			Si7021_set_sensor_ready(service);
			break;
		}
		case SI7021_READ_HEATER_STATE:
		{
			SI7021_heater_val_write_op(service->sensor);
			Si7021_set_sensor_ready(service);
			break;
		}
		case SI7021_SET_HEATER_STATE:
		{
			SI7021_heater_write_st_op(service->sensor);
			Si7021_set_sensor_ready(service);
			break;
		}
	}
}

void Si7021_main_service(Si7021_service *service)
{

	switch(service->state)
	{
		case SI7021_SERVICE_READY:
			break;

		case SI7021_SERVICE_OPERATING:
		{
			Si7021_data_read_cycle(service);
			break;
		}
		case SI7021_SERVICE_COMMAND:
		{
			Si7021_command_pick(service);

			//data_operation_fun(sensor);
			break;
		}
		case SI7021_SERVICE_ERROR:
			break;
	}

}

/***************************************************************************************************
 * Funkcje użytkownika
***************************************************************************************************/

void Si7021_read_temp(Si7021_service *service)
{
	if(Si7021_get_service_state(service) == SI7021_SERVICE_READY)
	{
		Si7021_set_service_state(service, SI7021_SERVICE_OPERATING);
		Si7021_set_data_state(service, SI7021_DATA_ASKED);
		Si7021_set_command(service, SI7021_READ_TEMP);

		Si7021_send_command(service->sensor, SI7021_TEMP_MEASURE_NOHOLD_MODE, SI7021_COMMAND_ONLY_L, REGISTER_VALUE_NULL, nohold_mode_TX_wrap);
	}
}

void Si7021_read_humid(Si7021_service *service)
{
	if(Si7021_get_service_state(service) == SI7021_SERVICE_READY)
	{
		Si7021_set_service_state(service, SI7021_SERVICE_OPERATING);
		Si7021_set_data_state(service, SI7021_DATA_ASKED);
		Si7021_set_command(service, SI7021_READ_HUMID);

		Si7021_send_command(service->sensor, SI7021_HUMID_MEASURE_NOHOLD_MODE, SI7021_COMMAND_ONLY_L, REGISTER_VALUE_NULL, nohold_mode_TX_wrap);
	}
}


void Si7021_read_heater_val(Si7021_service *service)
{
	if(Si7021_get_service_state(service) == SI7021_SERVICE_READY)
		{
		Si7021_set_service_state(service, SI7021_SERVICE_OPERATING);
		Si7021_set_data_state(service, SI7021_DATA_ASKED);
		Si7021_set_command(service, SI7021_READ_HEATER_VAL);

		Si7021_send_command(service->sensor, SI7021_READ_HEATER_REG, SI7021_COMMAND_ONLY_L, REGISTER_VALUE_NULL, nohold_mode_TX_wrap);
		}
}

void Si7021_set_heater_val(Si7021_service *service, uint8_t value)
{
	if(Si7021_get_service_state(service) == SI7021_SERVICE_READY)
	{
		Si7021_read_heater_val(service);
	    service->sensor->heater.value = value;
		Si7021_set_command(service, SI7021_SET_HEATER_VAL);
	}
}

void Si7021_read_heater_state(Si7021_service *service)
{
	if(Si7021_get_service_state(service) == SI7021_SERVICE_READY)
	{
		Si7021_set_service_state(service, SI7021_SERVICE_OPERATING);
		Si7021_set_data_state(service, SI7021_DATA_ASKED);
		Si7021_set_command(service, SI7021_READ_HEATER_STATE);

		Si7021_send_command(service->sensor, SI7021_READ_USER_REG, SI7021_COMMAND_ONLY_L, REGISTER_VALUE_NULL, nohold_mode_TX_wrap);
	}
}

void Si7021_set_heater_state(Si7021_service *service, Si7021_heater_state value)
{
	if(Si7021_get_service_state(service) == SI7021_SERVICE_READY)
	{
		Si7021_read_heater_state(service);
		service->sensor->heater.state = value;
		Si7021_set_command(service, SI7021_SET_HEATER_STATE);
	}
}


/***************************************************************************************************
 * Funkcje odczytu/ustawiania flag
***************************************************************************************************/

 Si7021_data_ready Si7021_is_temp_ready(Si7021_service *service)
{
	return service->temp_ready_flag ;
}

Si7021_data_ready Si7021_is_humid_ready(Si7021_service *service)
{
	return service->humid_ready_flag ;
}




void Si7021_set_temp_ready(Si7021_service *service, Si7021_data_ready state)
{
	service->temp_ready_flag = state;
}

void Si7021_set_humid_ready(Si7021_service *service, Si7021_data_ready state)
{
	service->humid_ready_flag = state;
}


void Si7021_set_service_state(Si7021_service* service, Si7021_service_state state)
{
	service->state = state;
}

Si7021_service_state Si7021_get_service_state(Si7021_service* service)
{
	return 	service->state;
}




void Si7021_set_data_state(Si7021_service *service, Si7021_data_state state)
{
	service->data_state = state;
}

Si7021_data_state Si7021_get_data_state(Si7021_service *service)
{
	return service->data_state;
}

void Si7021_set_command(Si7021_service *service, Si7021_user_command command)
{
	service->command = command;
}

Si7021_user_command Si7021_get_command(Si7021_service *service)
{
	return service->command;
}

/***************************************************************************************************
 * Funkcje odczytu danych temperatury wilgotności
***************************************************************************************************/

int Si7021_temp_return(Si7021_service *service)
{
	Si7021_set_temp_ready(service, SI7021_DATA_ISNOTREADY);
	return SI7021_get_temp_metric(service->sensor);
}

int Si7021_humid_return(Si7021_service *service)
{
	Si7021_set_humid_ready(service, SI7021_DATA_ISNOTREADY);
	return SI7021_get_humid_metric(service->sensor);
}

void Si7021_set_sensor_ready(Si7021_service *service)
{
	 Si7021_set_service_state(service, SI7021_SERVICE_READY);
	 Si7021_set_data_state(service, SI7021_READY_TOASK);
	 Si7021_set_command(service, SI7021_NULL_COMMAND);
}
