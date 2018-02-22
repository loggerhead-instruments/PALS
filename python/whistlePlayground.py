#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Feb 21 16:44:41 2018

@author: dmann
"""

import matplotlib.pyplot as plt
import numpy as np
import scipy.io.wavfile as wav
import scipy.signal as signal
import glob, os


path = '/Users/dmann/w/AMS/python/testSignals/'
#fileName = '2017-06-13T205600_0004e9e500042ae3_2.0.wav'
fileName = 'whistleTest.wav'
Fs, y = wav.read(path + fileName)
# frequency resolution
fftPts = 256
binWidth = Fs / fftPts
fftDurationMs = 1000.0 / binWidth
startBin = int(startFreq/binWidth)
endBin = int(endFreq/binWidth)

### Settings that can be tweaked to change sensitivity

# frequency range to look for peak
startFreq = 5000
endFreq = 22000

# adjacent bins need to be within x Hz of each other to add to runLength
whistleDelta = 1500.0 # default = 500

# minimum run length to count as whistle
minRunLength = 150.0 / fftDurationMs # default = 300

# candidate whistle must cover this number of bins
fmThreshold = 500.0 # default = 1000

### End user settings


# run through all files
os.chdir(path)
for fileName in glob.glob('*.wav'):
    # step through chunks
    index = 0
    peaks = []
    toneIndex = []
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
    for start in range(0, len(y)-fftPts, fftPts):
        chunk = y[start:start+fftPts] * np.hanning(len(y[start:start+fftPts]))
        Y = abs(np.fft.fft(chunk)) / fftPts # fft and normalization
        peakFrequency = (Y[startBin:endBin].argmax() + startBin) * binWidth
        peakAmplitude = Y[Y[startBin:endBin].argmax() + startBin]
        refAmplitude = Y[Y[startBin:endBin].argmax() + startBin - 6 : Y[startBin:endBin].argmax() + startBin - 2].mean()
        peaks.append(peakFrequency)
        toneIndex.append(peakAmplitude/refAmplitude)
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
    
    
    print(whistles)
    
    plt.plot(whistles, np.zeros(len(whistles)), 'bo')
    plt.subplot(3,1,2)
    plt.plot(peaks)
    plt.subplot(3,1,3)
    plt.plot(toneIndex)
    plt.subplot(3, 1, 1)
    plt.specgram(y, NFFT=fftPts, Fs=Fs, noverlap=0, cmap=plt.cm.gist_heat)
    plt.show()
    wait = input("PRESS ENTER TO CONTINUE.")



