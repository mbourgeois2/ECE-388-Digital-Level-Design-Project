/*
 * accelerometertest
 *
 * Created: 10/19/2020 10:37:29 PM
 * Author : mbourgeois2
  heavily based off of https://github.com/YifanJiangPolyU/MPU6050
 */ 

//#define F_CPU 1000000
#define MPU6050_ADDR 0x68
#define PI 3.141592

#include "twi_master.h"
#include <avr/io.h>
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
uint8_t dindex(int q);
void dispdig(int q);

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
	ret_code_t 
	error_code;
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

uint8_t dindex(int q)
{
	uint8_t a=0,b=0,c=0;
	a = (q&1)<<2;
	b = q&2;
	c = (q&4)>>2;
	return (a+b+c);
}

void dispdig(int q)
{
	PORTD = 0x00;
	PORTB = 0b11000000;
	int k = 0, yes = 16384;
	while(yes > 0)
	{
		PORTD = (dindex(k) << 5);
		k++;
		if (k > (q-1))
			k=0;
		yes--;
	}
	PORTB = 0x00;
	PORTD = 0x00;
}

/************************************************************************/
/*							Main application                            */
/************************************************************************/

int main(void)
{
	DDRD = 0xFF;
	DDRB = 0b11000000;
	/* Initialize project configuration */
	tw_init(TW_FREQ_400K, true); // set I2C Frequency, enable internal pull-up
	mpu_init();
	mpu_data_t accel;
	float roll = 0, range = 0, x = 0, y = 0, z = 0, alpha = 0.9;
	uint8_t i = 0, j = 0, mode = 1, modeselect = mode*(mode - 2) + 3;
	x = accel.x;
	y = accel.y;
	z = accel.z;
	while (1)
	{
		//puts("Read accelerometer data.");
		mpu_get_accel(&accel);
		//actual config
		
		//lowpass filter to smooth accelerometer data
		x = accel.x + alpha*(x - accel.x);
		y = accel.y + alpha*(y - accel.y);
		z = accel.z + alpha*(z - accel.z);
		
		//sensitivity mode check
		if ((sqrt(pow(accel.x,2) + pow(accel.y,2) + pow(accel.z,2)) >= 32)) //min 32?
		{
			mode++;
			if (mode > 3)
				mode = 1;
			dispdig(mode);
			modeselect = mode*(mode - 2) + 3;
		}
				
		//angle and LED calculations
		range = 90/modeselect;
		roll = ((atan(-y / z)*180/PI)+90)*(z/fabs(z));
		if ((90 - range <= roll) && (90 + range > roll))
		{
			i = ((roll - range*(modeselect - 1))*modeselect/20);		//default roll/20	//obsolete (roll - range*(modeselect - 1))*modeselect/20;
			i=8-i;
			j = dindex(i);
			PORTB = (((~(i) & 0x08))<<4) | (((~(i) & 0x08))<<3);	//enable decoder | enable led8
			PORTD = (((j) & 0x0F)<<5);
		}
		else
		{
			PORTD = 0x00;
			PORTB = (1<<6);
		}
		
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
		/*
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
		*/
		//final code?
		//PORTB = 0b10000000;
		//_delay_ms(50);
	}
}