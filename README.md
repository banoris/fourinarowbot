Four-in-a-Row Bot
=================


## Setup on the BBB

1. Copy the FourInARow binary and the html folder to the /root directory.
2. Follow the instructions in BBBIOlib to "make install" the BBBIO-EHRPWM overlay.
3. Set up the BBBIO-EHRPWM overlay to be enabled at boot. Edit the file /etc/default/capemgr to read:
```
CAPE=BBBIO-EHRPWM
```
4. Set up FourInARow to launch at boot. Create a file named /etc/systemd/system/fourinarow.service with the following content:
```
[Unit]
Description=Four-in-a-Row Bot
After=network.target

[Service]
ExecStart=/root/FourInARow

[Install]
WantedBy=multi-user.target
```
5. Disable other web services running on the BBB:
```sh
systemctl disable cloud9.service
systemctl disable cloud9.socket
systemctl disable bonescript.service
systemctl disable bonescript-autorun.service
systemctl disable bonescript.socket
```
6. Enable the Four-in-a-Row Bot service:
```sh
systemctl enable fourinarow.service
```
7. Restart the BeagleBone Black. The program should start after the system boots.