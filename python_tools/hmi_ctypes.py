import ctypes

class In(ctypes.Structure):
    _fields_ = [
        ("di_", ctypes.c_uint16), 
        ("ai_1", ctypes.c_uint16)]

class Out(ctypes.Structure):
    _fields_ = [
        ("do_", ctypes.c_uint16), 
        ("ao_1", ctypes.c_uint16)]

class Can(ctypes.Structure):
    _fields_ = [
        ("rx_count", ctypes.c_uint16),
        ("tx_count", ctypes.c_uint16),
        ("error_count", ctypes.c_uint16),
        ("last_rx_time", ctypes.c_uint32),
        ("in", In),
        ("out", Out),
    ]

class App(ctypes.Structure):
    _fields_ = [
        ("pid_output", ctypes.c_float), 
        ("state", ctypes.c_uint32), 
        ("alarm_active", ctypes.c_uint32), 
        ("alarm_latched", ctypes.c_uint32)
        ]

class Config(ctypes.Structure):
    _fields_ = [
        ("setpoint", ctypes.c_float), 
        ("kp", ctypes.c_float), 
        ("ki", ctypes.c_float), 
        ("kd", ctypes.c_float), 
        ("cycle_time_ms", ctypes.c_uint16)
    ]

class Metrics(ctypes.Structure):
    _fields_ = [
        ("exec_time", ctypes.c_uint32), 
        ("max_exec_time", ctypes.c_uint32), 
        ("jitter", ctypes.c_int32), 
        ("max_jitter", ctypes.c_int32), 
        ("deadline_miss", ctypes.c_uint32)
    ]

class ProcessImage(ctypes.Structure):
    _fields_ = [
        ("version", ctypes.c_uint32),
        ("timestamp_ms", ctypes.c_uint32),
        ("can", Can),
        ("config", Config),  
        ("app", App),
        ("metrics", Metrics)
        # ... add remaining fields
    ]

