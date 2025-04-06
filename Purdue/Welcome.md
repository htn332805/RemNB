###### proxy

	export http_proxy=http://173.36.224.109:80/
	export https_proxy=http://173.36.224.109:80/
	export ftp_proxy=http://173.36.224.109:80/
	export no_proxy=.cisco.com
	export HTTP_PROXY=http://173.36.224.109:80/
	export HTTPS_PROXY=http://173.36.224.109:80/
	export FTP_PROXY=http://173.36.224.109:80/

echo "127.0.1.1 ThermalUSB.localdomain ThermalUSB" >> /mnt/etc/hosts

###### Connsat
1. start connsat session 
> `$CONNSAT_PATH -A -W "120" -S /run/current-system/sw/bin/bash -i 127.0.0.1  -p $STARTING_PORT`

> `./connsat -A -W "100" -S /run/current-system/sw/bin/bash -i 172.25.27.208 -p 9000`

2. stat
