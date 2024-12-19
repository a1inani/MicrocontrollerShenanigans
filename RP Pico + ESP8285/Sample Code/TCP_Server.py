#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－Hunan Chuanglebo Intelligent Technology Co., Ltd.－－－－
#  File name：TCP_Client.py
#  Version：V2.0
#  author: zhulin
# Description: WIFI TCP server-side communication case
#---------------------------------------
from machine import UART, Pin
import utime,time

# WIFI router information, please fill in your own WIFI router information
SSID='***********'       # WIFI name
password = '*********'   # WIFI password
Port = '8080'            # Custom port number

# The serial port is mapped to the GP0 and GP1 ports. When using this port and
# when communicating with the WIFI module, do not use the GP0 and GP1 ports
esp_uart = UART(0, 115200) # Serial port 0, baud rate 115200

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

# Send data
def esp_sendData(ID,data):
    esp_sendCMD('AT+CIPSEND='+str(ID)+','+str(len(data)),'>')
    esp_uart.write(data)

# Receive data
def esp_ReceiveData():
    s_get=esp_uart.read()
    if(s_get != None):
        s_get=s_get.decode()
        print(s_get)
        if(s_get.find('+IPD') >= 0):
            n1=s_get.find('+IPD,')
            n2=s_get.find(',',n1+5)
            ID=int(s_get[n1+5:n2])
            n3=s_get.find(':')
            s_get=s_get[n3+1:]
            return ID,s_get
    return None,None

# Program entrance
if __name__ == "__main__":
    esp_uart.write('+++')            # Initialize and exit transparent transmission mode
    time.sleep(1)
    if(esp_uart.any()>0):
        esp_uart.read()
    esp_sendCMD("AT","OK")           # AT command
    esp_sendCMD("AT+CWMODE=3","OK")  # Configure WiFi mode
    esp_sendCMD("AT+CWJAP=\""+SSID+"\",\""+password+"\"","OK",20000) # Connect to router
    esp_sendCMD("AT+CIPMUX=1","OK")            # Enable multiple connections 
    esp_sendCMD("AT+CIPSERVER=1,"+Port,"OK")   # Establish TCP server
    esp_sendCMD("AT+CIFSR","OK")               # Query the IP address of the WIFI module

    while True:
        ID,s_get=esp_ReceiveData()            # receive data
        if(ID != None):
            esp_sendData(ID,s_get)            # If the received data is not empty, return
