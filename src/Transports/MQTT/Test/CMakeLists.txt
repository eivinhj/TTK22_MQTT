cmake_minimum_required(VERSION 2.8.9)


find_package(PahoMqttCpp REQUIRED)


add_executable(test_mqtt test.cpp)
target_link_libraries(test_mqtt paho-mqttpp3 paho-mqtt3as)