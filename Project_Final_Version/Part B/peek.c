#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>


//Global variable for memory location
char mem_location_peek[8];
char data_buffer_peek[1024];
char storage_buffer_peek[8];
char *mem_pointer_peek = &mem_location_peek[0];
char *peek_pointer = &data_buffer_peek[0];
int buffer_counter = 0;
int storage_counter = 0;

static int open_sesame(struct inode* inode_pointer, struct file* file_pointer) {
  printk(KERN_INFO "Opened");
  return 0;
}

static int bye_sesame(struct inode* inode_pointer, struct file* file_pointer) {
  printk(KERN_INFO "Closed");
  return 0;
}

static ssize_t read_sesame(struct file *file, char *data, size_t length, loff_t *offset_in_file){
   // retrieves 1 byte from location & sends it to user
   copy_to_user(data, mem_pointer_peek, 1);
   unsigned long* peek_data = *((unsigned long*)mem_location_peek); 
   printk(KERN_INFO "peek data is in hex: %lx", peek_data[0]);
   printk(KERN_INFO "Data is at memory location: %lx \n ", *((unsigned long*)mem_location_peek));
return 1;
}

static ssize_t write_sesame(struct file *file, const char *data, size_t length, loff_t *offset_in_file){
copy_from_user(data_buffer_peek, data, length);							// Copy user data to kernel 
for (buffer_counter = 0; buffer_counter < length; buffer_counter++)				// Traverse copied user data 
{
  storage_buffer_peek[storage_counter] = data_buffer_peek[buffer_counter];
  //printk(KERN_INFO "peek storage data is : %x", storage_buffer_peek[storage_counter]);	// For testing purposes
  storage_counter++;										// Increment storage buffer index
  if (storage_counter == 8)						
  { 
    memcpy(mem_location_peek, storage_buffer_peek, 8);						// Save storage buffer when full (8 byte) to mem location
    printk(KERN_INFO "mem location peek: %lx", *((unsigned long*)mem_location_peek));		// Print mem location address to kernel
    storage_counter = 0;									// Reset storage buffer to 0 
  }
}

return 0;
}

static struct device* device_data;
static struct class* class_stuff;

static struct file_operations file_ops =
{
   .open = open_sesame,
   .release = bye_sesame,
   .read = read_sesame,
   .write = write_sesame,
};

static int __init peek_init(void) {

   int major = register_chrdev(0, "peek", &file_ops);
   class_stuff = class_create(THIS_MODULE, "peek class");
   device_data = device_create(class_stuff, NULL, MKDEV(major, 0), NULL,
			       "peek dev");
 // printk(KERN_INFO "HI!\n");

 // takes a pointer
 // using the 8 byte pointer, retrieves 1 byte from the location 
 // sends the read byte to user, print
  return 0;
}

static void __exit peek_exit(void) {
  printk(KERN_INFO "Exiting peek\n");
}

module_init(peek_init);
module_exit(peek_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SY");
MODULE_DESCRIPTION("A simple driver that performs a complete write using a pointer and reads 1 byte from the memory location");
