#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－湖南创乐博智能科技有限公司－－－－
#  文件名：TCP_Client.py
#  版本：V2.0
#  author: zhulin
# 说明：WIFI TCP 服务器端通讯案例
#---------------------------------------
from machine import UART, Pin
import utime,time

# WIFI 路由器信息，请填上自己的WIFI路由器信息
SSID='***********'       # WIFI名称
password = '*********'   # WIFI密码
Port = '8080'            # 自定义端口号

# 串口映射到GP0和GP1端口上，在使用该端口和
# WIFI模块通讯时，不要使用GP0和GP1端口
esp_uart = UART(0, 115200) # 串口0,波特率为115200

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

# 发送数据
def esp_sendData(ID,data):
    esp_sendCMD('AT+CIPSEND='+str(ID)+','+str(len(data)),'>')
    esp_uart.write(data)

# 接收数据
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

# 程序入口
if __name__ == "__main__":
    esp_uart.write('+++')            # 初始化退出透传模式
    time.sleep(1)
    if(esp_uart.any()>0):
        esp_uart.read()
    esp_sendCMD("AT","OK")           # AT指令
    esp_sendCMD("AT+CWMODE=3","OK")  # 配置 WiFi 模式
    esp_sendCMD("AT+CWJAP=\""+SSID+"\",\""+password+"\"","OK",20000) # 连接路由器
    esp_sendCMD("AT+CIPMUX=1","OK")            # 使能多连接 
    esp_sendCMD("AT+CIPSERVER=1,"+Port,"OK")   # 建⽴ TCP server
    esp_sendCMD("AT+CIFSR","OK")               # 查询 WIFI模块的 IP 地址

    while True:
        ID,s_get=esp_ReceiveData()            # 接收数据
        if(ID != None):
            esp_sendData(ID,s_get)            # 如果接收到的数据不为空，则回传
