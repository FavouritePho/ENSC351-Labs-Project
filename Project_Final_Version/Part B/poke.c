#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

// Global Variables
char mem_location_poke[8];
char data_buffer_poke[1024];
char* poke_pointer = &data_buffer_poke[0];
char storage_buffer_poke[9];
char* mem_pointer_poke = &mem_location_poke[0];
int buffer_count = 0;			
int storage_count = 0;

static int open_sesame(struct inode* inode_pointer, struct file* file_pointer) {
  printk(KERN_INFO "Opened");
  return 0;
}

static int bye_sesame(struct inode* inode_pointer, struct file* file_pointer) {
  printk(KERN_INFO "Closed");
  return 0;
}

static ssize_t read_sesame(struct file *file, char *data, size_t length, loff_t *offset_in_file){
  return 0;
}

static ssize_t write_sesame(struct file *file, const char *data, size_t length, loff_t *offset_in_file){
copy_from_user(data_buffer_poke, data, length);         			// Copy user data to kernel
//printk(KERN_INFO "memory location : %lx",*((unsigned long*)data_buffer_poke));// For testing purpose
for (buffer_count = 0; buffer_count < length; buffer_count++)			// Traverse the data buffer byte by byte
{
  storage_buffer_poke[storage_count] = data_buffer_poke[buffer_count];
  //printk(KERN_INFO "poke storage data is : %x", storage_buffer_poke[storage_count]);	// For testing purposes
  storage_count++;								// Increment storage buffer index
  if (storage_count == 9)							// Save storage buffer when full (9 byte)
  {
   memcpy(mem_location_poke, storage_buffer_poke, 8);   			// Save mem location
   unsigned long p = *((unsigned long*) mem_location_poke);       	       
   unsigned long* write_data = p;						// Copy mem location
   *write_data = storage_buffer_poke[8];    	                		// Write 9th byte from storage buffer to memory location
   printk(KERN_INFO "written poke data is : %lx", *write_data);			// Print mem location address to kernel
   storage_count = 0;					        		// Reset storage buffer index
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

static int __init poke_init(void) {

   int major = register_chrdev(0, "Poke", &file_ops);
   class_stuff = class_create(THIS_MODULE, "Poke class");
   device_data = device_create(class_stuff, NULL, MKDEV(major, 0), NULL,
			       "poke dev");
  // printk(KERN_INFO "HI!\n");
  // takes 9 bytes
  // segments 9 bytes into 8 byte and 1 byte chunks
  // first 8 bytes are a pointer
  // last byte is data
  // data is written to the memory address given by the pointer
  return 0;
}

static void __exit poke_exit(void) {
  printk(KERN_INFO "BYE!\n");
}

module_init(poke_init);
module_exit(poke_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SY");
MODULE_DESCRIPTION("A simple driver uses a 9 byte complete write, 8 bytes are a pointer, 1 byte is data written to memory address");
