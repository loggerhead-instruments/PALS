import matplotlib.pyplot as plt
import numpy as np
import scipy.io.wavfile as wav

# range to look for peak
startFreq = 5000
endFreq = 10000
fftPts = 512

path = '/w/loggerhead/AMS/python/testSignals/'
fileName = '2017-06-13T205600_0004e9e500042ae3_2.0.wav'
# Create sine wave for test
# Fs = 150.0;  # sampling rate
# Ts = 1.0/Fs; # sampling interval
# t = np.arange(0,1,Ts) # time vector

# ff = 5;   # frequency of the signal
# y = np.sin(2*np.pi*ff*t)

Fs, y = wav.read(path + fileName)
binWidth = Fs / fftPts
startBin = int(startFreq/binWidth)
endBin = int(endFreq/binWidth)
hpts = int(fftPts/2)

# step through chunks
index = 0
peaks = []

# no overlap of FFTs
for start in range(0, len(y), fftPts):
    Y = abs(np.fft.fft(y[start:start+fftPts])) / fftPts # fft and normalization
    peakFrequency = (Y[startBin:endBin].argmax() + startBin) * binWidth
    peaks.append(peakFrequency)
    index = index + 1

Ts = 1.0 / Fs; # sampling interval
t = np.arange(0, len(peaks)) * (Fs/fftPts) # time vector
plt.plot(t, peaks, '.')
plt.title(fileName)
plt.ylabel('Peak Frequency (Hz)')
plt.xlabel('Time (s)')
plt.show()