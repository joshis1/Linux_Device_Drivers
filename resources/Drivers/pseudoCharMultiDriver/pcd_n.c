#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
/** for copy_to_user and copy_from_user **/
#include <linux/uaccess.h> 
/** For IS_ERR(), PTR_ERR(), and ERR_PTR() **/
#include <linux/err.h>

#define NO_OF_DEVICES        (4)
#define MEM_SIZE_MAX_PCDEV1 (1024)
#define MEM_SIZE_MAX_PCDEV2 (512)
#define MEM_SIZE_MAX_PCDEV3 (1024)
#define MEM_SIZE_MAX_PCDEV4 (512)

char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

/**Pseudo device specific private data **/
struct pcdev_private_data
{
  char *buffer;
  unsigned size;
  const char *serial_number;
  int perm;
  struct cdev cdev_device;
};

/**Pseudo driver private data **/
struct pcdrv_private_data
{
  int total_devices;
  dev_t device_number;
  struct class *class_pcd;
  struct device *device_pcd;
  struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv_data = 
{
  .total_devices = NO_OF_DEVICES,
  .pcdev_data = {
   [0] = {
           .buffer =  device_buffer_pcdev1,
	   .size = MEM_SIZE_MAX_PCDEV1,
	   .serial_number = "PCDEV1XYZ123",
           .perm = 0x1, /*RDONLY */
	 },
   
   [1] = {
	   .buffer = device_buffer_pcdev2,
	   .size = MEM_SIZE_MAX_PCDEV2,
	   .serial_number = "PCDEV2XYZ123",
	   .perm = 0x10, /*WRONLY*/
	 },

   [2] = {
	   .buffer = device_buffer_pcdev3,
	   .size = MEM_SIZE_MAX_PCDEV3,
	   .serial_number = "PCDEV3XYZ123",
           .perm = 0x11, /*RDWR*/
	 },

   [3] = {
	   .buffer = device_buffer_pcdev4,
	   .size = MEM_SIZE_MAX_PCDEV4,
	   .serial_number = "PCDEV4XYZ123",
	   .perm = 0x11, /*RDWR */
	  },
  },

};


loff_t pcd_lseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *flip);

#define DEV_SIZE  (256)
char device_memory[DEV_SIZE] = "Hello World \n";


/**Trick to get the function name **/
#undef pr_fmt
#define pr_fmt(fmt) "%s : "fmt,__func__

struct file_operations pcd_fops =
{
  .open = pcd_open,
  .release = pcd_release,
  .read = pcd_read,
  .write = pcd_write,
  .llseek = pcd_lseek,
  .owner = THIS_MODULE

};

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init pcd_driver_init(void)
{
   int i = 0;
   int ret;
   /** 1. Dynamically allocate a device number **/
   ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcd_devices");
   if( ret < 0){
	   pr_err("Alloc chrdev failed\n");
	   goto out;
   }

   /*create device class under /sys/class */
   pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
   if(IS_ERR(pcdrv_data.class_pcd)) {
	  pr_err("Error creating class\n");
	  ret = PTR_ERR(pcdrv_data.class_pcd);
	  goto cdev_del;
   }  

   
   for( i = 0; i < NO_OF_DEVICES; i++){

   pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));

   /*Initialize the cdev structure with file operations*/
   cdev_init(&pcdrv_data.pcdev_data[i].cdev_device, &pcd_fops);

   /*Register a device ( cdev stucture) with VFS */ 
   pcdrv_data.pcdev_data[i].cdev_device.owner = THIS_MODULE;
   ret= cdev_add(&pcdrv_data.pcdev_data[i].cdev_device, pcdrv_data.device_number + i, 1);
   if(ret < 0){
          pr_err("Cdev add failed\n");
	  goto unreg_chrdev;
   }
   
  
   /*populate the sysfs with device information */
   pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number + i, NULL, "pcd-%d",i+1);
   /** IS_ERR(), PTR_ERR() and ERR_PTR() defined in /linux/err.h **/
   /** PTR_ERR - > converts pointer to error number value **/
   /** ERR_PTR() -> converts the error number to the pointer**/
   if(IS_ERR(pcdrv_data.device_pcd)) {
	   pr_err("Device Create failed\n");
	   ret = PTR_ERR(pcdrv_data.device_pcd);
	   goto class_del;
    }
 }

   pr_info("Module init was successful\n");

   return 0;


 
cdev_del:
   for(;i>=0;i--){
   device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
   cdev_del(&pcdrv_data.pcdev_data[i].cdev_device); 
   }
class_del:
   class_destroy(pcdrv_data.class_pcd);

unreg_chrdev:
   unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);
out:
   pr_err("Module init failed\n");
   return ret;  
}

/** Called when the module is removed - rmmod**/
static void __exit pcd_driver_cleanup(void)
{
   int i;
   for(i = 0; i < NO_OF_DEVICES; i++) {
       device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
       cdev_del(&pcdrv_data.pcdev_data[i].cdev_device);
   }

   class_destroy(pcdrv_data.class_pcd);
   unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);
   pr_info("module unloaded\n");
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
#if 0
  loff_t temp;
  pr_info("lseek was invoked count = %llu and fpos = %lld\n",offset, filp->f_pos);

  switch(whence) {
     case SEEK_SET:
	    temp = offset;
            if((offset  > DEV_SIZE)  || (offset < 0)) {
               return -EINVAL;
            }
            filp->f_pos = offset;       
	    break; 
    case SEEK_CUR:
           temp = filp->f_pos + offset;
           if( (temp > DEV_SIZE) || temp < 0) {
             return -EINVAL;
           }
           filp->f_pos = temp;
	   break;
    case SEEK_END:
	   temp = offset + DEV_SIZE;
	   if( (temp > DEV_SIZE) || temp < 0) {
            return -EINVAL;
           }
	   filp->f_pos = temp;
	  break; 
   }

  pr_info("New f_pos = %lld\n",filp->f_pos);
  return filp->f_pos;
 #endif 
  return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
 #if 0
  loff_t temp;
  int ret;
  pr_info("read was invoked count = %zu and f_pos = %lld\n",count, *f_pos);
  pr_info("Fpos = %llu \n", *f_pos);

  temp = count + (*f_pos);
   
   if( temp > (DEV_SIZE - *(f_pos)))
   {
      pr_info("Copying the data -- count is more than required\n");	
      count = DEV_SIZE - *f_pos;   
      ret = copy_to_user(buff, &device_memory[*f_pos], count);
      if( ret  < 0) {
	pr_err("Copy to user failed = %d\r\n",ret);      
        return -EFAULT;
      }
      *f_pos = DEV_SIZE;
      pr_info("New f_pos = %llu and bytes written = %zu\n",*f_pos, count);
      return count;
   }
   
   else
    {
       pr_info("Copy the remaining data\n");	    
       ret = copy_to_user(buff, &device_memory[*f_pos], count);
       if( ret < 0) {
	 pr_err("Copy to user failed = %d\r\n",ret);      
         return -EFAULT;
       }
       *f_pos +=count;
       pr_info("New f_pos = %llu\n",*f_pos);
       return count;
    }
 #endif
  return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
#if 0
  loff_t temp;
  int ret;
  pr_info("write was invoked count = %zu and f_pos = %lld\n",count,*f_pos);

  temp = count + (*f_pos);
   
  if( temp > (DEV_SIZE - *(f_pos))) {
      pr_info("Writing the data --over flow\n");
      return -ENOMEM;  /*check out of memory error code.*/
   }
   
   else {
       pr_info("Write the remaining data\n");	    
       ret = copy_from_user(&device_memory[*f_pos], buff, count);
       if(ret < 0) {
	pr_err("Copy from user failed = %d\r\n", ret);       
        return -EFAULT;
       }

       *f_pos +=count;
       pr_info("Updated f_pos = %lld\n", *f_pos);
       return count;
    }

  pr_info("Updated f_pos = %lld\n", *f_pos);
#endif 
  return 0;
}

int pcd_open(struct inode *inode, struct file *filp)
{
  pr_info("open was invoked\n");
  return 0;
}

int pcd_release(struct inode *inode, struct file *flip)
{
  pr_info("release was invoked\n");
  return 0;
}

/**module registration **/
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

/*GPL - check module.h - very important */
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Pseudo Character Multi Driver");
MODULE_INFO(board, "Zedboard - AVNET");

