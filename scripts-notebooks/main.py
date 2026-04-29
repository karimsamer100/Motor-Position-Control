import pandas as pd

df = pd.read_csv('PulseResponse.csv')
print(df.head())
df['Volt'] = df['Volt'].replace(5, 12)
df.to_csv('PulseResponse.csv', index=False)
