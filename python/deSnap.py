# deSnap
# simple detection and removal of snapping shrimp clicks
# replace clicks with short segment of signal from just before click

import matplotlib.pyplot as plt
import numpy as np
import scipy.io.wavfile as wav
import scipy.signal as signal

path = '/w/loggerhead/AMS/python/testSignals/'
fileName = '2017-06-13T205600_0004e9e500042ae3_2.0.wav'

Fs, y = wav.read(path + fileName)

# Things to try if have issues with introducing clicks from repeated segments
# - high pass filter before de-click
# - smooth across edges of insert


clickDur = 0.005 # click duration in seconds
clickPts = int(clickDur * Fs)
rewindPts = clickPts
preClickPts = 40

# 1. diffence and threshold to find clicks
yDiff = abs(np.diff(y))
ySmooth = signal.convolve(yDiff, np.ones(10)/10.0) # moving average
clickThreshold = 5 * ySmooth.std() # set threshold to 10 * rms of diff signal
click = ySmooth > clickThreshold # threshold to detect clicks
clickDiff = np.diff(click.astype(np.int)) # difference to get start and end of clicks

# 2. replace clicks with short segment from rewindPts before
indicesToReplace = np.argwhere(clickDiff == 1) - preClickPts
y2 = np.copy(y) # make copy
for i in indicesToReplace:
    for j in range(0, clickPts-1):
        goodIndex = i - rewindPts + j
        if (goodIndex<0): # make sure not before start of signal
            goodIndex = 0
        if(i+j<len(y2)):
            y2[i+j] = y2[goodIndex]

plt.plot(y, 'b')
plt.plot(y2, 'r')
plt.show()
