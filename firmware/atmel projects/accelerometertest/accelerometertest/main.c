/*
 * accelerometertest.c
 *
 * Created: 10/19/2020 10:37:29 PM
 * Author : mbourgeois2
  heavily based off of https://github.com/YifanJiangPolyU/MPU6050
 */ 

#define F_CPU 16000000
#define MPU6050_ADDR 0x68
#define PI 3.141592

#include "twi_master.h"
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

/************************************************************************/
/*							Initializations                             */
/************************************************************************/

/* MPU6050 register address */
#define ACCEL_XOUT_H	0x3B
#define ACCEL_XOUT_L	0x3C
#define ACCEL_YOUT_H	0x3D
#define ACCEL_YOUT_L	0x3E
#define ACCEL_ZOUT_H	0x3F
#define ACCEL_ZOUT_L	0x40
#define PWR_MGMT_1		0x6B

typedef struct
{
	float x;
	float y;
	float z;
} mpu_data_t;


/************************************************************************/
/*							Prototype functions                         */
/************************************************************************/

void ERROR_CHECK(ret_code_t error_code);
void mpu_init(void);
void mpu_get_accel_raw(mpu_data_t* mpu_data);
void mpu_get_accel(mpu_data_t* mpu_data);


/************************************************************************/
/*							Function definitions                        */
/************************************************************************/

void ERROR_CHECK(ret_code_t error_code)
{
	if (error_code != SUCCESS)
	{
		while (1); // loop indefinitely
	}
}


void mpu_init(void)
{
	ret_code_t error_code;
	//puts("Write 0 to PWR_MGMT_1 reg to wakeup MPU.");
	uint8_t data[2] = {PWR_MGMT_1, 0};
	error_code = tw_master_transmit(MPU6050_ADDR, data, sizeof(data), false);
	//ERROR_CHECK(error_code);
}


void mpu_get_accel_raw(mpu_data_t* mpu_data)
{
	ret_code_t error_code;
	/* 2 registers for each of accel x, y and z data */
	uint8_t data[6];
	
	data[0] = ACCEL_XOUT_H;
	error_code = tw_master_transmit(MPU6050_ADDR, data, 1, true);
	//ERROR_CHECK(error_code);
	
	error_code = tw_master_receive(MPU6050_ADDR, data, sizeof(data));
	//ERROR_CHECK(error_code);
	
	/* Default accel config +/- 2g */
	mpu_data->x = (int16_t)(data[0] << 8 | data[1]) / 16384.0;
	mpu_data->y = (int16_t)(data[2] << 8 | data[3]) / 16384.0;
	mpu_data->z = (int16_t)(data[4] << 8 | data[5]) / 16384.0;
}


void mpu_get_accel(mpu_data_t* mpu_data)
{
	mpu_get_accel_raw(mpu_data);
	mpu_data->x = mpu_data->x * 9.81;
	mpu_data->y = mpu_data->y * 9.81;
	mpu_data->z = mpu_data->z * 9.81;
}


/************************************************************************/
/*							Main application                            */
/************************************************************************/

int main(void)
{
	DDRD = 0xFF;
	DDRB = 0x03;
	/* Initialize project configuration */
	tw_init(TW_FREQ_400K, true); // set I2C Frequency, enable internal pull-up
	mpu_init();
	mpu_data_t accel;
	float roll=0;//,pitch=0;
	uint8_t i = 0;
	while (1)
	{
		//puts("Read accelerometer data.");
		mpu_get_accel(&accel);
		//actual config
		roll = ((atan(-accel.y / accel.z)*180/PI)+90)*(accel.z/fabs(accel.z));
		if (roll >= 0)
			i = roll/20;
		else
			i = 9;
		//test config
		//roll = ((atan(-accel.y / accel.z)*180/PI)+90);
		//i = roll/20;
		//pitch = atan(accel.x / sqrt(pow(accel.y,2) + pow(accel.z,2)))*180/PI;
		//i = 4.5 + (roll/20);
		//i = 4 + (roll/22.5);
		
		
		//test code that works
		/*
		if (i < 8)
		{
			PORTB = 0x00;
			PORTD = (1<<(i));
		}
		if (i == 8)
		{
			PORTD = 0x00;
			PORTB = 0x01;
		}
		if (i == 9)
		{
			PORTD = 0x00;
			PORTB = 0x00;
		}
		_delay_ms(100);
		*/
		
		
		//prototype v2 (implemented decoder)
		/*
		if (i == 9)
		{
			PORTD = 0x00;
			PORTB = 0x00;
		}
		else
		{
			PORTD = (((i) & 0x0F)<<5);
			PORTB = (((~(i) & 0x08))<<4) | (((~(i) & 0x08))<<3);
		}
		*/
		
		//prototype v3 (prototype demonstration)
		if (i == 9)
		{
			PORTD = 0x00;
			PORTB = 0x01;
		}
		else
		{
			PORTD = (((i) & 0x0F)<<5);
			PORTB = (((~(i) & 0x08))>>2) | (((~(i) & 0x08))>>3);
		}
		//final code?
		/*
		if (i == 9)
		{
			PORTD = 0x00;
			PORTB = (1<<6);
		}
		else
		{
			PORTD = (((8+i) & 0x0F)<<5);
			PORTB = (((~(8+i) & 0x08))<<4) | (((~(8+i) & 0x08))<<3);
		}
		*/
		_delay_ms(100);
	}
}
