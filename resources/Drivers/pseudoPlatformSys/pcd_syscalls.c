#include "pcd_syscalls.h"

static int check_permission(fmode_t mode, int perm);

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {
  struct pcdev_private_data *pcdev_data;
  loff_t temp;

  pcdev_data = (struct pcdev_private_data *)filp->private_data;
  pr_info("lseek was invoked count = %llu and fpos = %lld\n", offset,
          filp->f_pos);

  switch (whence) {
  case SEEK_SET:
    temp = offset;
    if ((offset > pcdev_data->pData.size) || (offset < 0)) {
      return -EINVAL;
    }
    pr_info("lseek --Whence= SEEK_SET and offset = %lld\n", offset);
    filp->f_pos = offset;
    break;
  case SEEK_CUR:
    temp = filp->f_pos + offset;
    if ((temp > pcdev_data->pData.size) || temp < 0) {
      return -EINVAL;
    }
    pr_info("lseek --Whence= SEEK_CUR and offset = %lld\n", temp);
    filp->f_pos = temp;
    break;
  case SEEK_END:
    temp = offset + pcdev_data->pData.size;
    if ((temp > pcdev_data->pData.size) || temp < 0) {
      return -EINVAL;
    }
    pr_info("lseek --Whence= SEEK_END and offset = %lld\n", temp);
    filp->f_pos = temp;
    break;
  }

  pr_info("New f_pos = %lld\n", filp->f_pos);
  return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count,
                 loff_t *f_pos) {
  struct pcdev_private_data *pcdev_data;
  loff_t temp;
  int ret;

  pcdev_data = (struct pcdev_private_data *)filp->private_data;

  pr_info("read was invoked count = %zu and f_pos = %lld\n", count, *f_pos);
  pr_info("Fpos = %llu \n", *f_pos);

  temp = count + (*f_pos);

  if (temp > (pcdev_data->pData.size - *(f_pos))) {
    pr_info("Copying the data -- count is more than required\n");
    count = pcdev_data->pData.size - *f_pos;
    ret = copy_to_user(buff, pcdev_data->buffer + *f_pos, count);
    if (ret < 0) {
      pr_err("Copy to user failed = %d\r\n", ret);
      return -EFAULT;
    }
    *f_pos = pcdev_data->pData.size;
    pr_info("New f_pos = %llu and bytes written = %zu\n", *f_pos, count);
    return count;
  }

  else {
    pr_info("Copy the remaining data\n");
    ret = copy_to_user(buff, pcdev_data->buffer + *f_pos, count);
    if (ret < 0) {
      pr_err("Copy to user failed = %d\r\n", ret);
      return -EFAULT;
    }
    *f_pos += count;
    pr_info("New f_pos = %llu\n", *f_pos);
    return count;
  }
  return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count,
                  loff_t *f_pos) {
  struct pcdev_private_data *pcdev_data;
  loff_t temp;
  int ret;

  pcdev_data = (struct pcdev_private_data *)filp->private_data;
  pr_info("write was invoked count = %zu and f_pos = %lld\n", count, *f_pos);

  temp = count + (*f_pos);

  if (temp > (pcdev_data->pData.size - *(f_pos))) {
    pr_info("Writing the data --over flow\n");
    return -ENOMEM; /*check out of memory error code.*/
  }

  else {
    pr_info("Write the remaining data\n");
    ret = copy_from_user(pcdev_data->buffer + *f_pos, buff, count);
    if (ret < 0) {
      pr_err("Copy from user failed = %d\r\n", ret);
      return -EFAULT;
    }

    *f_pos += count;
    pr_info("Updated f_pos = %lld\n", *f_pos);
    return count;
  }

  pr_info("Updated f_pos = %lld\n", *f_pos);

  return 0;
}

int pcd_open(struct inode *inode, struct file *filp) {
  int minor_n;
  int ret;
  struct pcdev_private_data *pcdev_data;
  pr_info("open was invoked\n");

  minor_n = MINOR(inode->i_rdev);
  pr_info("minor access = %d\n", minor_n);

  /*get device's private data structure*/
  pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);
  filp->private_data = pcdev_data;

  ret = check_permission(filp->f_mode, pcdev_data->pData.perm);
  (!ret) ? pr_info("Correct permission\n") : pr_err("Incorrect permission\n");

  return ret;
}

int pcd_release(struct inode *inode, struct file *flip) {
  pr_info("release was invoked\n");
  return 0;
}

#if 1
static int check_permission(fmode_t mode, int perm) {
  if (perm == RDWR) {
    pr_info("Permission granted - RW\n");
    return 0;
  } else if ((perm == RONLY) && (mode & FMODE_READ) && !(mode & FMODE_WRITE)) {
    pr_info("Permission granted - RONLY\n");
    return 0;
  } else if ((perm == WRONLY) && (mode & FMODE_WRITE) && !(mode & FMODE_READ)) {
    pr_info("Permission granted - WRONLY\n");
    return 0;
  } else {
    return -EPERM;
  }
}
#endif