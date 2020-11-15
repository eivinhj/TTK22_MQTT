MQTT: Message Queuing Telemetry Transport
======================================


## Install requirements



```
sudo apt-get install build-essential gcc make cmake cmake-gui cmake-curses-gui
sudo apt-get install doxygen graphviz
sudo apt-get install libcppunit-dev
sudo apt-get install libssl-dev
sudo apt-get install mosquitto
```

## Paho MQTT C Library
Navigate to Lib/paho.mqtt.c

The Paho MQTT C-library is also available to download from https://github.com/eclipse/paho.mqtt.c 

Install with:



```
make
sudo make install
```





Documentation can be generated with:

```
	make html
```

## Paho MQTT C++ Library
Navigate to Lib/paho.mqtt.c
The Paho MQTT C++-library is also available to download from https://github.com/eclipse/paho.mqtt.cpp 

Install with:

```
cmake -Bbuild -H. -DPAHO_BUILD_DOCUMENTATION=TRUE -DPAHO_BUILD_SAMPLES=TRUE
sudo cmake --build build/ --target install
sudo ldconfig
```







## Test that it works

Start mosquitto MQTT broker:

```
mosquitto -v -p 2023
```


Navigate to dune/build folder.

Build with 
```
make rebuild_cache && make
```
Run noptilus simulator:

```
sudo ./dune -c lauv-noptilus-1 -p Simulation
```



Upload planSpec &quot;s&quot; via Neptus or change planId in MQTT_PlanControl_StartPlan and MQTT_PlanControl_StopPlan .ini files located in /etc/


Start planSpec with

```
./dune -c MQTT_PlanControl_StartPlan -p Simulation

```

Stop planSpec with

```
./dune -c MQTT_PlanControl_StopPlan -p Simulation

```
