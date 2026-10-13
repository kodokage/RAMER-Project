### Check the pi os version 
>>> cat /etc/os-release

### check pi hardware version
>>>-- cat /proc/cpuinfo

### Check internet configuration
>>> ifconfig -a

>>> cat /etc/network/interfaces

### Edit the network interfaces:

>>> nano /etc/network/interfaces   

### Enter this: 

>>> auto lo   <br>
>>> iface lo inet loopback     <br>
>>> auto eth0  <br>
>>> allow-hotplug eth0   <br>
>>> iface eth0 inet static   <br>
>>> &emsp;        address 192.168.50.2   # Set preferred IP address must be same IP as pc <br>
>>> &emsp;         netmask 255.255.255.0  # Set subnet  <br>


### Reset the network config <br>
>>>  sudo ifdown eth0    <br>
>>>  sudo ifup eth0      <br>
>>>  ifconfig            <br>

### Check SSH

>>>  sudo service ssh status

>>>  sudo service ssh start

### SSH into Raspberry pi from cmd

>>>  ssh pi@192.168.50.2


### Copy file from PC to Raspberrypi vis SSH <br>
In commandline in pc, enter :<br>
>>> pscp "C:\Users\YourName\Desktop\<file_name>" pi@192.168.1.50:/home/pi/ramer/



###   Raspberry pi lan ethernet connection settings

>>> cat /etc/network/interfaces
    
## Ethernet connections   
auto lo <br>
iface lo inet loopback <br>
auto eth0 <br>

allow-hotplug eth0 <br>
iface eth0 inet static <br>
&emsp; address 192.168.50.2 <br>
&emsp; netmask 255.255.255.0 <br>


## wifi connection
allow-hotplug wlan0 <br>
iface wlan0 inet dhcp <br>
&emsp; wpa-ssid "< Wifi device name >" <br>
&emsp; wpa-psk "< wifi password >" <br>





