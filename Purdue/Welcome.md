###### proxy

	export http_proxy=http://173.36.224.109:80/
	export https_proxy=http://173.36.224.109:80/
	export ftp_proxy=http://173.36.224.109:80/
	export no_proxy=.cisco.com
	export HTTP_PROXY=http://173.36.224.109:80/
	export HTTPS_PROXY=http://173.36.224.109:80/
	export FTP_PROXY=http://173.36.224.109:80/

	Tim to verify threshold on fishtail

echo "127.0.1.1 ThermalUSB.localdomain ThermalUSB" >> /mnt/etc/hosts

###### Connsat
1. start connsat session 
> `$CONNSAT_PATH -A -W "120" -S /run/current-system/sw/bin/bash -i 127.0.0.1  -p $STARTING_PORT`

> `./connsat -A -W "100" -S /run/current-system/sw/bin/bash -i 172.25.27.208 -p 9000`

2. stat


###### Misc trick
1. boot bios
	```
	while true
	do
	ipmitool -I bmc chassis bootdev 		bios
	sleep 10
	done
	```
	```
	while true
	do
	IPMICmd 0  0  8  1  f0
	/usr/local/bin/IPMICmd c 0 21 0 1 1
	IPMICmd 36 00 1A FF
	ipmitool -I bmc raw 0x00 0x08 0x01 0xf0
 	ipmitool -I bmc chassis bootdev bios
 	sleep 5
	done
	```


###### BLuefield
1. check for link type
	>  mlxconfig -d /dev/mst/mt4119_pciconf0 query 
	>  mlxconfig -d /dev/mst/mt41692_pciconf0 -e q
2. change the link IB
	> mlxconfig -d /dev/mst/mt4119_pciconf0 set LINK_TYPE_P1=1
	> mlxconfig -d /dev/mst/mt4119_pciconf1 set LINK_TYPE_P1=2
3. check if in dpu mode
	> mlxconfig -d /dev/mst/mt41692_pciconf0 q INTERNAL_CPU_MODEL expect emmbed
	> mlxconfig -d /dev/mst/mt41692_pciconf1 q INTERNAL_CPU_MODEL
	> mlxconfig -d /dev/mst/mt41692_pciconf1 q INTERNAL_CPU_OFFLOAD_ENGINE expect enable
	> mlxconfig -d /dev/mst/mt41692_pciconf0 q INTERNAL_CPU_OFFLOAD_ENGINE
	> mlxconfig -d /dev/mst/mt41692_pciconf0 s INTERNAL_CPU_OFFLOAD_ENGINE=1
	> mlxconfig -d /dev/mst/mt41692_pciconf1 s INTERNAL_CPU_OFFLOAD_ENGINE=1


./network_diag -d 3600 -ip 172.25.27.74 -p 0000:3a:00.1 -pw Free4AllFree4All -phip 172.25.27.81 -phpw Free4AllFree4All -pp 0000:da:00.1 -si -sp
./network_diag -d 3600 -ip <oob_net0 IP address> -p <BlueField-3 PCIe Domain:Bus:Device> -pw <root
password of BlueField-3> -phip <Partner Host IP> -phpw <Partner Host password> -pp <Partner Host
PCI DOMAIN:BUS:DEVICE> -si -sp


Step 1: Isolate p0 in OVS Configuration
Remove pf0hpf from the OVS Bridge
bash
ovs-vsctl del-port br0-ovs pf0hpf
Reconfigure OVS Bridge to Use Only p0
bash
ovs-vsctl add-br br0-ovs \
  -- set Bridge br0-ovs datapath_type=netdev \
  -- add-port br0-ovs p0 \
  -- set Interface p0 type=dpdk options:dpdk-devargs=0000:03:00.0
Replace 0000:03:00.0 with your PCIe device ID from mst status.

Step 2: Configure Traffic Steering Rules
Drop Traffic Directed to pf0hpf
bash
ovs-ofctl add-flow br0-ovs "priority=100,in_port=pf0hpf,actions=drop"
Route All Traffic Through p0
bash
ovs-ofctl add-flow br0-ovs "priority=100,in_port=p0,actions=normal"
Step 3: Disable Host PF Ownership
Ensure the host PF (pf0hpf) is not the port owner to prevent host-side interference:

bash
mlxprivhost -d /dev/mst/mt41692_pciconf0 --disable_port_owner
Step 4: Update hai_single_port.json
Ensure your JSON configuration file specifies p0 as the sole interface:

json
{
  "interfaces": ["p0"],
  "test_parameters": {
    "traffic_direction": "p0"
  }
}
Step 5: Validate Configuration
Check OVS Ports and Flows
bash
ovs-vsctl show
ovs-ofctl dump-flows br0-ovs
Verify Traffic Path
Use tcpdump to confirm traffic is only on p0:

bash
tcpdump -i p0 -nn
Troubleshooting
Link State: Ensure p0 has a valid link (ethtool p0).


DPDK Bindings: Confirm p0 is bound to DPDK:

bash
dpdk-devbind.py -s
Firmware Compatibility: Use BlueField OS v3.9.0+ for full p0 offload support.

This configuration ensures all diagnostic traffic is confined to p0, aligning with your single-port requirement.




To calculate the Sharpe and Treynor ratios for the mutual fund, we need to follow these steps:

### Sharpe Ratio

 $$ \[ w_A = \frac{(\sigma_B^2 (E(R_A) - R_f) - \sigma_A \sigma_B \rho_{AB} (E(R_B) - R_f))}{(\sigma_A^2 \sigma_B^2 (1 - \rho_{AB}^2))} \] $$