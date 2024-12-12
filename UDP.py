#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－湖南创乐博智能科技有限公司－－－－
#  文件名：UDP.py
#  版本：V2.0
#  author: zhulin
# 说明：WIFI UDP通讯案例
#---------------------------------------
from machine import UART, Pin
import utime,time

# WIFI 路由器信息，请填上自己的WIFI路由器信息
SSID='***********'               # WIFI名称
password = '*********'           # WIFI密码
remote_IP = '192.168.100.14'     # 电脑端的IP地址，需要自己改
remote_Port = '8080'             # 电脑端的端口号
local_Port = '1112'              # 本地的 UDP 端⼝

# 串口映射到GP0和GP1端口上，在使用该端口和
# WIFI模块通讯时，不要使用GP0和GP1端口
esp_uart = UART(0, 115200)   # 串口0,波特率为115200

# 发送命令函数
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

# 程序入口
if __name__ == "__main__":
    esp_uart.write('+++')           # 初始化退出透传模式
    time.sleep(1)
    if(esp_uart.any()>0):
        esp_uart.read()
    esp_sendCMD("AT","OK")          # AT指令
    esp_sendCMD("AT+CWMODE=3","OK") # 配置 WiFi 模式
    esp_sendCMD("AT+CWJAP=\""+SSID+"\",\""+password+"\"","OK",20000) # 连接路由器
    esp_sendCMD("AT+CIFSR","OK")                                     # 查询 WIFI模块的 IP 地址
    esp_sendCMD("AT+CIPSTART=\"UDP\",\""+remote_IP+"\","+remote_Port+","+local_Port+",0","OK",10000) # 创建 UDP 传输
    esp_sendCMD("AT+CIPMODE=1","OK")    # 开启透传模式，数据可以直接传输 
    esp_sendCMD("AT+CIPSEND",">")       # 发送数据 

    esp_uart.write('Hello makerobo !!!\r\n')       # 发送对应的字符串
    esp_uart.write('RP2040 UDP message!\r\n')
    while True:
        s_get=esp_uart.read()                      # 接收字符
        if(s_get != None):                         # 判断字符不为空
            s_get=s_get.decode()             
            print(s_get)                           # 字符串打印
            esp_uart.write(s_get)                  # 字符串回传    
