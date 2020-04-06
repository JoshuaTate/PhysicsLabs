#import relevant libraries and functions
from mcculw import ul
from mcculw.enums import ULRange
import ctypes
from mcculw.enums import ScanOptions
import matplotlib.pyplot as plt
import numpy as np

#choosing parameters
board_num = 0
channel1 = 0
channel2 = 1
ai_range = ULRange.BIP5VOLTS
sample_rate = 2000
no_samples = 20000

#convert memhandle to array
def memhandle_as_ctypes_array(memhandle):
    return ctypes.cast(memhandle, ctypes.POINTER(ctypes.c_ushort))

#store data as an array
memhandle = ul.win_buf_alloc(no_samples)
ctypes_array = memhandle_as_ctypes_array(memhandle)

#taking measurements
ul.a_in_scan(board_num, channel1, channel2, no_samples, sample_rate, ULRange.BIP5VOLTS, memhandle, ScanOptions.FOREGROUND)

#create empty arrays for each channel
array_ch1 = [0] * int(no_samples/2)
array_ch2 = [0] * int(no_samples/2)

angle = np.arctan2(array_ch1, array_ch2)

for i in range(int(no_samples/2)):
    array_ch1[i] = ctypes_array[2*i]
    array_ch2[i] = ctypes_array[2*i + 1]

#time array
t = (1/sample_rate)*np.linspace(0, 1 + no_samples/2, no_samples/2)

array_ch1 = array_ch1 - np.mean(array_ch1)
array_ch2 = array_ch2 - np.mean(array_ch2)

angle = np.unwrap(np.arctan2(array_ch1, array_ch2))
np.savetxt("angles.csv", angle, delimiter=",")

#plot signals against time and as a lissajous
fig_1 = plt.figure(1)
plt.plot(t, array_ch1, )
plt.xlabel(' Time / s ')
plt.ylabel('D1 Amplitude / V')
plt.title('Voltage-Time Plot for D1')
fig_1.savefig('Voltage-Time Plot for D1.png')

fig_2 = plt.figure(2)
plt.plot(t, array_ch2, )
plt.xlabel(' Time / s ')
plt.ylabel('D2 Amplitude / V' )
plt.title('Voltage-Time Plot for D2')
fig_2.savefig('Voltage-Time Plot for D2.png')

fig_3 = plt.figure(3)
plt.plot(array_ch1, array_ch2)
plt.xlabel(' D2 Amplitude / V')
plt.ylabel('D1 Amplitude / V')
plt.title('Lissajous Figure of D1 Against D2')
fig_3.savefig('Lissajous Figure of D1 Against D2.png')

fig_4 = plt.figure(4)
plt.plot(t, angle)
plt.ylabel(' Angle / rad')
plt.xlabel('Time / s')
plt.title('Angle Subtended by the Lissajous Figure as a Function of Time')
fig_4.savefig('Angle Subtended by the Lissajous Figure as a Function of Time.png')

#calculate refractive index from the angle data
N = (angle[-1] - angle[1])/(2*np.pi)
n = 1 + 6.33e-6*1.0548*N
print("The Refractive Index is %s"%(n))
