#include <SPI.h>
byte c_param = 0;
byte read_len_high = 0;
byte read_len_low = 0;
char read_buff[256];
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  pinMode(10, OUTPUT);

  digitalWrite(10, LOW);
  SPI.transfer(0x05); //reset all
  digitalWrite(10, HIGH);
  for(int d = 0; d<256; ++d)
  {
    read_buff[d] = NULL;
  }
  delay(100);
  Serial.print("\n");
  usb_check_exist();
  usb_get_version();
  usb_set_mode_5();
  usb_set_mode_7();
  usb_set_mode_6();
  usb_disk_connect();
  usb_disk_mount();
  usb_get_status();
}

void loop()
{
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
    }
  delay(500);
  Serial.println("Ready.");
  }
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
