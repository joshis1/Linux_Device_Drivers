#include <linux/module.h>


/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init helloWorld_init(void)
{
   int i = 0;
   for(i = 0; i <=10; i++)
      {
         pr_info("SJ--In-tree - Hello World\r\n");
      }
   return 0; // returning 0 means successful
}

//because of _exit macro and in-tree building, this will be stripped off in the code section
/** Called when the module is removed - rmmod**/
static void __exit helloWorld_cleanup(void)
{
   pr_info("In- tree Won't be part at all..\r\n"); // pr_info is wrapper over printk
}

/**module registration **/
module_init(helloWorld_init);
module_exit(helloWorld_cleanup);

MODULE_LICENSE("GPLv2"); //GPL - check module.h - very important 
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Hello World Program");
MODULE_INFO(board, "Zedboard - AVNET");



