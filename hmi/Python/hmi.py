import mmap
import struct
import time
from python_tools import ProcessImage

#run cmd => python3 python_tools/hmi.py
with open("/dev/shm/process_image", "r+b") as f:
    mm = mmap.mmap(f.fileno(), 0)

    while True:
        di = struct.unpack("H", mm[4:6])[0]
        do = struct.unpack("H", mm[8:10])[0]

        print(f"DI: {di}  DO: {do}")

        alarm_active = struct.unpack("I", mm[16:20])[0]
        alarm_latched = struct.unpack("I", mm[20:24])[0]

        print(f"ALARMS: {bin(alarm_active)}")
        print(f"LATCHED: {bin(alarm_latched)}")

        time.sleep(0.5)

with open("/dev/shm/process_image", "r+b") as f:
    mm = mmap.mmap(f.fileno(), 0)
    pi = ProcessImage.from_buffer(mm)

    print(pi.version, pi.can.rx_count, pi.canin.di, pi.app.state)