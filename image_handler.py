import os
import sys
import serial
import signal
import threading
import time

countI = 0
exitThread = False   # ������ ����� ����
# ���� ����
file = open('F:/output/sample.txt', 'wb')

with open('F:/output/sample.jpg', 'rb') as image:
  f = image.read()
  b= bytearray(f)
  file.write("#define IMAGE_LENGTH %d " % len(b))
  file.write("unsigned char sample_image[IMAGE_LENGTH] = {")
  for item in list(b):
    file.write("%s, " % item)
  file.seek(-2, os.SEEK_CUR)
  file.write("};");
  file.flush()

file = open('F:/output/output.jpg', 'wb')
#������ ����� �ñ׳� �Լ�
def handler(signum, frame):
     exitThread = True

#������ ó���� �Լ�
def readThread(ser):
    global exitThread, file, countI
    while not exitThread:
       #�����Ͱ� ���ִٸ�
       for c in ser.read():
          if(countI >= 5):
             print(c)
             file.write(c)
             file.flush()
          if(c == 'i'):
             countI += 1;
          

#���� �ñ׳� ���
signal.signal(signal.SIGINT, handler)

#�ø��� ����
ser = serial.Serial('COM17', 115200)

#�ø��� ���� ������ ����
thread = threading.Thread(target=readThread, args=(ser,))

# ����
thread.start()
