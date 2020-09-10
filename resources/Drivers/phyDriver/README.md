**This driver has dependency on the following.**

	1) CONFIG_MDIO_DEVICE=y
	2) CONFIG_PHYLIB=y
	3) CONFIG_SWPHY=y

Ensure that the main device tree is compiled with symbol option.
In my microzed case, it was not compiled with symbol option thus DTB overlay was failing.

Here is how I have created the main device tree with the symbol.

	$bitbake -c devshell virtual/kernel	 
	$rm -f ../../../work/microzed_zynq7-poky-linux-gnueabi/linux-xlnx/4.19-xilinx-v2019.1+gitAUTOINC+9811303824-r0/linux-microzed_zynq7-standard-build/arch/arm/boot/dts/zynq-microzed.dtb  
	$DTC_FLAGS="-@" make dtbs
	$cp ../../../work/microzed_zynq7-poky-linux-gnueabi/linux-xlnx/4.19-xilinx-v2019.1+gitAUTOINC+9811303824-r0/linux-microzed_zynq7-standard-build/arch/arm/boot/dts/zynq-microzed.dtb  

**Compiling DTBO**

/opt/poky/2.7.3/sysroots/x86_64-pokysdk-linux/usr/bin/dtc -o dtb -o phyDriver.dtbo -@ ./phyDriver.dts

**Loading DTBO on the zynq device**

	#mkdir -p /sys/kernel/config/device-tree/overlays/phyDriver
	# cat /home/root/phyDriver.dtbo  > /sys/kernel/config/device-tree/overlays/phyDriver/dtbo


**Recompilation of the driver**

	$bitbake phy-mod -c compile -f

The driver output can be found at ./tmp/work/microzed_zynq7-poky-linux-gnueabi/phy-mod/0.1-r0/phyDriver.ko

**Resources**
**Device tree editor**

https://marketplace.visualstudio.com/items?itemName=plorefice.devicetree 

**Adding this driver to yocto**  
We will first create a layer i.e. our custom layer, where we can build the LKM.  
$bitbake-layers create-layer meta-drivers  
$cd meta-drivers  
$cp -vrf ../meta-skeleton/recipes-kernel/ ./  
$gedit  build/conf/bblayers.conf  
${TOPDIR}/../meta-drivers \  
$bitbake hello-mod  
or  
$bitbake -s | grep -i phy  

