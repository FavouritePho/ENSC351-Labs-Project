#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

// allocate memory 1024 bytes
char *ptr;
char* test;

static int open_sesame(struct inode* inode_pointer, struct file* file_pointer) {
  printk(KERN_INFO "Opened");
  return 0;
}

static int bye_sesame(struct inode* inode_pointer, struct file* file_pointer) {
  printk(KERN_INFO "Closed");
  return 0;
}

static ssize_t read_sesame(struct file *file, char *data, size_t length, loff_t *offset_in_file){
  // Copy memory to userspace, Output 8 bytes
  copy_to_user(data, &ptr, length);
  return 8;
}

static ssize_t write_sesame(struct file *file, const char *data, size_t length, loff_t *offset_in_file){
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

static int __init getptr_init(void) {

   int major = register_chrdev(0, "getptr", &file_ops);
   class_stuff = class_create(THIS_MODULE, "getptr class");
   device_data = device_create(class_stuff, NULL, MKDEV(major, 0), NULL, "getptr dev");
   ptr = kmalloc(1024, GFP_KERNEL);
   printk(KERN_INFO "HI!\n");
   return 0;
}

static void __exit getptr_exit(void) {
  printk(KERN_INFO "BYE\n");
}

module_init(getptr_init);
module_exit(getptr_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SY");
MODULE_DESCRIPTION("A simple driver that emits 8 byte strings that represent a pointer");
