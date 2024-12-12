#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－Hunan Chuanglebo Intelligent Technology Co., Ltd.－－－－
#  File name：TCP_Client.py
#  Version：V2.0
#  author: zhulin
#---------------------------------------
from machine import UART, Pin
import utime,time

# WIFI router information, please fill in your own WIFI router information
SSID='***********'            # WIFI name
password = '*********'        # WIFI password
ServerIP = '192.168.100.14'   # Computer server IP address needs to be modified
Port = '8080'                 # Computer port number

# The serial port is mapped to the GP0 and GP1 ports. When using this port and
# when communicating with the WIFI module, do not use the GP0 and GP1 ports
esp_uart = UART(0, 115200) # Serial port 0, baud rate 115200

# Send command function
def esp_sendCMD(cmd,ack,timeout=2000):
    esp_uart.write(cmd+'\r\n')
    i_t = utime.ticks_ms()
    while (utime.ticks_ms() - i_t) < timeout:
        s_get = esp_uart.read()
        if(s_get != None):
            s_get=s_get.decode()
            print(s_get)
            if(s_get.find(ack) >= 0):
                return True
    return False


# Program entrance
if __name__ == "__main__":
    esp_uart.write('+++')   # Initialize and exit transparent transmission mode
    time.sleep(1)
    if(esp_uart.any()>0):
        esp_uart.read()
    esp_sendCMD("AT","OK")          # AT command
    esp_sendCMD("AT+CWMODE=3","OK") # Configure WiFi mode
    esp_sendCMD("AT+CWJAP=\""+SSID+"\",\""+password+"\"","OK",20000) # Connect to router
    esp_sendCMD("AT+CIFSR","OK")     # Query the IP address of the WIFI module
    esp_sendCMD("AT+CIPSTART=\"TCP\",\""+ServerIP+"\","+Port,"OK",10000) # The RP2040-w module connects to the server as a TCP client
    esp_sendCMD("AT+CIPMODE=1","OK")   # Enable transparent transmission mode
    esp_sendCMD("AT+CIPSEND",">")      # RP2040-w module sends data to the server

    esp_uart.write('Hello makerobo !!!\r\n')   # Send relevant content
    esp_uart.write('RP2040-W TCP Client\r\n') 

    while True:
        i_s=esp_uart.read()
        if(i_s != None):
            i_s=i_s.decode()
            print(i_s)
