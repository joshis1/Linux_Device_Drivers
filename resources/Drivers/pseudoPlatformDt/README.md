#**Build device tree**

Go to kernel source directory.


	export CROSS_COMPILE=arm-linux-gnueabihf-
	export ARCH=arm
	make kush-zinc.dtb
	
#**Important data structure**

	mod_devicetable.h
	
	struct of_device_id {
		char	name[32];
		char	type[32];
		char	compatible[128];
		const void *data;
	};

[https://elixir.bootlin.com/linux/latest/source/include/linux/platform_device.h](https://elixir.bootlin.com/linux/latest/source/include/linux/platform_device.h) 

	struct platform_driver {
		int (*probe)(struct platform_device *);
		int (*remove)(struct platform_device *);
		void (*shutdown)(struct platform_device *);
		int (*suspend)(struct platform_device *, pm_message_t state);
		int (*resume)(struct platform_device *);
		struct device_driver driver;
		const struct platform_device_id *id_table;
		bool prevent_deferred_probe;
	};

[https://elixir.bootlin.com/linux/latest/source/include/linux/device/driver.h#L95](https://elixir.bootlin.com/linux/latest/source/include/linux/device/driver.h#L95) 

	struct device_driver {
		const char		*name;
		struct bus_type		*bus;
	
		struct module		*owner;
		const char		*mod_name;	/* used for built-in modules */
	
		bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */
		enum probe_type probe_type;
		const struct of_device_id	*of_match_table;
		const struct acpi_device_id	*acpi_match_table;
	
		int (*probe) (struct device *dev);
		void (*sync_state)(struct device *dev);
		int (*remove) (struct device *dev);
		void (*shutdown) (struct device *dev);
		int (*suspend) (struct device *dev, pm_message_t state);
		int (*resume) (struct device *dev);
		const struct attribute_group **groups;
		const struct attribute_group **dev_groups;
	
		const struct dev_pm_ops *pm;
		void (*coredump) (struct device *dev);
	
		struct driver_private *p;
	};

#**Testing**

		#lsblk
		#mount /dev/mmcblk0p1 /mnt
		#cd /mnt 
		#ls -l <boot partition files>
		#sync
		#cd /sys/devices 
		#cd platform 
		
You will see devices like - 

	  pcdev-1, pcdev-2, pcdev-3 

	cd pc-dev-1 
	cd of_node --> properties of the node. 
	cat compatible ->
        
        
