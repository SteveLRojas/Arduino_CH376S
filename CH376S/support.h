typedef union Bitfield_16
{
	struct Bits_16
	{
		unsigned bit0 : 1;
		unsigned bit1 : 1;
		unsigned bit2 : 1;
		unsigned bit3 : 1;
		unsigned bit4 : 1;
		unsigned bit5 : 1;
		unsigned bit6 : 1;
		unsigned bit7 : 1; 
		unsigned bit8 : 1;
		unsigned bit9 : 1;
		unsigned bit10 : 1;
		unsigned bit11 : 1;
		unsigned bit12 : 1;
		unsigned bit13 : 1;
		unsigned bit14 : 1;
		unsigned bit15 : 1; 
	} bits;
	unsigned int value;
} bitfield_16_t;

extern bitfield_16_t bitfield_16;

unsigned int get_word();
unsigned int build_word();
byte get_test_byte();
void byte_to_hex(byte value, char* buff);
void word_to_hex(word, char*);
void print_hex_byte(byte);
void print_hex_word(uint16_t);
void print_test_data();
uint16_t get_hex();
