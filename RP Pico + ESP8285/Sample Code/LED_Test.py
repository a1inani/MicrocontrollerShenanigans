#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# －－－－Hunan Chuanglebo Intelligent Technology Co., Ltd.－－－－
#  File name：LED_Test.py
#  Version：V2.0
#  author: zhulin
#---------------------------------------
from machine import Pin, Timer  # Load the corresponding library

led = Pin('LED', Pin.OUT)   # Onboard LED definition, set to output mode

tim = Timer()               # define a counter

# function, specifying the time to flip the level
def tick(timer):             
    global led
    led.toggle()
    
# Initialize the counter, the frequency is 2.5HZ
tim.init(freq=2.5, mode=Timer.PERIODIC, callback=tick)
