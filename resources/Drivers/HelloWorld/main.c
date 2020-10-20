#include <linux/module.h>


/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init helloWorld_init(void)
{
   pr_info("Hello World\r\n");
   return 0; // returning 0 means successful
}

/** Called when the module is removed - rmmod**/
static void __exit helloWorld_cleanup(void)
{
   pr_info("Good Bye World\r\n"); // pr_info is wrapper over printk
}

/**module registration **/
module_init(helloWorld_init);
module_exit(helloWorld_cleanup);

MODULE_LICENSE("GPL"); //GPL - check module.h - very important 
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Hello World Program");
MODULE_INFO(board, "Zedboard - AVNET");



