# Raspberry Pi Model B (Wheezy) — Wi-Fi + SSH Procedure

## 1. Stop hostapd

>>> sudo service hostapd stop

## 2. Set wlan0 to Managed mode <br>
Managed mode means the Raspberry Pi's Wi-Fi adapter acts as a Wi-Fi client/station, connecting to an existing Wi-Fi network.<br>

>>> sudo ifconfig wlan0 down <br>
>>> sudo iwconfig wlan0 mode managed <br>
>>> sudo ifconfig wlan0 up <br>

## 3. Scan for Wi-Fi

>>> sudo iwlist wlan0 scan | grep ESSID

## 4. Configure Wi-Fi

>>> sudo nano /etc/wpa_supplicant/wpa_supplicant.conf <br>

Add:
[

ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1

network={
    ssid="YOUR_WIFI_NAME"
    psk="YOUR_WIFI_PASSWORD"
}
]

Save and exit.

## 5. Connect to Wi-Fi

>>> sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf <br>

Check:

>>> iwconfig wlan0 <br>

## 6. Get an IP address

sudo dhclient wlan0

Check:

ifconfig wlan0

Find the "inet addr" value, for example:

inet addr:10.93.160.4

## 7. Test Wi-Fi

route -n

Then ping the gateway shown in the routing table:

ping -c 3 GATEWAY_IP

## 8. Check SSH

sudo service ssh status

It should show:

[ ok ] sshd is running.

## 9. Connect from Windows 11

Connect Windows to the SAME Wi-Fi network as the Raspberry Pi.

Open Command Prompt:

cd \

Then use the Raspberry Pi's current wlan0 IP:

ssh pi@RASPBERRY_PI_WIFI_IP

Example:

ssh pi@10.93.160.4

Enter the Raspberry Pi password.

## 10. When changing to another Wi-Fi network

Reconnect the Raspberry Pi to the new Wi-Fi.

On the Pi:

ifconfig wlan0

Find the new "inet addr" IP address.

Example:

inet addr:192.168.1.25

Connect the Windows computer to the SAME Wi-Fi network.

From Windows:

ssh pi@192.168.1.25

The IP address can change whenever the Raspberry Pi connects to a different Wi-Fi network.



### Ramer1 wireless IP : 
>>> ssh pi@:10.93.160.4