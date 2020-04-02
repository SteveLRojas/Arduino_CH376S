#include <SPI.h>
byte c_param = 0;
byte read_len_high = 0;
byte read_len_low = 0;
char read_buff[256];
union Bitfield_16
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
} bitfield_16;

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

void ByteToHex(byte value, char* buff)
{
  char table[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};
  buff[0] = table[(value >> 4) & 0x0f];
  buff[1] = table[(value) & 0x0f];
  buff[2] = NULL;
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
    ByteToHex((current_word >> 8) & 0xff, buff);
    Serial.print(buff);
    ByteToHex(current_word & 0xff, buff);
    Serial.print(buff);
    if((counter & 0x0f) == 0x0f)
      Serial.print('\n');
    ++counter;
  } while(counter != 0);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
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
    }
  delay(100);
  Serial.println("Ready.");
  }
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
  Serial.print("logical bytes: ");
  Serial.println(total_sectors * 512, DEC);
  Serial.print("free bytes: ");
  Serial.println(free_sectors * 512, DEC);
  delay(1);
}

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
