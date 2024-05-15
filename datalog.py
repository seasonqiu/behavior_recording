# script adopted from https://makersportal.com/blog/2018/2/25/python-datalogger-reading-the-serial-output-from-arduino-to-analyze-data-using-pyserial
import serial
import time
import csv

ser = serial.Serial('/dev/cu.usbmodem1301')
#ser = serial.Serial('/dev/cu.usbmodem11301')
ser.flushInput()

while True:
    try:
        ser_bytes = ser.readline()
        decoded_bytes = ser_bytes[0:len(ser_bytes)-2].decode('UTF-8')
        print(decoded_bytes)
        current_event = decoded_bytes.split(",")
        with open("test_data.csv","a") as f:
            writer = csv.writer(f,delimiter=",")
            current_event.insert(0,time.time())
            writer.writerow(current_event)
#            writer.writerow([time.time(),decoded_bytes])
    except Exception as e:
        print("Keyboard Interrupt")
        print(e)
        break
