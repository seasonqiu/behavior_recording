# script adopted from https://makersportal.com/blog/2018/2/25/python-datalogger-reading-the-serial-output-from-arduino-to-analyze-data-using-pyserial
import serial
import time
import csv
from datetime import datetime
import sys

port = str(sys.argv[1])
save_path = "/home/pi/buffer/" + str(sys.argv[2]) + "_" + datetime.now().strftime('%Y%m%d_%H%M%S%f') + ".csv"
ser = serial.Serial(port)

#ser = serial.Serial('/dev/cu.usbmodem1301')
#ser = serial.Serial('/dev/cu.usbmodem11301')
ser.flushInput()
#save_path = "/Users/season/buffer/event_log_" + datetime.now().strftime('%Y%m%d_%H%M%S%f') + ".csv"
first_row = [['time','event','weight_lever','sync_state','duration_hold','weight_lever_threshold']]
with open(save_path,mode='w',newline='') as file:
    writer = csv.writer(file)
    writer.writerows(first_row)
while True:
    try:
        ser_bytes = ser.readline()
        decoded_bytes = ser_bytes[0:len(ser_bytes)-2].decode('UTF-8')
        print(decoded_bytes)
        current_event = decoded_bytes.split(",")
        with open(save_path,"a") as f:
            writer = csv.writer(f,delimiter=",")
            current_event.insert(0,time.time())
            writer.writerow(current_event)
    except Exception as e:
        print("Keyboard Interrupt")
        print(e)
        break
