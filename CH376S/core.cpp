#include <SPI.h>
#include "core.h"
#include "support.h"

byte timeout_flag = 0;
uint16_t bytes_read = 0;
unsigned long file_size;	//in bytes
unsigned long capacity;		//in sectors
unsigned long total_sectors;
unsigned long free_sectors;

byte wait_status(byte target, uint16_t timeout)
{
	byte last_status;
	timeout_flag = 0;

	while (timeout != 0)
	{
		delay(1);
		last_status = usb_get_status();
		if(last_status == target)
			return 0x00;
		timeout--;
	}

	Serial.println(F("ERR: Timeout"));
	timeout_flag = 1;
	return last_status;
}

void usb_file_create()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x34); //file create command
	digitalWrite(10, HIGH);
}

void usb_byte_write()
{
	byte* size = (byte*)((void*)&file_size);
	digitalWrite(10, LOW);
	SPI.transfer(0x3c); //byte write command
	SPI.transfer(size[0]);  //length low byte
	SPI.transfer(size[1]);	//length high byte
	digitalWrite(10, HIGH);
}

void usb_byte_wr_go()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x3d);
	digitalWrite(10, HIGH);
}

void usb_write_test_data()
{
	uint16_t bytes_written;
	byte bytes_requested;
	
	//usb_disk_query();
	if(free_sectors < (0x20000 >> 9))
	{
		Serial.println("Not enough space");
		return;
	}

	random_init(0);
	
	usb_file_create();
	wait_status(0x14, 3000);
	if(timeout_flag)
		return;

	for(byte d = 0; d < 4; ++d)	//WE WILL WRITE 4 BLOCKS OF 32KB
	{
		bytes_written = 0;
		file_size = 0x8000;	//set file size to 32 KB
		usb_byte_write();
		wait_status(0x1E, 3000);
		if(timeout_flag)
			return;

		while(1)
		{
			digitalWrite(10, LOW);
			SPI.transfer(0x2D); //WR_REQ_DATA command
			bytes_requested = SPI.transfer(0x00);

			while(bytes_requested)
			{
				SPI.transfer(get_test_byte());
				++ bytes_written;
				if(bytes_written == file_size)
					break;
				--bytes_requested;
			}
			digitalWrite(10, HIGH);
			wait_status(0x1E, 3000);
			if(timeout_flag)
				return;
			if(bytes_written == file_size)
				break;

			usb_byte_wr_go();
			wait_status(0x1E, 3000);
			if(timeout_flag)
				return;
		}
		usb_byte_wr_go();
		wait_status(0x14, 3000);
		if(timeout_flag)
			return;
	}

	usb_file_close();
	wait_status(0x14, 3000);
}

void usb_disk_capacity()
{	
	digitalWrite(10, LOW);
	SPI.transfer(0x3E); //disk capacity command
	digitalWrite(10, HIGH);

	wait_status(0x14, 100);
	if(timeout_flag)
		return;

	digitalWrite(10, LOW);
	SPI.transfer(0x27); //RD_USB_DATA0 command
	SPI.transfer(0x00); //get number of bytes (always 4)
	for(int d = 0; d < 4; ++d)
	{
		capacity = capacity >> 8;
		capacity = capacity | ((unsigned long)SPI.transfer(0x00) << 24);
	}
	digitalWrite(10, HIGH);
}

void usb_disk_query()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x3F); //disk query command
	digitalWrite(10, HIGH);

	wait_status(0x14, 100);
	if(timeout_flag)
		return;
	
	digitalWrite(10, LOW);
	SPI.transfer(0x27); //RD_USB_DATA0 command
	SPI.transfer(0x00); //get number of bytes (always 8)
	for(int d = 0; d < 4; ++d)
	{
		total_sectors = total_sectors >> 8;
		total_sectors = total_sectors | ((unsigned long)SPI.transfer(0x00) << 24);
	}
	for(int d = 0; d < 4; ++d)
	{
		free_sectors = free_sectors >> 8;
		free_sectors = free_sectors | ((unsigned long)SPI.transfer(0x00) << 24);
	}
	digitalWrite(10, HIGH);
}

void usb_reset_all()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x05); //reset all
	digitalWrite(10, HIGH);
}

void usb_set_file_name()
{
	Serial.println("input file name");
	while(Serial.available()<1)
	{

	}
	String fname = Serial.readString();
	fname.toUpperCase();  //the CH376S expects the name to be all caps.
	byte count = 0;
	digitalWrite(10, LOW);
	SPI.transfer(0x2f); //set file name
	SPI.transfer('/'); // "/" character at start of name
	while(fname[count] != NULL)
	{
		SPI.transfer(fname[count]);
		Serial.print(fname[count]);
		++count;
	}
	SPI.transfer(0x00);
	digitalWrite(10, HIGH);
	Serial.print("\n");
	delay(1);
}

void usb_file_open()
{
	digitalWrite(10, LOW);  //enable chip
	SPI.transfer(0x32); //file open
	digitalWrite(10, HIGH);
}

void usb_file_close()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x36); //file close command
	SPI.transfer(0x01);  //update length or not
	digitalWrite(10, HIGH);
}

void usb_byte_read()
{
	byte* size = (byte*)((void*)&file_size);
	
	digitalWrite(10, LOW);
	SPI.transfer(0x3a); //byte read command
	SPI.transfer(size[0]);
	SPI.transfer(size[1]);
	digitalWrite(10, HIGH);
}

void usb_byte_read_go()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x3b);
	digitalWrite(10, HIGH);
}

void usb_read_data0()
{
	uint8_t n_bytes = 0;
	digitalWrite(10, LOW);
	SPI.transfer(0x27); //RD_USB_DATA0 command
	n_bytes = SPI.transfer(0x00);
	bytes_read = bytes_read + n_bytes;

	while(n_bytes)
	{
		Serial.print((char)SPI.transfer(0x00));
		--n_bytes;
	}
	
	digitalWrite(10, HIGH);
	Serial.print("\n");
}

void usb_file_read()
{
	byte n_bytes = 0;
	bytes_read = 0;

	usb_file_open();
	wait_status(0x14, 100);
	if(timeout_flag)
		return;

	usb_get_file_size();
	wait_status(0x14, 100);
	if(timeout_flag)
		return;
	if(file_size == 0 || file_size > 65535)
	{
		Serial.println("Got bad file size");
		return;
	}

	usb_byte_read();
	wait_status(0x1D, 100);
	if(timeout_flag)
		return;

	while(1)
	{
		usb_read_data0();
		Serial.print(bytes_read, DEC);
		Serial.print("/");
		Serial.println(file_size, DEC);
		if(bytes_read == file_size)
			break;
		usb_byte_read_go();
		wait_status(0x1D, 255);
		if(timeout_flag)
			return;
	}

	usb_file_close();
	wait_status(0x14, 100);
}

void usb_get_file_size()
{
	byte* size = (byte*)((void*)&file_size);
	
	digitalWrite(10, LOW);
	SPI.transfer(0x0c); //get file size
	SPI.transfer(0x68); //command data
	size[0] = SPI.transfer(0x00);
	size[1] = SPI.transfer(0x00);
	size[2] = SPI.transfer(0x00);
	size[3] = SPI.transfer(0x00);
	digitalWrite(10, HIGH);
}

byte usb_check_exist(byte test_data)
{
	byte temp;
	digitalWrite(10, LOW);
	SPI.transfer(0x06);			//check exist
	SPI.transfer(test_data);	//dummy data
	temp = SPI.transfer(0x00);	//should get inverted dummy data
	digitalWrite(10, HIGH);
	return temp;
}

byte usb_get_version()
{
	byte temp;
	digitalWrite(10, LOW);
	SPI.transfer(0x01); //get version
	temp = SPI.transfer(0x00);
	digitalWrite(10, HIGH);
	return temp;
}

void usb_set_mode(byte mode)
{
	digitalWrite(10, LOW);
	SPI.transfer(0x15);	//set USB mode
	SPI.transfer(mode);
	digitalWrite(10, HIGH);
}

void usb_disk_connect()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x30); //disk connect
	digitalWrite(10, HIGH);
}

void usb_disk_mount()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x31); //disk mount
	digitalWrite(10, HIGH);
}

byte usb_get_status()
{
	byte temp;
	digitalWrite(10, LOW);
	SPI.transfer(0x22); //get status
	temp = SPI.transfer(0x00);
	digitalWrite(10, HIGH);
	return temp;
}

void usb_file_enum_go()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x33); //file enum go (CMD_FILE_ENUM_GO)
	digitalWrite(10, HIGH);
}

// Enumerate the contents of the current directory
void usb_enumerate()
{
	uint8_t n_bytes = 0;
	digitalWrite(10, LOW);
	SPI.transfer(0x2f); // CMD_SET_FILE_NAME
	SPI.transfer('/');
	SPI.transfer('*');
	SPI.transfer(0x00);
	digitalWrite(10, HIGH);

	usb_file_open();
	
	while(1)
	{
		wait_status(0x1D, 100); // Wait for USB_INT_DISK_READ
		if(timeout_flag)
			break;

		digitalWrite(10, LOW);
		SPI.transfer(0x27); //RD_USB_DATA0 command
		n_bytes = SPI.transfer(0x00);

		for(unsigned int d = 0; (d < 11) && n_bytes; ++d)
		{
			Serial.print((char)SPI.transfer(0x00));
			--n_bytes;
		}
		
		--n_bytes;
		if(SPI.transfer(0x00) & 0x10)
			Serial.print(F(" (DIR)"));
		else
			Serial.print(F(" (FILE)"));
		
		while(n_bytes)
		{
			SPI.transfer(0x00);
			--n_bytes;
		}
		
		digitalWrite(10, HIGH);
		Serial.print(F("\n"));
	
		usb_file_enum_go();
	}
}
