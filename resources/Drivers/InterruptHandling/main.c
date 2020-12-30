#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/slab.h>



#define DESKTOP_PCI_ETH_IRQ  (16)


typedef struct mydevice_t
{
  int irq_no;
  char name[512];
}mydevice_t;


/** do_IRQ() -- is the common function for all hardware interrupts **/
/** watch -d --> highlight the differences between successive updates **/
/** watch -n 0.1 -d 'cat /proc/interrupts | less' **/


/** lspci interrupt testing for local pc
 *  lspci 
 *  lspci | grep -i Ethernet
 *  lspci -i 00:03.0 -v
 *   0:08.0 Ethernet controller: Intel Corporation 82540EM Gigabit Ethernet Controller (rev 02)
 *	Subsystem: Intel Corporation 82540EM Gigabit Ethernet Controller (PRO/1000 MT Desktop Adapter)
 *	Flags: bus master, 66MHz, medium devsel, latency 64, IRQ 16
 *  
 * Got the Interrupt number i.e. IRQ 16  */


/** Ping every 300 ms - 
 * ping -i 0.3 127.0.0.1 
 * */

/** Useful error number utility in linux 
 * $errno 16
EBUSY 16 Device or resource busy
*/

/** Which CPU should serve the interrupt **/
/* $cat /proc/irq/16/smp_affinity
f
Changing the affinity - echo 1 > /proc/irq/16/smp_affinity
**/

/** level triggered - active logic level - either high or low **/
/** allow multiple devices to share a common interrupt signal **/
/** Edge triggered - falling edge or rising edge -- falling - high to low and rising - low to high **/

mydevice_t *mydev;

static irqreturn_t eth_my_interrupt_handler(int irq, void *dev)
{
   mydevice_t *mydev = (mydevice_t*)dev;
   pr_info("Ethernet interrupt handled irq = %d and irq owner is %s \r\n",mydev->irq_no, mydev->name);
   return IRQ_NONE;
}

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init interrupt_init(void) {
  pr_info("interrupt World\r\n");
  
  mydev = kzalloc(sizeof(mydevice_t), GFP_KERNEL);

  mydev->irq_no = 16;
  strcpy(mydev->name,"Shreyas");

  if(request_irq(DESKTOP_PCI_ETH_IRQ, eth_my_interrupt_handler, IRQF_SHARED, "eth_irq_handler", mydev ))
  {
    pr_err("Failed to register interrupt \r\n");
  }
  return 0; // returning 0 means successful
}

/** Called when the module is removed - rmmod**/
static void __exit interrupt_cleanup(void) {
  pr_info("Good Bye interrupt World\r\n"); // pr_info is wrapper over printk
  synchronize_irq(DESKTOP_PCI_ETH_IRQ);
  free_irq(DESKTOP_PCI_ETH_IRQ, mydev);
}

/**module registration **/
module_init(interrupt_init);
module_exit(interrupt_cleanup);

MODULE_LICENSE("GPL"); // GPL - check module.h - very important
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Interrupt Program");
MODULE_INFO(board, "Zedboard - AVNET");
