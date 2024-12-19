#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－Hunan Chuanglebo Intelligent Technology Co., Ltd.－－－－
#  File name：UDP.py
#  Version：V2.0
#  author: zhulin
# Description: WIFI UDP communication case
#---------------------------------------
from machine import UART, Pin
import utime,time

# WIFI router information, please fill in your own WIFI router information
SSID='***********'               # WIFI name
password = '*********'           # WIFI password
remote_IP = '192.168.100.14'     # The IP address of the computer needs to be changed by yourself.
remote_Port = '8080'             # Computer port number
local_Port = '1112'              # Local UDP port

# The serial port is mapped to the GP0 and GP1 ports. When using this port and
# when communicating with the WIFI module, do not use the GP0 and GP1 ports
esp_uart = UART(0, 115200)   # Serial port 0, baud rate 115200

# Send command function
def esp_sendCMD(cmd,ack,timeout=2000):
    esp_uart.write(cmd+'\r\n')
    i_t = utime.ticks_ms()
    while (utime.ticks_ms() - i_t) < timeout:
        s_get=esp_uart.read()
        if(s_get != None):
            s_get=s_get.decode()
            print(s_get)
            if(s_get.find(ack) >= 0):
                return True
    return False

# Program entrance
if __name__ == "__main__":
    esp_uart.write('+++')           # Initialize and exit transparent transmission mode
    time.sleep(1)
    if(esp_uart.any()>0):
        esp_uart.read()
    esp_sendCMD("AT","OK")          # AT command
    esp_sendCMD("AT+CWMODE=3","OK") # Configure WiFi mode
    esp_sendCMD("AT+CWJAP=\""+SSID+"\",\""+password+"\"","OK",20000) # Connect to router
    esp_sendCMD("AT+CIFSR","OK")                                     # Query the IP address of the WIFI module
    esp_sendCMD("AT+CIPSTART=\"UDP\",\""+remote_IP+"\","+remote_Port+","+local_Port+",0","OK",10000) # Create UDP transport
    esp_sendCMD("AT+CIPMODE=1","OK")    # Turn on transparent transmission mode, data can be transmitted directly 
    esp_sendCMD("AT+CIPSEND",">")       # Send data 

    esp_uart.write('Hello makerobo !!!\r\n')       # Send the corresponding string
    esp_uart.write('RP2040 UDP message!\r\n')
    while True:
        s_get=esp_uart.read()                      # receive characters
        if(s_get != None):                         # Check if the character is not empty
            s_get=s_get.decode()             
            print(s_get)                           # String printing
            esp_uart.write(s_get)                  # String return   
