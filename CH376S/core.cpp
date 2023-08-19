#include <SPI.h>
#include "core.h"
#include "support.h"

byte c_param = 0;
byte read_len_high = 0;
byte read_len_low = 0;
char read_buff[256];
unsigned long free_space;

void wait_status(byte target)
{
	int timeout = 3000;
	while ((usb_return_status() != target) && (timeout != 0))
	{
		delay(1);
		timeout--;
	}
	if (timeout == 0)
		Serial.println("ERR: Timeout");
}

void usb_file_create()
{
	Serial.println("CMD_FILE_CREATE");
	digitalWrite(10, LOW);
	SPI.transfer(0x34); //file create command
	digitalWrite(10, HIGH);
}

void usb_byte_write(unsigned int length)
{
	Serial.println("CMD_BYTE_WRITE");
	digitalWrite(10, LOW);
	SPI.transfer(0x3c); //byte write command
	SPI.transfer(length & 0xff);  //length low byte
	SPI.transfer((length >> 8) & 0xff); //length high byte
	digitalWrite(10, HIGH);
}

void usb_byte_wr_go()
{
	//Serial.println("CMD_BYTE_WR_GO");
	digitalWrite(10, LOW);
	SPI.transfer(0x3d);
	digitalWrite(10, HIGH);
}

void usb_write_test_data()
{
	unsigned long bytes_written = 0;
	byte bytes_requested;
	Serial.println("write USB test data");
	//usb_disk_query();
	if(free_space < 0x20000)
	{
		Serial.println("Not enough space");
		return;
	}
	bitfield_16.value = 0;
	usb_file_create();
	wait_status(0x14);
	usb_byte_write(0xffff);
	wait_status(0x1e);
	//wr_req_data command
	digitalWrite(10, LOW);
	SPI.transfer(0x2d); //WR_REQ_DATA command
	bytes_requested = SPI.transfer(NULL);
	for(int d = 0; d < bytes_requested; ++d)
	{
		SPI.transfer(get_test_byte());
		++ bytes_written;
	}
	digitalWrite(10, HIGH);
	wait_status(0x1e);
	while(bytes_written != 0x20000)
	{
		usb_byte_wr_go();
		wait_status(0x1e);
		if(usb_return_status() == 0x14)
		{
			usb_byte_write(((0x20000 - bytes_written) <= 0xffff) ? (0x20000 - bytes_written) : (0xffff));
			wait_status(0x1e);
			if(usb_return_status() != 0x1e)
				break;
		}
		//wr_req_data command
		digitalWrite(10, LOW);
		SPI.transfer(0x2d); //WR_REQ_DATA command
		bytes_requested = SPI.transfer(NULL);
		for(int d = 0; d < bytes_requested; ++d)
		{
			SPI.transfer(get_test_byte());
			++ bytes_written;
			if(bytes_written == 0x20000)
				break;
		}
		digitalWrite(10, HIGH);
		wait_status(0x1e);
		//Serial.println(bytes_written, HEX);
	}
	usb_byte_wr_go();
	wait_status(0x14);
	usb_get_status();
	c_param = 1;
	usb_file_close();
	c_param = 0;
	wait_status(0x14);
	usb_get_status();
}

void usb_disk_capacity()
{
	unsigned long capacity;
	Serial.println("CMD_DISK_CAPACITY");
	digitalWrite(10, LOW);
	SPI.transfer(0x3E); //disk capacity command
	digitalWrite(10, HIGH);
	delay(10);
	digitalWrite(10, LOW);
	SPI.transfer(0x27); //RD_USB_DATA0 command
	SPI.transfer(NULL); //get number of bytes (always 4)
	for(int d = 0; d < 4; ++d)
	{
		capacity = capacity >> 8;
		capacity = capacity | ((unsigned long)SPI.transfer(NULL) << 24);
	}
	digitalWrite(10, HIGH);
	Serial.println(capacity * 512, DEC);
	delay(1);
}

void usb_disk_query()
{
	unsigned long total_sectors, free_sectors;
	Serial.println("CMD_DISK_QUERY");
	digitalWrite(10, LOW);
	SPI.transfer(0x3F); //disk query command
	digitalWrite(10, HIGH);
	delay(100);
	digitalWrite(10, LOW);
	SPI.transfer(0x27); //RD_USB_DATA0 command
	SPI.transfer(NULL); //get number of bytes (always 8)
	for(int d = 0; d < 4; ++d)
	{
		total_sectors = total_sectors >> 8;
		total_sectors = total_sectors | ((unsigned long)SPI.transfer(NULL) << 24);
	}
	for(int d = 0; d < 4; ++d)
	{
		free_sectors = free_sectors >> 8;
		free_sectors = free_sectors | ((unsigned long)SPI.transfer(NULL) << 24);
	}
	digitalWrite(10, HIGH);
	free_space = free_sectors * 512;
	Serial.print("logical bytes: ");
	Serial.println(total_sectors * 512, DEC);
	Serial.print("free bytes: ");
	Serial.println(free_sectors * 512, DEC);
	delay(1);
}

void usb_reset_all()
{
	Serial.println("USB reset all");
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
	SPI.transfer(0x2f); // "/" character at start of name
	while(fname[count]!=NULL)
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
	Serial.println("CMD_FILE_OPEN");
	digitalWrite(10, LOW);  //enable chip
	SPI.transfer(0x32); //file open
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_file_close()
{
	Serial.println("CMD_FILE_CLOSE");
	digitalWrite(10, LOW);
	SPI.transfer(0x36); //file close command
	SPI.transfer(c_param);  //update length or not
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_byte_read()
{
	Serial.println("CMD_READ_BYTE");
	digitalWrite(10, LOW);
	SPI.transfer(0x3a); //byte read command
	SPI.transfer(read_len_low);
	SPI.transfer(read_len_high);
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_byte_read_go()
{
	Serial.println("CMD_BYTE_RD_GO");
	digitalWrite(10, LOW);
	SPI.transfer(0x3b);
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_read_data0()
{
	byte n_bytes = 0;
	Serial.println("CMD_RD_USB_DATA0");
	digitalWrite(10, LOW);
	SPI.transfer(0x27); //RD_USB_DATA0 command
	n_bytes = SPI.transfer(NULL);
	for(int d = 0; d < n_bytes; ++d)
	{
		read_buff[d] = SPI.transfer(NULL);
		Serial.print(read_buff[d]);
	}
	digitalWrite(10, HIGH);
	Serial.print("\n");
	delay(1);
}

void usb_file_read()
{
	Serial.println("usb_file_read:");
	unsigned int bytes_read = 0;
	unsigned int file_size = 0;
	byte n_bytes = 0;
	byte size_0, size_1, size_2, size_3;

	//usb file open command
	digitalWrite(10, LOW);  //enable chip
	SPI.transfer(0x32); //file open
	digitalWrite(10, HIGH);
	//delay(10);
	//usb_get_status();

	int timeout = 100;
	while ((usb_return_status() != 0x14) && (timeout != 0))
	{
		delay(1);
		timeout--;
	}
	if (timeout == 0)
	{
		Serial.println("ERR: Timeout\n");
		return;
	}

	//get file size command
	digitalWrite(10, LOW);
	SPI.transfer(0x0c); //get file size
	SPI.transfer(0x68); //command data
	size_0 = SPI.transfer(0x00);
	size_1 = SPI.transfer(0x00);
	size_2 = SPI.transfer(0x00);
	size_3 = SPI.transfer(0x00);
	digitalWrite(10, HIGH);
	file_size = (size_1 << 8)|size_0;
	//delay(10);
	//usb_get_status();
	timeout = 100;
	while ((usb_return_status() != 0x14) && (timeout != 0))
	{
		delay(1);
		timeout--;
	}
	if (timeout == 0)
	{
		Serial.println("ERR: Timeout\n");
		return;
	}

	//send the byte read command
	digitalWrite(10, LOW);
	SPI.transfer(0x3a); //byte read command
	SPI.transfer(size_0);
	SPI.transfer(size_1);
	digitalWrite(10, HIGH);
	timeout = 100;
	while ((usb_return_status() != 0x1D) && (timeout != 0))
	{
		delay(1);
		timeout--;
	}
		if (timeout == 0)
	{
		Serial.println("ERR: Timeout\n");
		return;
	}

	//send usb read data0 command
	digitalWrite(10, LOW);
	SPI.transfer(0x27); //RD_USB_DATA0 command
	n_bytes = SPI.transfer(NULL);
	for(int d = 0; d < n_bytes; ++d)
	{
		read_buff[d] = SPI.transfer(NULL);
		Serial.print(read_buff[d]);
		++bytes_read;
	}
	digitalWrite(10, HIGH);
	delay(1);
	//Serial.println(bytes_read, HEX);
	//Serial.println(file_size, HEX);
	while(bytes_read < file_size)
	{
		Serial.print(bytes_read, HEX);
		Serial.print("/");
		Serial.println(file_size, HEX);
		usb_byte_read_go();
		delay(100);
		digitalWrite(10, LOW);
		SPI.transfer(0x27); //RD_USB_DATA0 command
		n_bytes = SPI.transfer(NULL);
		for(int d = 0; d < n_bytes; ++d)
		{
			read_buff[d] = SPI.transfer(NULL);
			Serial.print(read_buff[d]);
			++bytes_read;
		}
		digitalWrite(10, HIGH);
		delay(1);
	}
	Serial.print("\n");

	usb_file_close();
}

void usb_get_file_size()
{
	byte size_0, size_1, size_2, size_3;
	Serial.print("CMD_GET_FILE_SIZE: ");
	digitalWrite(10, LOW);
	SPI.transfer(0x0c); //get file size
	SPI.transfer(0x68); //command data
	size_0 = SPI.transfer(0x00);
	size_1 = SPI.transfer(0x00);
	size_2 = SPI.transfer(0x00);
	size_3 = SPI.transfer(0x00);
	digitalWrite(10, HIGH);
	Serial.print(size_3, HEX);
	Serial.print(size_2, HEX);
	Serial.print(size_1, HEX);
	Serial.println(size_0, HEX);
	read_len_high = size_1;
	read_len_low = size_0;
}

void usb_check_exist()
{
	Serial.print("check exist: "); 
	digitalWrite(10, LOW);
	SPI.transfer(0x06); //check exist
	SPI.transfer(0x55); //dummy data
	Serial.println(SPI.transfer(0x00), HEX);  //should print inverted dummy data
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_get_version()
{
	Serial.print("get version: ");
	digitalWrite(10, LOW);
	SPI.transfer(0x01); //get version
	Serial.println(SPI.transfer(0x00), HEX);
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_set_mode_5()
{
	Serial.println("set USB mode 5");
	digitalWrite(10, LOW);
	SPI.transfer(0x15); //set USB mode
	SPI.transfer(0x05); //mode 5
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_set_mode_7()
{
	Serial.println("set USB mode 7");
	digitalWrite(10, LOW);
	SPI.transfer(0x15); //set USB mode
	SPI.transfer(0x07); //mode 7
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_set_mode_6()
{
	Serial.println("set USB mode 6");
	digitalWrite(10, LOW);
	SPI.transfer(0x15); //set USB mode
	SPI.transfer(0x06); //mode 6
	digitalWrite(10, HIGH);
	delay(1);
}

void usb_disk_connect()
{
	Serial.println("CMD_DISK_CONNECT");
	digitalWrite(10, LOW);
	SPI.transfer(0x30); //disk connect
	digitalWrite(10, HIGH);
	delay(500);
}

void usb_disk_mount()
{
	Serial.println("CMD_DISK_MOUNT");
	digitalWrite(10, LOW);
	SPI.transfer(0x31); //disk mount
	digitalWrite(10, HIGH);
	delay(500);
}

void usb_get_status()
{
	Serial.print("get status: ");
	digitalWrite(10, LOW);
	SPI.transfer(0x22); //get status
	Serial.println(SPI.transfer(0x00), HEX);
	//Serial.println(SPI.transfer(0x00), HEX);  //is there more?
	digitalWrite(10, HIGH);
	delay(1);
}

byte usb_return_status()
{
	digitalWrite(10, LOW);
	SPI.transfer(0x22); //get status
	byte status = SPI.transfer(0x00);
	digitalWrite(10, HIGH);
	return status;
}
