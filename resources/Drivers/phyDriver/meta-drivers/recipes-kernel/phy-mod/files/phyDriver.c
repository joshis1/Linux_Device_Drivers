#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/mii.h>

#include <linux/mdio.h>
#include <linux/of_mdio.h>


#include <linux/of_platform.h>
//links to drivers/net/phy/phy.c

struct custom_port_phy_platform_local {
	struct device *dev;
	struct mii_bus *mii_bus;
	struct device_node *dev_node;
};

#if 1
/* Match table for of_platform binding */
static const struct of_device_id port_phy_of_match[] = {
	{ .compatible = "custom,custom-port-phy", },
        { },
};

MODULE_DEVICE_TABLE(of, port_phy_of_match);

#endif


/**
 * custom_port_phy_mdio_write - MDIO interface write function
 * @bus:	Pointer to mii bus structure
 * @phy_id:	Address of the PHY device
 * @reg:	PHY register to write to
 * @val:	Value to be written into the register
 *
 * Return:	0 on success, -ETIMEDOUT on a timeout
 *
 * Does nothing, just return 0 since there really isn't an MDIO bus
 */
static int custom_port_phy_mdio_write(struct mii_bus *bus, int phy_id, int reg,
			      u16 val)
{	
	return 0;
}


/**
 * custom_port_phy_mdio_read - MDIO interface read function
 * @bus:	Pointer to mii bus structure
 * @phy_id:	Address of the PHY device
 * @reg:	PHY register to read
 *
 * Return:	The register contents on success, -ETIMEDOUT on a timeout
 *
 * Does nothing, just return 0 since there really isn't an MDIO bus
 */
static int custom_port_phy_mdio_read(struct mii_bus *bus, int phy_id, int reg)
{
	if (reg == MII_PHYSID1) {
	   printk(KERN_ALERT "MII_PHYSID1 = %d\r\n",MII_PHYSID1);
	   return 0x00000002 >> 16;
	}
	if (reg == MII_PHYSID2) {
	    printk(KERN_ALERT "MII_PHYSID2 = %d\r\n",MII_PHYSID2);
    	return 0x00000002 & 0xFFFF;
	}
	return 0;
}


int custom_port_phy_mdio_setup(struct custom_port_phy_platform_local *lp, struct device_node *np)
{
	int ret;
	struct mii_bus *bus;

	bus = mdiobus_alloc();
	if (!bus)
		return -ENOMEM;

	snprintf(bus->id, MII_BUS_ID_SIZE, "%s",np->name);

	bus->priv = lp;
	bus->name = "Custom Port PHY MDIO";
	bus->read = custom_port_phy_mdio_read;
	bus->write = custom_port_phy_mdio_write;
	bus->parent = lp->dev;
	lp->mii_bus = bus;

	ret = of_mdiobus_register(bus, np); 
	if (ret) {
		mdiobus_free(bus);
		return ret;
	}

	dev_err(lp->dev, "MDIO Bus 0x%p registered\n",bus);

	return 0;
}


void custom_port_phy_mdio_teardown(struct custom_port_phy_platform_local *lp)
{
	mdiobus_unregister(lp->mii_bus);
	mdiobus_free(lp->mii_bus);
	lp->mii_bus = NULL;
}


static int phy_platform_probe(struct platform_device *pdev) { 

  struct custom_port_phy_platform_local *lp;
	int ret;
	
        printk(KERN_ALERT "Shreyas..phy_platform_probe..Invoked\r\n");
	lp  = devm_kzalloc(&pdev->dev,sizeof(struct custom_port_phy_platform_local),GFP_KERNEL);
	if (!lp) {
	  dev_err(&(pdev->dev),"Failed to allocatate platform_local\n");
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, lp);

	lp->dev = &pdev->dev;
	ret = custom_port_phy_mdio_setup(lp, pdev->dev.of_node);
	if (ret < 0) {
	  dev_err(&(pdev->dev),"Failed to setup MDIO bus\n");
	  return ret;
	}

	return 0;
}


static int phy_platform_remove(struct platform_device *pdev) { 
  struct custom_port_phy_platform_local *lp = platform_get_drvdata(pdev);
	dev_info(&pdev->dev,"%s Enter\n",__func__);

  custom_port_phy_mdio_teardown(lp);
	return 0;
}

static struct phy_driver custom_phy_driver = {

  .phy_id = 0x00000002,
  .phy_id_mask = 0xffffffff,
  .name = "custom_port_phy",

};

static struct platform_driver custom_phy_platform_driver = {
 .probe = phy_platform_probe,
 .remove = phy_platform_remove,
 #if 1
 .driver = {
    .name = "custom_port_phy",
    .of_match_table = port_phy_of_match,
 },
 #endif
};



static int __init phy_init(void)
{
   int ret = 0;
   printk(KERN_ALERT "Shreyas >>> phy_driver_register started\r\n");
   ret = phy_driver_register(&custom_phy_driver, THIS_MODULE);
   printk(KERN_ALERT "Shreyas >>> phy_driver_register END\r\n");
   if(ret < 0) {
	   printk(KERN_ALERT "custom phy driver registration failed\r\n");
	   return ret;
   }

   ret = platform_driver_register(&custom_phy_platform_driver);
   if(ret < 0) {
	   phy_driver_unregister(&custom_phy_driver);
	   printk("%s: Failed to register as Platform Driver\r\n", __func__);
	   return ret;
   }
   
   return ret;
}

static void __exit phy_exit(void)
{
    pr_info("Goodbye phy_driver .\n");
	phy_driver_unregister(&custom_phy_driver);
	platform_driver_unregister(&custom_phy_platform_driver);
}

module_init(phy_init);
module_exit(phy_exit);
/** The module licencse is very important in order to use GPL functions/symbols **/
MODULE_LICENSE("GPL");
