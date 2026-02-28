import time
import serial

port = serial.Serial('COM13',
                     baudrate=9600,
                     timeout=1,
                     bytesize=serial.EIGHTBITS,
                     parity=serial.PARITY_NONE,
                     stopbits=serial.STOPBITS_ONE)

def modbus_crc(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc

def modbus_request(slave_addr: int, register_addr: int) -> int:
    print()
    frame = bytearray([
        slave_addr,
        0x04,
        (register_addr >> 8) & 0xFF,
        register_addr & 0xFF,
        0x00,
        0x02
    ])
    crc = modbus_crc(frame)
    frame.append(crc & 0xFF)        # CRC low
    frame.append((crc >> 8) & 0xFF) # CRC high

    print("{}: request: {}".format(time.strftime('%Y%m%d:%H%M%S'), frame))

    for attempt in range(1, 4):
        try:
            if not port.is_open:
                port.open()

            port.reset_input_buffer()
            port.reset_output_buffer()

            port.write(frame)
            # small pause to let slave respond; tune if needed
            time.sleep(0.05)

            resp = port.read(9)  # expected length for 2 registers
            if len(resp) != 9:
                raise IOError(f"Short/empty response (len={len(resp)})")
            else:
                print("{}: respond: {}".format(time.strftime('%Y%m%d:%H%M%S'), resp))

            # CRC check
            resp_crc = resp[-2] | (resp[-1] << 8)
            if modbus_crc(resp[:-2]) != resp_crc:
                raise IOError("CRC mismatch")

            # parse 32-bit big-endian from bytes 3..6
            value = (resp[3] << 24) | (resp[4] << 16) | (resp[5] << 8) | resp[6]
            if value & 0x80000000:
                value -= 0x100000000
            return value

        except Exception as e:
            print(f"{time.strftime('%Y%m%d:%H%M%S')}: attempt {attempt} error: {e}")
            # clear buffers and small backoff before retry
            try:
                port.reset_input_buffer()
            except Exception:
                pass
            time.sleep(0.2 * attempt)
    return -9999



devices = range(0x01, 0x08)

while True:
    # # -------- Read temperature from slave 0x01 --------
    # temp_x100 = modbus_request(0x01, 0x01)

    # if temp_x100 == -9990:
    #     print("Temperature Sensor ERROR")
    # elif -5000 <= temp_x100 <= 15000:
    #     print(temp_x100 / 100, "Celsius")
    # else:
    #     print("Temperature Invalid Response:", temp_x100)

    # time.sleep(2)



    # # -------- Read luminance from slave 0x02 --------
    # lux = modbus_request(0x02, 0x01)

    # if lux == -9990:
    #     print("Luminance Sensor ERROR")
    # elif lux >= 0:
    #     print(lux, "Lux")
    # else:
    #     print("Luminance Invalid Response:", lux)

    # time.sleep(2)



    # # read temperaturex100, humidityx100 from DHT22 (each value using 2 databytes)
    # # Slave packs: ((uint16_t)temp_x100 << 16) | (uint16_t)hum_x100
    # # -------- Read temperature & humidity from DHT22 (slave 0x03) --------
    raw_dht = modbus_request(0x03, 0x01)

    if raw_dht == -9990:
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: DHT22 Sensor ERROR (-9990)")
    elif raw_dht == -9999:
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: DHT22 read failed (-9999)")
    else:
        temp16 = (raw_dht >> 16) & 0xFFFF
        hum16  = raw_dht & 0xFFFF

        # Sign-extend temperature if negative
        if temp16 & 0x8000:
            temp16 -= 0x10000

        temperature_c = temp16 / 100.0
        humidity_pct  = hum16 / 100.0

        print(f"{time.strftime('%Y%m%d:%H%M%S')}: DHT22 Temperature = {temperature_c:.2f} °C, Humidity = {humidity_pct:.2f} %RH")

    time.sleep(2)

    # -------- Read eCO2 & TVOC from SGP30 (slave 0x04) --------
    # read eCO2, TVOC from Grove SGP30 (each value using 2 databytes)
    # Slave packs: ((uint16_t)eco2_ppm << 16) | (uint16_t)tvoc_ppb
    raw_sgp = modbus_request(0x04, 0x01)

    if raw_sgp == -9990:
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: SGP30 Sensor ERROR (-9990)")
    elif raw_sgp == -9999:
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: SGP30 read failed (-9999)")
    else:
        eco2 = (raw_sgp >> 16) & 0xFFFF
        tvoc = raw_sgp & 0xFFFF
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: SGP30 eCO2 = {eco2} ppm, TVOC = {tvoc} ppb")

    time.sleep(2)


    # -------- Read AC Voltage (slave 0x05) --------
    voltage_mv = modbus_request(0x05, 0x01)

    if voltage_mv == -9990:
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: AC Voltage Sensor ERROR")
    elif 0 <= voltage_mv <= 24000:
        voltage_v = voltage_mv / 1000.0
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: AC Voltage = {voltage_v:.2f} V")
    else:
        print("AC Voltage Invalid Response:", voltage_mv)

    time.sleep(2)


    # -------- Read Power Consumption (slave 0x06) --------
    power_mw = modbus_request(0x06, 0x01)

    if power_mw == -9990:
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: Power Sensor ERROR")
    elif 0 <= power_mw <= 24000:
        power_w = power_mw / 1000.0
        print(f"{time.strftime('%Y%m%d:%H%M%S')}: Power = {power_w:.2f} W")
    else:
        print("Power Invalid Response:", power_mw)

    time.sleep(2)

    