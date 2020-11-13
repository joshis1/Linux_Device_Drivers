**How to compile the dts overlay?**

	dtc -@ -I dts -O dtb -o pcdev0.dtbo pcdev0.dts 
	
**How to install device-tree-compiler?**

	sudo apt-get install device-tree-compiler
	
**Main device tree compilation**

	export DTC_FLAGS='-@'
	
Otherwise, the main device tree cannot use overlay.

