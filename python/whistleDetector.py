import matplotlib.pyplot as plt
import numpy as np
import scipy.io.wavfile as wav
import scipy.signal as signal
import glob, os

fftPts = 256


path = '/Users/dmann/w/AMS/python/testSignals/'
fileName = 'whistleTest.wav'
# fileName = 'whis6.wav'

# run through all files
os.chdir(path)
#for fileName in glob.glob('*.wav'):

Fs, y = wav.read(path + fileName)
whistles, peaks = whistleDetect(y)
print(whistles)

plt.subplot(2, 1, 1)
plt.specgram(y, NFFT=fftPts, Fs=Fs, noverlap=0, cmap=plt.cm.gist_heat)
plt.plot(whistles, np.zeros(len(whistles)), 'bo')
plt.title('original')

y = deSnap(y)
whistles = whistleDetect(y)
print(whistles)

plt.subplot(2, 1, 2)
plt.specgram(y, NFFT=fftPts, Fs=Fs, noverlap=0, cmap=plt.cm.gist_heat)
plt.plot(whistles, np.zeros(len(whistles)), 'bo')
plt.title('deSnap')
plt.show()


def whistleDetect(y):
    # frequency resolution

    binWidth = Fs / fftPts
    fftDurationMs = 1000.0 / binWidth
    
    ### Settings that can be tweaked to change sensitivity
    
    # frequency range to look for peak
    startFreq = 5000
    endFreq = 22000
    startBin = int(startFreq/binWidth)
    endBin = int(endFreq/binWidth)
    
    # adjacent bins need to be within x Hz of each other to add to runLength
    whistleDelta = 1500.0 # default = 500
    
    # minimum run length to count as whistle
    minRunLength = 150.0 / fftDurationMs # default = 300
    
    # candidate whistle must cover this number of bins
    fmThreshold = 500.0 # default = 1000
    
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
    return whistleIndex, peaks

def deSnap(y):
    # declick signal
    clickDur = 0.005 # click duration in seconds
    clickPts = int(clickDur * Fs)
    rewindPts = clickPts
    preClickPts = 40
    nStd = 1.5
    
    # 1. diffence and threshold to find clicks
    yDiff = abs(np.diff(y))
    ySmooth = signal.convolve(yDiff, np.ones(10)/10.0) # moving average
    clickThreshold = nStd * ySmooth.std() # set threshold to nStd * rms of diff signal
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


    return y2
    
main()