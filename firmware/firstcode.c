#define F_CPU 16000000		//CPU frequency (will change in final design, uses 3V instead of 5V)
#define MPU6050 0x68		//twi address of the accelerometer

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

int main(void)
{
	//page 307-308 in the 328PB datasheet
	TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	while (!(TWCR0 & (1<<TWINT)));

	if ((TWSR0 & 0xF8) != TW_START) {
		return -1;
	}

	TWDR0 = SLA_W;
	TWCR0 = (1<<TWINT)|(1<<TWEN);

	while (!(TWCR0 & (1<<TWINT)));

	if ((TWSR0 & 0xF8) != TW_MT_SLA_ACK) {
		return -1;
	}

	TWDR0 = 0x6B;
	TWCR0 = (1<<TWINT)|(1<<TWEN);

	while (!(TWCR0 & (1<<TWINT)));

	if ((TWSR0 & 0xF8) != TW_MT_DATA_ACK) {
		return -1;
	}

	TWCR0 = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	return 0;
}

