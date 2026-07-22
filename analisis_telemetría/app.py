import streamlit as st
import pandas as pd
import plotly.graph_objects as go

st.set_page_config(page_title="Telemetría SAE - DAQ", layout="wide")
st.title("Dashboard de Telemetría APPS (Fórmula SAE)")

@st.cache_data
def load_data():
    df = pd.read_csv("Vanessa Sánchez - data.csv")
    condicion_bruta = (df['brake_pressure'] > 90) & (df['throttle'] > 90)
    # 2. Regla T.4.2
    df['plausibility_error'] = condicion_bruta.rolling(window=10).sum() >= 10
    return df
df = load_data()

st.subheader("Análisis Dinámico en el Tiempo")
t_min = float(df['Timestamp'].min())
t_max = float(df['Timestamp'].max())

t_actual = st.slider("Recorrer telemetría (Segundos):", min_value=t_min, max_value=t_max, value=t_min, step=0.1)

ventana = df[(df['Timestamp'] >= t_actual) & (df['Timestamp'] <= t_actual + 3.0)]

falla_activa = ventana['plausibility_error'].iloc[0] if not ventana.empty else False

if falla_activa:
    st.error("VIOLACIÓN DE PLAUSIBILIDAD APPS DETECTADA.",)
else:
    st.success("Sistemas nominales. Sin anomalías de seguridad.")

fig = go.Figure()

# Acelerador
fig.add_trace(go.Scatter(x=ventana['Timestamp'], y=ventana['throttle'], 
                         mode='lines', name='Acelerador (%)', line=dict(color='#2ca02c', width=3)))

# Freno
fig.add_trace(go.Scatter(x=ventana['Timestamp'], y=ventana['brake_pressure'], 
                         mode='lines', name='Presión de Freno (%)', line=dict(color='#d62728', width=3)))

fig.update_layout(title="Comportamiento Dinámico de los Pedales", 
                  xaxis_title="Tiempo (s)", 
                  yaxis_title="Apertura / Presión (%)", 
                  yaxis_range=[0, 110],
                  hovermode="x unified")

st.plotly_chart(fig, use_container_width=True)