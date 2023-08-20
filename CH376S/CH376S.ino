#include <SPI.h>
#include "core.h"
#include "support.h"

void usb_print_disk_info()
{
	Serial.print(F("capacity in KBs: "));
	Serial.println(capacity >> 1, DEC);	//convert sectors to KB
	Serial.print(F("logical KBs: "));
	Serial.println(total_sectors >> 1, DEC);
	Serial.print(F("free KBs: "));
	Serial.println(free_sectors >> 1, DEC);
}

void usb_autoconfig()	//TODO: try to get rid of the fixed delays...
{
	Serial.print(F("check exist: "));
	print_hex_byte(usb_check_exist(0x55));
	Serial.print(F("get version: "));
	print_hex_byte(usb_get_version());
	usb_set_mode(0x05);
	delay(1);
	usb_set_mode(0x07);
	delay(1);
	usb_set_mode(0x06);
	delay(1);
	usb_disk_connect();
	delay(500);
	usb_disk_mount();
	delay(500);
	print_hex_byte(usb_get_status());
	usb_disk_capacity();
	usb_disk_query();
	usb_print_disk_info();
}


void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	SPI.begin();
	SPI.setClockDivider(2);
	SPI.setDataMode(SPI_MODE0);
	pinMode(10, OUTPUT);

	random_init(0);

	usb_reset_all();
	delay(100);
	Serial.print("\n");
	usb_autoconfig();
}

void test()
{
	file_size = 0xDEADBEEF;
	byte* size = (byte*)((void*)&file_size);
	print_hex_byte(size[0]);
	print_hex_byte(size[1]);
	print_hex_byte(size[2]);
	print_hex_byte(size[3]);

	size[0] = 0x55;
	size[1] = 0x00;
	size[2] = 0x00;
	size[3] = 0xAA;
	Serial.println(file_size, HEX);
}

void loop()
{
	switch(get_hex())
	{
		case 0x00:
			Serial.print(F("check exist: "));
			print_hex_byte(usb_check_exist(0x55));
			break;
		case 0x01:
			Serial.print(F("get version: "));
			print_hex_byte(usb_get_version());
			break;
		case 0x02:
			Serial.println(F("set USB mode 5"));
			usb_set_mode(0x05);
			break;
		case 0x03:
			Serial.println(F("set USB mode 7"));
			usb_set_mode(0x07);
			break;
		case 0x04:
			Serial.println(F("set USB mode 6"));
			usb_set_mode(0x06);
			break;
		case 0x05:
			Serial.println(F("CMD_DISK_CONNECT"));
			usb_disk_connect();
			break;
		case 0x06:
			Serial.println(F("CMD_DISK_MOUNT"));
			usb_disk_mount();
			break;
		case 0x07:
			Serial.print(F("get status: "));
			print_hex_byte(usb_get_status());
			break;
		case 0x08:
			Serial.println(F("usb set file name: "));
			usb_set_file_name();
			break;
		case 0x09:
			Serial.println(F("CMD_FILE_OPEN"));
			usb_file_open();
			break;
		case 0x0A:
			Serial.print(F("CMD_GET_FILE_SIZE: "));
			usb_get_file_size();
			Serial.println(file_size, DEC);
			break;
		case 0x0B:
			Serial.println(F("CMD_FILE_CLOSE"));
			usb_file_close();
			break;
		case 0x0C:
			Serial.println(F("CMD_BYTE_READ"));
			usb_byte_read();
			break;
		case 0x0D:
			Serial.println(F("CMD_BYTE_RD_GO"));
			usb_byte_read_go();
			break;
		case 0x0E:
			Serial.println(F("CMD_RD_USB_DATA0"));
			usb_read_data0();
			break;
		case 0x0F:
			Serial.println(F("USB Autoconfig:"));
			usb_autoconfig();
			break;
		case 0x10:
			Serial.println(F("USB reset all"));
			usb_reset_all();
			break;
		case 0x11:
			Serial.println(F("usb_file_read:"));
			usb_file_read();
			break;
		case 0x12:
			Serial.println(F("CMD_DISK_CAPACITY (KB)"));
			usb_disk_capacity();
			Serial.println(capacity >> 1, DEC);	//convert sectors to KB
			break;
		case 0x13:
			Serial.println(F("CMD_DISK_QUERY"));
			usb_disk_query();
			Serial.print(F("logical KBs: "));
			Serial.println(total_sectors >> 1, DEC);
			Serial.print(F("free KBs: "));
			Serial.println(free_sectors >> 1, DEC);
			break;
		case 0x14:
			Serial.println(F("print test data: "));
			print_test_data();
			break;
		case 0x15:
			Serial.println(F("CMD_FILE_CREATE"));
			usb_file_create();
			break;
		case 0x16:
			Serial.println(F("write USB test data"));
			usb_write_test_data();
			break;
		case 0x17:
			Serial.println(F("file enum go"));
			usb_file_enum_go();
			break;
		case 0x18:
			Serial.println(F("usb enumerate"));
			usb_enumerate();
			break;
		case 0xFF:
			Serial.println(F("test: "));
			test();
			break;
	}
	delay(100);
	Serial.println(F("Ready."));
}
