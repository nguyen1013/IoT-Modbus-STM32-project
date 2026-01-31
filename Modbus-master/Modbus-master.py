"""
Author: Tommi Rintala <tommi.rintala@vamk.fi>
Copyright: GPL v3 2024 All rights reserved

This simple program sends Modbus request to port (defined in source)
and prints out the result to console.

Demo will show negative values!!

Install:
    pip install serial
    pip install pyserial
"""

import time
import random
import serial

port = serial.Serial('COM5', 
                     baudrate = 9600, 
                     timeout=2, 
                     bytesize=serial.EIGHTBITS,
                     parity=serial.PARITY_NONE, 
                     stopbits=serial.STOPBITS_ONE,
                     )
global received
port.close()

def modbusCrc(msg:str) -> int:
    crc = 0xFFFF
    for n in range(len(msg)):
        crc ^= msg[n]
        for i in range(8):
            if crc & 1:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc

#crc = modbusCrc(msg)
    #01 04 00 01 00 01 60 0A
    # https://npulse.net/en/online-modbus

def modbus_request(slave_addr, register_addr):
    # Build request frame: Read 2 input registers
    msg = [
        slave_addr,
        0x04,
        (register_addr >> 8) & 0xFF,
        register_addr & 0xFF,
        0x00,
        0x02  # quantity = 2 registers
    ]

    crc = modbusCrc(msg)
    msg.append(crc & 0xFF)
    msg.append((crc >> 8) & 0xFF)

    print("{}: request: {}".format(time.strftime('%Y%m%d:%H%M%S'), msg))

    port.open()
    port.write(msg)

    # Expected response: 9 bytes
    received = port.read(9)
    port.close()

    if len(received) == 9:
        print("{}: respond: {}".format(time.strftime('%Y%m%d:%H%M%S'), received))

        # 32-bit big-endian value
        value = (
            (received[3] << 24) |
            (received[4] << 16) |
            (received[5] << 8)  |
             received[6]
        )

        # signed conversion
        if value & 0x80000000:
            value -= 0x100000000

    else:
        value = -9999
        print("{}: Invalid response length: {}".format(
            time.strftime('%Y%m%d:%H%M%S'), received))

    time.sleep(1)
    return value


devices = range(0x01, 0x08)

while True:
    # read temperature from slave 0x01
    temp_x100 = modbus_request(0x01, 0x01)
    if -5000 <= temp_x100 <= 15000:
        print(temp_x100 / 100, "Celsius")    
    time.sleep(2)
    
    # read luminance from slave 0x02
    lux = modbus_request(0x02, 0x01)
    if lux >=0:
        print(lux, "Lux")

    time.sleep(2)
