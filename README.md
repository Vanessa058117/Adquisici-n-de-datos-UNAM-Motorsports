# Adquisicion de datos UNAM Motorsports
Sistemas de seguridad para el pedal de aceleración siguiendo la Regla T.4.2 de FSAE (Accelerator Pedal Position Sensor - APPS)  y diagnostico de telemetría y detección de anomalías.

## Proyecto de Telemetría y Análisis Dinámico - Fórmula SAE (DAQ)

Este repositorio contiene los scripts desarrollados para el sistema de adquisición de datos (DAQ) y telemetría de un vehículo de Fórmula SAE. El objetivo principal es visualizar, filtrar y analizar datos dinámicos del monoplaza, con especial enfoque en la validación de la regla de seguridad T.4.2 (Plausibilidad APPS/Freno) y el análisis de aceleración.


## Requisitos

Para ejecutar los scripts, se necesita tener instalado Python y las siguientes librerías:
```
pip install streamlit pandas plotly numpy matplotlib scipy
```
---

##  Descripción de los proyectos

Se hicieron 2: El Sistema de Validación de Plausibilidad APPS y el Diagnóstico de Telemetría y Detección de
Anomalías.

### I. Diagnóstico de Telemetría y Detección de Anomalías.
**Archivo** `script_analysis_main.py`

Este script se ejecuta por consola (o Jupyter Notebook) y está enfocado en la limpieza de datos de sensores y la extracción de métricas dinámicas (Fuerzas G) y fallas estructurales o de seguridad.

**Explicación por partes:**
1. Filtrado de Ruido (Filtro Butterworth): Los sensores inerciales en un monoplaza captan mucha vibración del chasis y motor. El código utiliza `scipy.signal` para crear un filtro paso bajo (frecuencia de corte 5 Hz, orden 4) para limpiar las aceleraciones laterales (`accel_y`) y longitudinales (`accel_x`).
2. Cálculo de Fuerzas G: Emplea el Teorema de Pitágoras con `numpy` (`np.sqrt(x^2 + y^2)`) para sacar la magnitud combinada (G Mag) de la señal cruda y de la señal filtrada para comparar los picos de Fuerza G.
3. Diagrama G-G: Genera un gráfico de dispersión (Scatter plot) usando `matplotlib`.
4. Detección de Caída del Sensor: Realiza una búsqueda eficiente de ceros absolutos (`accel_x == 0.0`), que en la realidad suele indicar una pérdida de conexión, caída de alimentación del sensor o saturación del bus de datos, imprimiendo el momento exacto en que falló.
5. Detección Aislada de Plausibilidad: Este bloque agrupa los errores (con `.shift().cumsum()`) para aislar los "eventos" (bloques donde el error ocurre sin interrupción). Si el bloque de error dura más de 10 registros de tiempo consecutivos, lanza una alerta por consola advirtiendo entre qué segundos ocurrió la falla de plausibilidad de los pedales.

### II. Dashboard Interactivo de Telemetría (Streamlit)
**Archivo** `app.py`
Este script crea una aplicación web interactiva que permite a los ingenieros inspeccionar el comportamiento de los pedales en ventanas de tiempo específicas.

### Configuración Inicial
1. Caché: Se configura la página de Streamlit y se utiliza `@st.cache_data` para cargar el CSV una sola vez, optimizando el rendimiento.
2. Validación de plausibilidad: La regla de Fórmula SAE dicta que si el piloto acelera (Throttle > 90%) y frena (Brake > 90%) simultáneamente durante un tiempo prolongado (en este caso validado por una ventana rodante de 10 muestras), el sistema debe cortar la potencia. El código crea la columna `plausibility_error` que marca estas infracciones.
3. Navegación en el Tiempo: Un control deslizante (`st.slider`) permite navegar a lo largo de toda la duración de la telemetría.
4. Ventana Dinámica: Para no saturar el gráfico, el código recorta un segmento temporal de 3 segundos (`t_actual` a `t_actual + 3.0`) para analizar a detalle.
5. Alertas Visuales: Mediante `st.error` y `st.success`, el dashboard advierte inmediatamente en pantalla si en la ventana actual ocurre una violación de plausibilidad.
6. Gráfica Plotly: Construye una gráfica interactiva (zoom, hover) que cruza el porcentaje del acelerador y la presión de freno a lo largo del tiempo, facilitando la identificación visual de la anomalía.
---

## Comandos

1. Para correr el análisis y obtener el diagrama G-G:
   En la terminal se ejecuta:
   ```bash
   python analisis_dinamico.py
   ```

2. Para lanzar el Dashboard interactivo:
   Se ejecuta este comando en una terminal:
   ```bash
   streamlit run dashboard_telemetria.py
   ```
