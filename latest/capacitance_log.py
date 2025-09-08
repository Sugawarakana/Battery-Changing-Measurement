from serial import Serial
import csv
from datetime import datetime

ser = Serial('COM4', 115200)   # serial port

def get_csv_writer():
    f = open('capacitance_data.csv', 'a', newline='', encoding='utf-8')
    writer = csv.writer(f)
    if f.tell() == 0:
        writer.writerow(['time', 'cap0', 'cap1', 'cap2', 'cap3'])
    return f, writer

if __name__ == '__main__':
    f, writer = get_csv_writer()
    count = 0
    caps = [''] * 4
    flag = 0
    while count < 45:
        line = ser.readline().decode('utf-8')[:-1]
        # print(line)
        if "0: " in line:
            flag = 100
        for i in range(4):
            identifier = str(i) + ': '
            idx = line.find(identifier)
            # print(identifier)
            # print(idx)
            if idx != -1:
                val = line.split(': ')[1]
                caps[i] = val[:-1]
               
        f.flush()
        count += 1 # comment this line to run indefinitely
        flag += 1
        if flag == 104:
            now = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            writer.writerow([now] + caps)
    f.close()