
import matplotlib
import numpy as np
import cmsisdsp as dsp
from scipy import signal
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

np.seterr(over='ignore')

F_SIGNAL = np.int16(500)
F_SAMPLING = np.int32(8000)
LEN_SIGNAL_S = 0.001

SG_ANGLE_INC_Q31 = np.int32((np.float32(0x1 << 31) * np.float32(2.0) * np.pi * np.float32(F_SIGNAL)) / np.pi / np.float32(F_SAMPLING))
LEN_SIGNAL_N_SAMPLES = np.uint32(np.ceil(LEN_SIGNAL_S * F_SAMPLING))

print(f'SIGNAL FREQUENCY:       {F_SIGNAL} Hz')
print(f'SAMPLING FREQUENCY:     {F_SAMPLING} Hz')
print(f'ANGLE INC Q31:          {SG_ANGLE_INC_Q31}')
print(f'N SAMPLES:              {LEN_SIGNAL_N_SAMPLES}')

samples_sin = []
samples_cos = []
angles = []
current_angle = np.int32(0)
for i in range(LEN_SIGNAL_N_SAMPLES):
    angles += [current_angle]
    s, c = dsp.arm_sin_cos_q31(current_angle)
    s = np.right_shift(np.int32(s), 16)
    c = np.right_shift(np.int32(c), 16)
    samples_sin += [s]
    samples_cos += [c]
    current_angle += SG_ANGLE_INC_Q31

fig, axs = plt.subplots(2)
axs[0].plot(samples_sin, 'o')
axs[0].plot(samples_cos, 'o')
axs[1].plot(angles)
plt.show()


