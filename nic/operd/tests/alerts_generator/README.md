# Alerts Generator

    Test binary used to generate all types of supported alerts along with
    the following options

         -r alerts generation rate per second [default: 1 alert per sec]
         -n total number of alerts to be generated  [default: 1 alert]
         -t type of alert to be generated [default: -1 (random type)]
         -p prefix for alert messages [default: 'TEST_ALERT_GEN']

## How to run

### On SIM

    OPERD_REGIONS=/naples/nic/conf/operd-regions.json /naples/nic/bin/alerts_gen -t 3 -n 10 -r 10

### On HW

    Being a testapp, It is not packaged in HW but it should work
    <copied_path>/alerts_gen -t 3 -n 10 -r 10

