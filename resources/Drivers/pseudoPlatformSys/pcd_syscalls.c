#include "pcd_syscalls.h"

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
  return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
   return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  return 0;
}

int pcd_open(struct inode *inode, struct file *filp)
{
  return 0;
}

int pcd_release(struct inode *inode, struct file *flip)
{
  pr_info("release was invoked\n");
  return 0;
}

#if 0
static int check_permission(fmode_t mode, int perm)
{
   if(perm == RDWR) {
      pr_info("Permission granted - RW\n");
      return 0;
   }
   else if((perm == RONLY) && (mode & FMODE_READ) && !(mode & FMODE_WRITE)) {
       pr_info("Permission granted - RONLY\n");
       return 0;
   }
   else if((perm == WRONLY) && (mode & FMODE_WRITE) && !(mode & FMODE_READ)) {
       pr_info("Permission granted - WRONLY\n");
       return 0;
   }
   else {
        return -EPERM;
    }
}
#endif