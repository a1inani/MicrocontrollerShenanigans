#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－湖南创乐博智能科技有限公司－－－－
#  文件名：LED测试程序.py
#  版本：V2.0
#  author: zhulin
# 说明：板载LED灯闪烁
#---------------------------------------
from machine import Pin, Timer  # 加载对应的库

led = Pin('LED', Pin.OUT)   # 板载LED定义，设置为输出模式

tim = Timer()               # 定义一个计数器

# 子函数，规定时间翻转电平
def tick(timer):             
    global led
    led.toggle()
    
#初始化计数器，频率为2.5HZ
tim.init(freq=2.5, mode=Timer.PERIODIC, callback=tick)
