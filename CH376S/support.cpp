#include <SPI.h>
#include "support.h"

byte n_bytes_in_test_buff = 0;
byte test_buff[2];

bitfield_16_t bitfield_16;

unsigned int get_word()
{
	byte next_bit;
	unsigned int result;
	result = bitfield_16.value;
	next_bit = ~(bitfield_16.bits.bit0 ^ bitfield_16.bits.bit7);
	bitfield_16.value = bitfield_16.value >> 1;
	bitfield_16.bits.bit15 = next_bit;
	return result;
}

unsigned int build_word()
{
	for(int d = 0; d < 15; ++d)
	{
		get_word();
	}
	return get_word();
}

byte get_test_byte()
{
	unsigned int current_word;
	if(n_bytes_in_test_buff == 0)
	{
		current_word = build_word();
		test_buff[0] = current_word & 0xff;
		test_buff[1] = (current_word >> 8) & 0xff;
		n_bytes_in_test_buff = 2;
	}
	-- n_bytes_in_test_buff;
	return test_buff[n_bytes_in_test_buff];
}

void byte_to_hex(byte value, char* buff)
{
	char table[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};
	buff[0] = table[(value >> 4) & 0x0f];
	buff[1] = table[(value) & 0x0f];
	buff[2] = NULL;
}

void word_to_hex(uint16_t value, char* buff)
{
	byte_to_hex(value >> 8, buff);
	byte_to_hex(value & 0xFF, buff + 2);
}

void print_hex_byte(byte value)
{
	char buff[3];
	byte_to_hex(value, buff);
	Serial.println(buff);
}

void print_hex_word(uint16_t value)
{
	char buff[5];
	word_to_hex(value, buff);
	Serial.println(buff);
}

void print_test_data()
{
	char buff[3];
	unsigned int counter = 0;
	unsigned int current_word;
	bitfield_16.value = 0;
	do
	{
		current_word = build_word();
		byte_to_hex((current_word >> 8) & 0xff, buff);
		Serial.print(buff);
		byte_to_hex(current_word & 0xff, buff);
		Serial.print(buff);
		if((counter & 0x0f) == 0x0f)
			Serial.print('\n');
		++counter;
	} while(counter != 0);
}

uint16_t get_hex()
{
	uint8_t hexbuf[6];  //leave space for pesky CR LF
	uint8_t temp;
	uint16_t value = 0;

	for(byte d = 0; d < 6; ++d)
	{
		hexbuf[d] = 0;
	}

	while(!Serial.available()) {}

	Serial.readBytes(hexbuf, 6);

	for(byte d = 0; d < 6; ++d)
	{
		temp = hexbuf[d];
		if(temp >= '0' && temp <= '9')  //convert numbers
			temp = temp - '0';
		else if(temp >= 'A' && temp <= 'F')   //convert uppercase letters
			temp = temp - 'A' + 10;
		else if(temp >= 'a' && temp <= 'f')   //convert the annoying lowercase letters
			temp = temp - 'a' + 10;
		else
			continue;
	
		value = value << 4;
		value = value | temp;
	}

	return value;
}
