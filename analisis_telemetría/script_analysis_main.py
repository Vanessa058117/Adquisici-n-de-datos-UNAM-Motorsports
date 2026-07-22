import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import butter, filtfilt

df = pd.read_csv("Vanessa Sánchez - data.csv")

# Parámetros
fs = 100
fc = 5
orden = 4

b, a = butter(orden, fc / (0.5 * fs), btype='low', analog=False)

df['accel_x_filt'] = filtfilt(b, a, df['accel_x'])
df['accel_y_filt'] = filtfilt(b, a, df['accel_y'])

df['G_mag_raw'] = np.sqrt(df['accel_x']**2 + df['accel_y']**2)
df['G_mag_filt'] = np.sqrt(df['accel_x_filt']**2 + df['accel_y_filt']**2)

print(f"Pico G (Señal Cruda): {df['G_mag_raw'].max():.2f} G")
print(f"Pico G (Señal Filtrada): {df['G_mag_filt'].max():.2f} G");plt.figure(figsize=(8, 8))

plt.scatter(df['accel_y_filt'], df['accel_x_filt'], alpha=0.5, s=2, color='royalblue')

plt.title("Diagrama G-G (Fórmula SAE)", fontsize=14, fontweight='bold')
plt.xlabel("Fuerza G Lateral ($a_y$)", fontsize=12)
plt.ylabel("Fuerza G Longitudinal ($a_x$)", fontsize=12)
plt.axhline(0, color='black', linewidth=1.2)
plt.axvline(0, color='black', linewidth=1.2)
plt.grid(True, linestyle='--', alpha=0.7)
plt.axis('equal')
plt.show()

falla_inercial = df[df['accel_x'] == 0.0]

t_ini_falla = falla_inercial['Timestamp'].iloc[0]
t_fin_falla = falla_inercial['Timestamp'].iloc[-1]

print(f"ALERTA: Sensor inercial caído entre los segundos {t_ini_falla} y {t_fin_falla}.")

df['plausibility_error'] = (df['brake_pressure'] > 90) & (df['throttle'] > 90)

df['bloque'] = (df['plausibility_error'] != df['plausibility_error'].shift()).cumsum()

bloques_error = df[df['plausibility_error']].groupby('bloque')

for nombre, bloque in bloques_error:
    if len(bloque) > 10:
        t_ini_pps = bloque['Timestamp'].iloc[0]
        t_fin_pps = bloque['Timestamp'].iloc[-1]
        print(f"VIOLACIÓN DE PLAUSIBILIDAD! Evento detectado desde {t_ini_pps}s a {t_fin_pps}s")

