
import numpy as np
from scipy.signal import freqz
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

FS_IN = 1.024e6
FS_OUT = 8e3
DECIMATION_FACTOR = 16

def frequency_regions_after_decimation():
    f_harmonics = FS_IN / DECIMATION_FACTOR
    output = []
    f = 0
    while f < (FS_IN / 2):
        if 0 == f:
            output += [(0, f + FS_OUT)]
        else:
            output += [(f - FS_OUT, f + FS_OUT)]
        f += f_harmonics
    output += [(FS_IN/2 - FS_OUT, FS_IN/2)]
    return output 

def draw_output_frequency_patch(ax):
    ymin, ymax = ax.get_ylim()
    rect = Rectangle((0, ymin), FS_OUT, ymax-ymin, 
                    facecolor='lightgreen', alpha=0.3)  # alpha makes it semi-transparent
    ax.add_patch(rect)

def draw_frequency_region_patch(axs, f_min, f_max):
    width = f_max - f_min
    ymin, ymax = ax.get_ylim()
    rect = Rectangle((f_min, ymin), width, ymax-ymin, 
                    facecolor='lightgreen', alpha=0.3)  # alpha makes it semi-transparent
    ax.add_patch(rect)


def plot_freqs(axs, len, order=1):
    h = np.ones(len) / len     # pulse response - same as zeros
    w, H = freqz(h, worN=1024)
    H = H**order
    f = w * FS_IN / (2 * np.pi)
    axs.plot(f, 20*np.log10(abs(H)), label=f'Len: {len}, Order: {order}')
    #axs.plot(f, abs(H), label=f'Len: {len}, Order: {order}')

def plot_filter_characteristic(axs, lengths, order):
    axs.set_xlabel("Frequency (Hz)")
    axs.set_ylabel("Magnitude (dB)")
    for l in lengths:
        plot_freqs(axs, l, order)
    axs.legend()
    axs.grid() 

filter_lengths = [2, 4, 8, 16, 32]
frequency_bands = frequency_regions_after_decimation()
"""
fig, axs = plt.subplots(nrows=1, ncols=4, sharey=True, sharex=True)

plot_filter_characteristic(axs[0], filter_lengths, 1)
plot_filter_characteristic(axs[1], filter_lengths, 2)
plot_filter_characteristic(axs[2], filter_lengths, 4)
plot_filter_characteristic(axs[3], filter_lengths, 8)

ylim_min = min([ax.get_ylim()[0] for ax in axs])
ylim_max = max([ax.get_ylim()[1] for ax in axs])


for ax in axs:
    ax.set_ylim((ylim_min, ylim_max))
    for fb in frequency_bands:
        draw_frequency_region_patch(ax, fb[0], fb[1])
"""

fig, ax = plt.subplots()
plot_filter_characteristic(ax, filter_lengths, 5)    
for fb in frequency_bands:
    draw_frequency_region_patch(ax, fb[0], fb[1])

#print(frequency_regions_after_decimation())

plt.show()