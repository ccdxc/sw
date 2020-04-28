## penctl show metrics system temp

System temperature information

### Synopsis



---------------------------------
 System temperature information:


Value Description:

local_temperature: Temperature of the board.
die_temperature: Temperature of the die.
hbm_temperature: Temperature of the hbm.
qsfp_port1_temperature: Temperature of qsfp port 1.
qsfp_port2_temperature: Temperature of qsfp port 2.
qsfp_port1_warning_temperature: Warning temperature of qsfp port 1.
qsfp_port2_warning_temperature: Warning temperature of qsfp port 2.
qsfp_port1_alarm_temperature: Alarm temperature of qsfp port 1.
qsfp_port2_alarm_temperature: Alarm temperature of qsfp port 2.
The temperature is degree Celcius

LocalTemperature	: Temperature of the board in celsius
DieTemperature	: Temperature of the die in celsius
HbmTemperature	: Temperature of the HBM in celsius
QsfpPort1Temperature	: QSFP port 1 temperature in celsius
QsfpPort2Temperature	: QSFP port 2 temperature in celsius
QsfpPort1WarningTemperature	: QSFP port 1 warning temperature in celsius
QsfpPort2WarningTemperature	: QSFP port 2 warning temperature in celsius
QsfpPort1AlarmTemperature	: QSFP port 1 alarm temperature in celsius
QsfpPort2AlarmTemperature	: QSFP port 2 alarm temperature in celsius

---------------------------------


```
penctl show metrics system temp [flags]
```

### Options

```
  -h, --help   help for temp
```

### Options inherited from parent commands

```
  -a, --authtoken string   path to file containing authorization token
      --compat-1.1         run in 1.1 firmware compatibility mode
      --dsc-url string     set url for Distributed Service Card
  -j, --json               display in json format (default true)
  -t, --tabular            display in table format
      --verbose            display penctl debug log
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format
```

### SEE ALSO
* [penctl show metrics system](penctl_show_metrics_system.md)	 - Metrics for system monitors

