import matplotlib.pyplot as plt
import numpy as np
import scipy.io.wavfile as wav

path = '/w/loggerhead/AMS/python/testSignals/'
fileName = '2017-06-13T205600_0004e9e500042ae3_2.0.wav'

Fs, y = wav.read(path + fileName)

# frequency resolution
fftPts = 512
binWidth = Fs / fftPts
fftDurationMs = 1000.0 / binWidth

### Settings that can be tweaked to change sensitivity

# frequency range to look for peak
startFreq = 5000
endFreq = 20000
startBin = int(startFreq/binWidth)
endBin = int(endFreq/binWidth)

# adjacent bins need to be within x Hz of each other to add to runLength
whistleDelta = 1500.0 # default = 500

# minimum run length to count as whistle
minRunLength = 200.0 / fftDurationMs # default = 300

# candidate whistle must cover this number of bins
fmThreshold = 1000.0 # default = 1000

### End user settings

# step through chunks
index = 0
peaks = []
runLength = 0
whistleCount = 0
maxPeakFreq = 0
minPeakFreq = endFreq
whistles = []
whistleIndex = []
rlIndex = []
rlPoints = []
oldPeakFrequency = 0

# no overlap of FFTs
for start in range(0, len(y), fftPts):
    Y = abs(np.fft.fft(y[start:start+fftPts])) / fftPts # fft and normalization
    peakFrequency = (Y[startBin:endBin].argmax() + startBin) * binWidth
    peaks.append(peakFrequency)
    if(peakFrequency > maxPeakFreq):
        maxPeakFreq = peakFrequency
    if(peakFrequency < minPeakFreq):
        minPeakFreq = peakFrequency
    if(abs(peakFrequency - oldPeakFrequency) < whistleDelta):
        runLength+=1
    else:
        # end of run
        if((runLength>minRunLength) &
            (maxPeakFreq - minPeakFreq > fmThreshold)):
            # store detected whistles
            whistleCount += 1
            whistles.append(peakFrequency)
            whistleIndex.append(index * (fftPts/Fs))
        # store run lengths
        rlIndex.append(index * (fftPts/Fs))
        rlPoints.append(runLength * fftDurationMs)
        maxPeakFreq = 0
        minPeakFreq = endFreq
        runLength = 0

    oldPeakFrequency = peakFrequency
    index = index + 1

t = np.arange(0, len(peaks)) * (fftPts/Fs) # time vector
fig, ax1 = plt.subplots()
ax1.plot(t, peaks, 'b.')
ax1.plot(whistleIndex, whistles, 'ro')

ax1.set_ylabel('Peak Frequency (Hz)')
ax1.set_xlabel('Time (s)')

ax2 = ax1.twinx() #create second y-axis for run length
ax2.plot(rlIndex, rlPoints, 'g.')
ax2.set_ylabel('Run Length')
plt.title(fileName)
plt.show()