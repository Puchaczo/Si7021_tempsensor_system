/*
 * Si7021.c
 *
 *  Created on: 24.11.2022
 *      Author: PKieda
 */
#include "Si7021.h"


/************************************************************************************************************/

HAL_StatusTypeDef hold_mode_TX_wrap(Si7021_sensor *sensor, uint8_t length)
{
	return (HAL_I2C_Master_Transmit(sensor->I2C_handler, SI7021_ADRESS, sensor->Tx, length, SI7021_CONV_TIME_MS));
}

HAL_StatusTypeDef hold_mode_RX_wrap(Si7021_sensor *sensor, uint8_t length)
{
	return (HAL_I2C_Master_Receive(sensor->I2C_handler, SI7021_ADRESS, sensor->Rx, length, SI7021_CONV_TIME_MS));
}

 HAL_StatusTypeDef nohold_mode_TX_wrap(Si7021_sensor *sensor, uint8_t length)
{
	return (HAL_I2C_Master_Transmit_IT(sensor->I2C_handler, SI7021_ADRESS, sensor->Tx, 2));
}

HAL_StatusTypeDef nohold_mode_RX_wrap(Si7021_sensor *sensor, uint8_t length)
{
	return (HAL_I2C_Master_Receive_IT(sensor->I2C_handler, SI7021_ADRESS, sensor->Rx, 2));
}

/************************************************************************************************************/

void Si7021_send_command(Si7021_sensor *sensor, uint8_t command, uint8_t length, uint8_t value, HAL_StatusTypeDef (*send_function)(Si7021_sensor *sensor, uint8_t lenght))
{
	sensor->Tx[1] = 0;

	sensor->Tx[0] = command;
	sensor->Tx[1] = value;

	send_function(sensor, length);

}

void Si7021_read_data(Si7021_sensor *sensor, uint8_t length, HAL_StatusTypeDef (*receive_function)(Si7021_sensor *sensor, uint8_t lenght))
{
	sensor->Rx[0] = 0;
	sensor->Rx[1] = 0;
	sensor->Rx[2] = 0;

	receive_function(sensor, length);
}

/*************************************************************************
 * Funkcje odczytu temperatury oraz wilgotności  w trybie pollingu
 ************************************************************************/


Si7021_sensor_state SI7021_read_temp_holdmode(Si7021_sensor *sensor)
{
	send_command(sensor, SI7021_TEMP_MEASURE_HOLD_MODE, SI7021_COMMAND_ONLY_L, REGISTER_VALUE_NULL, hold_mode_TX_wrap);
		read_data(sensor, 2, hold_mode_RX_wrap);

			sensor->temp_8bit[1] = (sensor->Rx[0]);
			sensor->temp_8bit[0] = (sensor->Rx[1]);

		return sensor->state;

}

Si7021_sensor_state SI7021_read_humid_holdmode(Si7021_sensor *sensor)
{
	send_command(sensor, SI7021_HUMID_MEASURE_HOLD_MODE, SI7021_COMMAND_ONLY_L, REGISTER_VALUE_NULL, hold_mode_TX_wrap);
		read_data(sensor, 2, hold_mode_RX_wrap);

			sensor->humid_8bit[1] = (sensor->Rx[0]);
			sensor->humid_8bit[0] = (sensor->Rx[1]);

			return sensor->state;
}


 /************************************************************************************************************/

void SI7021_temp_op(Si7021_sensor *sensor)
{
		sensor->temp_8bit[1] = (sensor->Rx[0]);
		sensor->temp_8bit[0] = sensor->Rx[1];
}

void SI7021_humid_op(Si7021_sensor *sensor)
{
	sensor->humid_8bit[1] = (sensor->Rx[0]);
	sensor->humid_8bit[0] = sensor->Rx[1];
}

void SI7021_heater_val_read_op(Si7021_sensor *sensor)
{
	sensor->heater.value = sensor->Rx[0] & (SI7021_HEATER_VAL_MASK);
}

void SI7021_heater_st_read_op(Si7021_sensor *sensor)
{
	sensor->heater.state = (sensor->Rx[0] & (SI7021_HEATER_STATE_MASK));
}


void SI7021_heater_val_write_op(Si7021_sensor *sensor)
{
	uint8_t temp;

	temp = sensor->Rx[0] & (~(SI7021_HEATER_VAL_MASK));
	temp |= sensor->heater.value;

	Si7021_send_command(sensor, SI7021_WRITE_HEATER_REG,SI7021_COMMAND_AND_VALUE_L, temp, nohold_mode_TX_wrap);

}

void SI7021_heater_write_st_op(Si7021_sensor *sensor)
{
	uint8_t temp;

	temp = sensor->Rx[0] & (~(SI7021_HEATER_STATE_MASK));
	temp |= sensor->heater.state;

	Si7021_send_command(sensor, SI7021_WRITE_USER_REG,SI7021_COMMAND_AND_VALUE_L, temp, nohold_mode_TX_wrap);
}

/************************************************************************************************************/


void SI7021_sensor_reset(Si7021_sensor *sensor)
{
	Si7021_send_command(sensor, SI7021_READ_HEATER_REG, SI7021_COMMAND_ONLY_L, REGISTER_VALUE_NULL, hold_mode_TX_wrap);
}

/************************************************************************************************************/

void SI7021_set_sensor_state(Si7021_sensor* sensor, Si7021_sensor_state state)
{
	sensor->state = state;
}

Si7021_sensor_state SI7021_get_sensor_state(Si7021_sensor* sensor)
{
	return sensor->state;
}

/************************************************************************************************************/

void SI7021_set_heater_state(Si7021_sensor* sensor, Si7021_heater_state state)
{
	sensor->heater.state = state;
}

Si7021_heater_state SI7021_get_heater_state(Si7021_sensor* sensor)
{
	return sensor->heater.state;
}

/************************************************************************************************************/

uint8_t SI7021_get_heater_val(Si7021_sensor *sensor)
{
    return sensor->heater.value;
}

Si7021_heater_state SI7021_get_heater(Si7021_sensor *sensor)
{
    return sensor->heater.state;
}


/************************************************************************************************************/

//temperatura obliczona według datasheet
int SI7021_get_temp_metric(Si7021_sensor *sensor)
{
	float temp;

	temp = 175.72 * ((float)(sensor->temp_16bit));
	temp /= 65536;
    temp -= 46.85;

    temp*= 10;

    return ((int)temp);
}

//wilgotnośc obliczona według datasheet
int SI7021_get_humid_metric(Si7021_sensor *sensor)
{
	float hum;

	hum = 125 * (float)sensor->humid_16bit;
	hum /= 65536;
	hum -= 6;


	//zgodnie z dokumentacją wartości wilgotności mogą przekraczac wartosci minimalne i maksymalne i nalezy je recznie ustawic
	if(hum > 100)
	{
		hum=100;
	}
	else if(hum < 0)
	{
		hum=0;
	}

	hum*= 10;
   return hum;
}


uint16_t SI7021_get_temp_uint16(Si7021_sensor sensor)
{
	return sensor.temp_16bit;
}

uint16_t SI7021_get_humid_uint16(Si7021_sensor sensor)
{
	return sensor.temp_16bit;
}


