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



ssh admin@172.25.27.205

Free4All

connect nxos

terminal session-timeout 10800

exit

connect cimc 2/1

sldp

admin

Free4Al

response key












export TMOUT=00;
readonly TMOUT;
readonly SECURE_LOGIN_USERNAME;
export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
export PS1="\d \t IBMC-SLOT["`/etc/scripts/getslotid.sh`"] # "
export HISTTIMEFORMAT="%c ";
ulimit -Sc unlimited
ulimit -Hc unlimited
set histappend

# Stops same command from being displayed twice when pressing 'up' arrow
export HISTCONTROL=ignoreboth
alias ll="ls -lh --color=auto"
export HOME="/root"
export PYTHONPATH=/usr/local/lib/python310.zip:/usr/local/lib/python3.10:/usr/local/lib/python3.10/lib-dynload:/usr/local/lib/python3.10/site-packages





./network_diag -d 3600 -ip 172.25.27.74 -p 0000:3a:00.1 -pw Free4AllFree4All -phip 172.25.27.81 -phpw Free4AllFree4All -pp 0000:da:00.1 -si -sp
./network_diag -d 3600 -ip <oob_net0 IP address> -p <BlueField-3 PCIe Domain:Bus:Device> -pw <root
password of BlueField-3> -phip <Partner Host IP> -phpw <Partner Host password> -pp <Partner Host
PCI DOMAIN:BUS:DEVICE> -si -sp


jrpc_client -a 127.3.0.1 -v -m user_create -s '{ "UserName":"hai", "AccountTypes" : [ "Redfish", "OEM" ], "Password":"Nbv12345", "RoleId":"admin", "Enabled" : true, "IPMI" : { "Password" : "Nbv12345" } }'


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


# Fundamentals of Corporate Finance: Key Concepts (Unit 8 Chapter 1)

## 1. Role of the Financial Manager
### Core Objective
- **Maximize shareholder wealth** through strategic decision-making[1].
- Focus on **positive residual cash flows** to sustain operations and growth[1].

### Three Fundamental Decisions
1. **Capital Budgeting**: Long-term asset investment decisions[1].
2. **Financing**: Optimizing debt-equity mix for asset funding[1].
3. **Working Capital Management**: Balancing short-term assets/liabilities[1].

### Stakeholder Considerations
- Manage relationships with employees, suppliers, creditors, and governments[1].
- Cash flow allocation impacts all stakeholders (e.g., wages, taxes)[1].

---

## 2. Business Organizational Structures
### Primary Forms
- **Sole Proprietorship**: Single owner, unlimited liability, tax simplicity[1].
- **Partnership**: Shared control, varied liability (general vs. limited)[1].
- **Corporation**: Limited liability, complex setup, double taxation[1].

### Hybrid Models
- **LLCs/LLPs**: Combine liability protection with partnership tax benefits[1].

| Feature               | Sole Proprietorship | Partnership | Corporation | LLC/LLP       |
|-----------------------|---------------------|-------------|-------------|---------------|
| Liability             | Unlimited           | Varies      | Limited     | Limited       |
| Tax Treatment         | Flow-through        | Flow-through| Double      | Flexible       |
| Capital Access        | Limited             | Moderate    | High        | Moderate      |

---

## 3. Financial Function Management
### Organizational Hierarchy
- **CFO**: Oversees treasurer, controller, risk manager, auditors[1].
- **Audit Committee**: Ensures financial integrity and compliance[1].

### Key Roles
- **Treasurer**: Manages cash flows and financing[1].
- **Controller**: Handles accounting/reporting[1].
- **Internal Auditor**: Monitors internal controls[1].

---

## 4. Corporate Goals and Valuation
### Primary Objective
- **Maximize stock value** by prioritizing cash flow timing/risk[1].

### Profit vs. Value
- Accounting profit ≠ economic profit due to cash flow timing[1].
- Market share expansion ≠ sustainable value without profitability[1].

### Stock Price Drivers
- External factors: Economic conditions, regulations[1].
- Internal factors: Management decisions, cash flow projections[1].

---

## 5. Agency Conflicts and Solutions
### Principal-Agent Dynamics
- Shareholders (principals) vs. Managers (agents)[1].
- **Agency Costs**: Misaligned incentives reducing shareholder value[1].

### Mitigation Strategies
- **Performance-linked compensation** (e.g., stock options)[1].
- **Oversight mechanisms**: Board governance, external audits[1].
- **Market pressures**: Takeover threats, managerial labor competition[1].

---

## 6. Ethical Frameworks in Finance
### Common Conflicts
- **Information asymmetry** (e.g., undisclosed asset risks)[1].
- **Conflicts of interest** (e.g., biased lender incentives)[1].

### Decision-Making Checklist
1. Legal compliance[1].
2. Role-specific obligations[1].
3. Shareholder impact assessment[1].
4. Transparency in conflicts[1].
5. "Wall Street Journal Test" for actions[1].

---

## 7. Regulatory Landscape (Sarbanes-Oxley)
### Key Requirements
- **Board independence**: Majority independent directors[1].
- **Financial accountability**: CEO/CFO certification of statements[1].
- **Ethics programs**: Whistleblower protections, complaint systems[1].

### Audit Enhancements
- External auditor rotation every 5 years[1].
- Restricted non-audit services by auditors[1].





