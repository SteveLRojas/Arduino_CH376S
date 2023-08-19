#include <SPI.h>
#include "core.h"
#include "support.h"

void usb_autoconfig()
{
	Serial.println("USB Autoconfig:");
	usb_check_exist();
	usb_get_version();
	usb_set_mode_5();
	usb_set_mode_7();
	usb_set_mode_6();
	usb_disk_connect();
	usb_disk_mount();
	usb_get_status();
	usb_disk_query();
}


void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	SPI.begin();
	SPI.setClockDivider(2);
	SPI.setDataMode(SPI_MODE0);
	pinMode(10, OUTPUT);

	for(int d = 0; d<256; ++d)
	{
		read_buff[d] = NULL;
	}

	usb_reset_all();
	delay(100);
	Serial.print("\n");
	usb_autoconfig();
}

void loop()
{
	switch(get_hex())
	{
		case 0x00:
			usb_check_exist();
			break;
		case 0x01:
			usb_get_version();
			break;
		case 0x02:
			usb_set_mode_5();
			break;
		case 0x03:
			usb_set_mode_7();
			break;
		case 0x04:
			usb_set_mode_6();
			break;
		case 0x05:
			usb_disk_connect();
			break;
		case 0x06:
			usb_disk_mount();
			break;
		case 0x07:
			usb_get_status();
			break;
		case 0x08:
			usb_set_file_name();
			break;
		case 0x09:
			usb_file_open();
			break;
		case 0x0A:
			usb_get_file_size();
			break;
		case 0x0B:
			usb_file_close();
			break;
		case 0x0C:
			usb_byte_read();
			break;
		case 0x0D:
			usb_byte_read_go();
			break;
		case 0x0E:
			usb_read_data0();
			break;
		case 0x0F:
			usb_autoconfig();
			break;
		case 0x10:
			usb_reset_all();
			break;
		case 0x11:
			usb_file_read();
			break;
		case 0x12:
			usb_disk_capacity();
			break;
		case 0x13:
			usb_disk_query();
			break;
		case 0x14:
			print_test_data();
			break;
		case 0x15:
			usb_file_create();
			break;
		case 0x16:
			usb_write_test_data();
			break;
	}
	delay(100);
	Serial.println("Ready.");
}
