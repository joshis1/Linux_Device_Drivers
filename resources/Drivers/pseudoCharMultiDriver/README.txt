*References:*

[https://code.woboq.org/linux/linux/include/linux/fs.h.html](https://code.woboq.org/linux/linux/include/linux/fs.h.html) 

[https://code.woboq.org/linux/linux/fs/char_dev.c.html](https://code.woboq.org/linux/linux/fs/char_dev.c.html) 

[https://code.woboq.org/linux/linux/include/linux/kernel.h.html](https://code.woboq.org/linux/linux/include/linux/kernel.h.html) 

*Testing*

**RDONLY - /dev/pcd-1 - test**

	# cat /dev/pcd-1
	Testing PCDEV1 RDONLY

	# echo "Shreyas" > /dev/pcd-1 
	bash: /dev/pcd-1: Operation not permitted
	
       #dd if=pcd_n.c of=/dev/pcd-1 count=1 
       #strace dd if=pcd_n.c of=/dev/pcd-1 count=1
       
**WRONLY - /dev/pcd-2 - test**

	# cat /dev/pcd-2
	cat: /dev/pcd-2: Operation not permitted
	echo "Shreyas" > /dev/pcd-2 


**RW - /dev/pcd-3 - test**

	dd if=pcd_n.c of=/dev/pcd-3 bs=1 count=10  -> i.e reading 10 	bytes. 

	#  dd if=/dev/pcd-3 of=hello.txt  bs=1 count=10
	cat hello.txt 	
	#include <

**lseek - test** 

Write the data first

	dd if=pcd_n.c of=/dev/pcd-3 bs=1 count=10
	# cat /dev/pcd-3 
	#include <

Now - seek first 5 characters.

	strace  dd if=/dev/pcd-3 of=hello.txt  bs=1 count=10  skip=5
	cat hello.txt
	ude <
