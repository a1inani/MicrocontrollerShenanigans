#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－湖南创乐博智能科技有限公司－－－－
#  文件名：TCP_Client.py
#  版本：V2.0
#  author: zhulin
# 说明：WIFI TCP 客户端通讯案例
#---------------------------------------
from machine import UART, Pin
import utime,time

# WIFI 路由器信息，请填上自己的WIFI路由器信息
SSID='***********'            # WIFI名称
password = '*********'        # WIFI密码
ServerIP = '192.168.100.14'   # 电脑端服务器IP地址，需要修改
Port = '8080'                 # 电脑端端口号

# 串口映射到GP0和GP1端口上，在使用该端口和
# WIFI模块通讯时，不要使用GP0和GP1端口
esp_uart = UART(0, 115200) # 串口0,波特率为115200

# 发送命令函数
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


# 程序入口
if __name__ == "__main__":
    esp_uart.write('+++')   # 初始化退出透传模式
    time.sleep(1)
    if(esp_uart.any()>0):
        esp_uart.read()
    esp_sendCMD("AT","OK")          # AT指令
    esp_sendCMD("AT+CWMODE=3","OK") # 配置 WiFi 模式
    esp_sendCMD("AT+CWJAP=\""+SSID+"\",\""+password+"\"","OK",20000) # 连接路由器
    esp_sendCMD("AT+CIFSR","OK")     # 查询 WIFI模块的 IP 地址
    esp_sendCMD("AT+CIPSTART=\"TCP\",\""+ServerIP+"\","+Port,"OK",10000) # RP2040-w 模块作为 TCP client 连接到服务器
    esp_sendCMD("AT+CIPMODE=1","OK")   # 开启透传模式
    esp_sendCMD("AT+CIPSEND",">")      # RP2040-w 模块向服务器发送数据

    esp_uart.write('Hello makerobo !!!\r\n')   # 发送相关内容
    esp_uart.write('RP2040-W TCP Client\r\n') 

    while True:
        i_s=esp_uart.read()
        if(i_s != None):
            i_s=i_s.decode()
            print(i_s)
