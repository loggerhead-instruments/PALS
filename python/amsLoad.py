import json
import pandas as pd

filename = '/w/loggerhead/AMS/python/AMS.json'
startString = 'data'

with open(filename, 'r') as data:
    lines = data.readlines()

df = pd.DataFrame()
df2 = pd.DataFrame()
whistles = []
dt = []
stationId = []
b0 =[]
b1 = []
b2 = []
b3 = []

for row in lines:
    goodJson = row[0:len(row)-1]
    try:
        df = pd.read_json(goodJson)
        w = (int(df.data.m['w']['n']))
        band0 = (int(df.data.m['0']['n']))
        band1 = (int(df.data.m['1']['n']))
        band2 = (int(df.data.m['2']['n']))
        band3 = (int(df.data.m['3']['n']))
        datetime = df.published_at.s
        sId = df.coreid.s
        whistles.append(w)
        dt.append(datetime)
        stationId.append(sId)
        b0.append(band0)
        b1.append(band1)
        b2.append(band2)
        b3.append(band3)
    except:
        print(row)

df2 = pd.DataFrame({'dt': dt, 'w': whistles, 'stationId': stationId,
    'b0': b0, 'b1': b1, 'b2':b2, 'b3':b3})
print(len(df2))

df2.to_csv('/w/loggerhead/AMS/python/ooma2.csv')

