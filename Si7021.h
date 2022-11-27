/*
 * Si7021.h
 *
 *  Created on: 24.11.2022
 *      Author: PKieda
 */

#ifndef INC_SI7021_H_
#define INC_SI7021_H_

#include "main.h"


#define SI7021_ADRESS 							(uint16_t)0x80  		// Adrtes 8 bitowy czujnika,  7-bit -> 0x40



// komunikacja z czujnikiem odbywa się za pomocą komend, bez adresów rejestrów



//komendy odczytu temperatury i wilgotności
#define SI7021_HUMID_MEASURE_HOLD_MODE       	(uint8_t)0xE5			//Pomiar wilgotności bez utraty połączenia
#define SI7021_HUMID_MEASURE_NOHOLD_MODE    	(uint8_t)0xF5			//Pomiar wilgotności z utratą połączenia
#define SI7021_TEMP_MEASURE_HOLD_MODE          	(uint8_t)0xE3			//Pomiar temperatury bez utraty połączenia
#define SI7021_TEMP_MEASURE_NOHOLD_MODE       	(uint8_t)0xF3			//Pomiar temperatury z utratą połączenia

//komendy pracy na rejestrze kontrolnym użytkownika
#define SI7021_WRITE_USER_REG  				   	(uint8_t)0xE6			//zapis do rejestru
#define SI7021_READ_USER_REG    			   	(uint8_t)0xE7			//odczyt z rejestru

//komendy pracy na rejestrze grzałki
#define SI7021_WRITE_HEATER_REG  			   	(uint8_t)0x51			//zapis do rejestru
	#define SI7021_HEATER_VAL_MASK				(uint8_t)0x0F			//maska wartości grzałki z jej rejestru
	#define SI7021_HEATER_STATE_MASK			(uint8_t)0x04			//maska bitu informującego o włączeniu grzałki
#define SI7021_READ_HEATER_REG    			   	(uint8_t)0x11			//odczyt z rejestru

#define SI7021_RESET  					       	(uint8_t)0xFE			//komenda wykonania resetu

//pomocnicze definicje rozmiaru komend oraz definicja pustego bajtu, przy komendach bez wartości
#define SI7021_COMMAND_ONLY_L					(uint8_t)0x01			//wysylamy wylacznie komende
#define SI7021_COMMAND_AND_VALUE_L				(uint8_t)0x02			//wysyłamy komendę oraz wartosc (przy komendach "write")
#define REGISTER_VALUE_NULL					   	(uint8_t)0x00			//gdy wysylamy wyłaczeni komende, ustawiamy niepotrzebny argument na zero


#define SI7021_CONV_TIME_MS						(uint32_t)11			//czas konwersji czujnika według datasheet


typedef enum Si7021_sensor_state
{
	SI7021_OK,
	SI7021_ERROR
}Si7021_sensor_state;


typedef enum Si7021_heater_state
{
	SI7021_HEATER_OFF,
	SI7021_HEATER_ON = SI7021_HEATER_STATE_MASK

}Si7021_heater_state;


typedef struct Si7021_heater
{
	uint8_t value;
	Si7021_heater_state state;
}Si7021_heater;


typedef struct
{
	I2C_HandleTypeDef *I2C_handler;        // handler i2c przypisany do potencjometru

	uint8_t Rx[3];                         // wartosc odczytana
	uint8_t Tx[2];                         // wartosc do wysłania/wyslana

	Si7021_sensor_state state;

// zmienna przechowująca temperaturę w formacie po odczytaniu z  ADC czujnika
	union {
		uint8_t temp_8bit[2];
		uint16_t temp_16bit;
	};

// zmienna przechowująca wilgotnoscw formacie po odczytaniu z  ADC czujnika
	union {
		uint8_t humid_8bit[2];
		uint16_t humid_16bit;
	};

	Si7021_heater heater;

}Si7021_sensor;


//wrappery na funkcje wysyłające oraz odbierające I2c

 HAL_StatusTypeDef hold_mode_TX_wrap(Si7021_sensor *sensor, uint8_t length);
 HAL_StatusTypeDef hold_mode_RX_wrap(Si7021_sensor *sensor, uint8_t length);
 HAL_StatusTypeDef nohold_mode_TX_wrap(Si7021_sensor *sensor, uint8_t length);
 HAL_StatusTypeDef nohold_mode_RX_wrap(Si7021_sensor *sensor, uint8_t length);


 // główna funkcja wysłania informacji do czujnika
 Si7021_sensor_state SI7021_send_command(Si7021_sensor *sensor, uint8_t command, uint8_t length, uint8_t value, HAL_StatusTypeDef (*send_function)(Si7021_sensor *sensor, uint8_t lenght));
 // główna funkcja odczytania informacji z czujnika
 Si7021_sensor_state SI7021_read_data(Si7021_sensor *sensor, uint8_t length, HAL_StatusTypeDef (*receive_function)(Si7021_sensor *sensor, uint8_t lenght));


// funkcje w pollingu do odczytu zapisu
// wykorzystane były na poczatku w ramach testu, teraz nieużywane
 Si7021_sensor_state SI7021_read_temp_holdmode(Si7021_sensor *sensor);
 Si7021_sensor_state SI7021_read_humid_holdmode(Si7021_sensor *sensor);


//Funkcje operujące na danych z czunika/jego rejestrach
 void SI7021_temp_op(Si7021_sensor *sensor);
 void SI7021_humid_op(Si7021_sensor *sensor);
 void SI7021_heater_val_read_op(Si7021_sensor *sensor);
 void SI7021_heater_st_read_op(Si7021_sensor *sensor);
 void SI7021_heater_val_write_op(Si7021_sensor *sensor);
 void SI7021_heater_write_st_op(Si7021_sensor *sensor);


 // reset
 void SI7021_sensor_reset(Si7021_sensor *sensor);

 // odczyt i zmiana stanu czujnika

 void SI7021_set_sensor_state(Si7021_sensor* sensor, Si7021_sensor_state state);
 Si7021_sensor_state SI7021_get_sensor_state(Si7021_sensor* sensor);


//odczyt gotowych danych

 uint8_t SI7021_get_heater_val(Si7021_sensor *sensor);
 Si7021_heater_state SI7021_get_heater(Si7021_sensor *sensor);

 uint16_t SI7021_get_temp_uint16(Si7021_sensor sensor);
 uint16_t SI7021_get_humid_uint16(Si7021_sensor sensor);

 int SI7021_get_temp_metric(Si7021_sensor *sensor);
 int SI7021_get_humid_metric(Si7021_sensor *sensor);


#endif /* INC_SI7021_H_ */
