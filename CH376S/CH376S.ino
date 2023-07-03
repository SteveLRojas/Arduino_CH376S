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
  //test
  char hex[3];
  uint16_t test_word;
  test_word = get_hex();
  byte_to_hex(test_word, hex);
  Serial.println(hex);
  delay(1000);
  return;
  
  // put your main code here, to run repeatedly:
  if(Serial.available())
  {
    switch(Serial.parseInt())
    {
      case 0:
        usb_check_exist();
        break;
      case 1:
        usb_get_version();
        break;
      case 2:
        usb_set_mode_5();
        break;
      case 3:
        usb_set_mode_7();
        break;
      case 4:
        usb_set_mode_6();
        break;
      case 5:
        usb_disk_connect();
        break;
      case 6:
        usb_disk_mount();
        break;
      case 7:
        usb_get_status();
        break;
      case 8:
        usb_set_file_name();
        break;
      case 9:
        usb_file_open();
        break;
      case 10:
        usb_get_file_size();
        break;
      case 11:
        usb_file_close();
        break;
      case 12:
        usb_byte_read();
        break;
      case 13:
        usb_byte_read_go();
        break;
      case 14:
        usb_read_data0();
        break;
      case 15:
        usb_autoconfig();
        break;
      case 16:
        usb_reset_all();
        break;
      case 17:
        usb_file_read();
        break;
      case 18:
        usb_disk_capacity();
        break;
      case 19:
        usb_disk_query();
        break;
      case 20:
        print_test_data();
        break;
      case 21:
        usb_file_create();
        break;
      case 22:
        usb_write_test_data();
        break;
    }
  delay(100);
  Serial.println("Ready.");
  }
}
