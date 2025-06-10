from serial import Serial

ser = Serial('COM4', 115200)   # serial port

def getFileID():
    try:
        f = open('arduino_data.txt', 'a')
        f0 = open('cap0_data.txt', 'a')
        f1 = open('cap1_data.txt', 'a')
        f2 = open('cap2_data.txt', 'a')
        f3 = open('cap3_data.txt', 'a')
    except:
        f = open('arduino_data.txt', 'w')
        f0 = open('cap0_data.txt', 'w')
        f1 = open('cap1_data.txt', 'w')
        f2 = open('cap2_data.txt', 'w')
        f3 = open('cap3_data.txt', 'w')
    return f, f0, f1, f2, f3



if __name__ == '__main__':
    f_lst = list(getFileID())
    # print(f_lst)
    while True:
        line = ser.readline().decode('utf-8')[:-1]
        f_lst[0].write(line) # delete \n
        f_lst[0].flush()
        print(line)
        for i in range(1, 5):
            identifier = str(i - 1) + ':'
            if identifier in line:
                f_lst[i].write(line)
                f_lst[i].flush()
        
    # f.close()
