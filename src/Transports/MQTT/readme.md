MQTT: Message Queuing Telemetry Transport
======================================

Install requirements:
 
'''
sudo apt-get install build-essential gcc make cmake cmake-gui cmake-curses-gui
sudo apt-get install doxygen graphviz
sudo apt-get install libcppunit-dev
sudo apt-get install libssl-dev
sudo apt-get install mosquitto
'''

Download the Paho MQTT C-library from https://github.com/eclipse/paho.mqtt.c
Install with: 
	'''
	make
	sudo make install
	'''

	Documentation can be generated with:
	'''
	make html
	'''

Download the Paho MQTT C++-library from 
https://github.com/eclipse/paho.mqtt.cpp
Install with: 
  '''
cmake -Bbuild -H. -DPAHO_BUILD_DOCUMENTATION=TRUE -DPAHO_BUILD_SAMPLES=TRUE
sudo cmake --build build/ --target install
sudo ldconfig
	'''



At runtime: 
Start mosquitto MQTT broker: 
'''
mosquitto -v -p #port
'''

Test:
Navigate to dune/build folder.
Build with make rebuild_cache && make
Run noptilus simulator: 
'''
sudo ./dune -c lauv-noptilus-1 -p Simulation
'''

Upload planSpec "s" via Neptus.

Start planSpec with
'''
./dune -c MQTT-send_IMC_message -p Simulation
'''
