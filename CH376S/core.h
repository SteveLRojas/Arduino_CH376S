extern byte timeout_flag;
extern unsigned long file_size;	//file size in bytes
extern unsigned long capacity;	//drive capacity in sectors
extern unsigned long total_sectors;	//total sectors in partition
extern unsigned long free_sectors;	//free sectors in partition

byte wait_status(byte target, uint16_t timeout);
void usb_file_create();
void usb_byte_write(unsigned int length);
void usb_byte_wr_go();
void usb_write_test_data();
void usb_disk_capacity();
void usb_disk_query();
void usb_reset_all();
void usb_set_file_name();
void usb_file_open();
void usb_file_close();
void usb_byte_read();
void usb_byte_read_go();
void usb_read_data0();
void usb_file_read();
void usb_get_file_size();
byte usb_check_exist(byte);
byte usb_get_version();
void usb_set_mode(byte);
//void usb_set_mode_5();
//void usb_set_mode_7();
//void usb_set_mode_6();
void usb_disk_connect();
void usb_disk_mount();
byte usb_get_status();
