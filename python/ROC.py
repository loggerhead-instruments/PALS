#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jul 22 16:36:06 2018

@author: dmann
"""
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

path = '/Users/dmann/w/AMS/python/'
fileName = 'PalmaSolaAnalysis.csv'

metric = 'bins3'

# import data

data = pd.read_csv(path + fileName)

noWhistle = data.where(data.Whistle==0).dropna()
whistle = data.where(data.Whistle==1).dropna()

totalNoWhistle = len(noWhistle)
totalWhistle = len(whistle)

ROC = pd.DataFrame(columns=list('tf'))

# loop through threshold
for i in range(0, 200):
    # true positive = correct detection / total with whistle
    detection = whistle[metric]>i
    percentCorrect = sum(detection) / totalWhistle
    # false positive = false detection / total no whistle
    detection = noWhistle[metric]>i
    percentFalsePositive = sum(detection) / totalNoWhistle
    
    ROC.loc[i] = ([percentCorrect, percentFalsePositive])
    
plt.plot(ROC.f, ROC.t)
plt.ylabel('True Positive')
plt.xlabel('False Positive')
plt.ylim([0,1])
plt.xlim([0,1])