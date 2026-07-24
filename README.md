# Adquisicion de datos UNAM Motorsports
Sistemas de seguridad para el pedal de aceleración siguiendo la Regla T.4.2 de FSAE (Accelerator Pedal Position Sensor - APPS)  y diagnostico de telemetría y detección de anomalías.

## Proyecto de Telemetría y Análisis Dinámico - Fórmula SAE (DAQ)

Este repositorio contiene los scripts desarrollados para el sistema de adquisición de datos (DAQ) y telemetría de un vehículo de Fórmula SAE. El objetivo principal es visualizar, filtrar y analizar datos dinámicos del monoplaza, con especial enfoque en la validación de la regla de seguridad T.4.2 (Plausibilidad APPS/Freno) y el análisis de aceleración.


## Descripción de los proyectos

Se hicieron 2: El Sistema de Validación de Plausibilidad APPS y el Diagnóstico de Telemetría y Detección de
Anomalías.

---

## Requisitos

Para ejecutar los scripts, se necesita tener instalado Python y las siguientes librerías:
```
pip install streamlit pandas plotly numpy matplotlib scipy
```
---

# I. El Sistema de Validación de Plausibilidad APPS.
Este sistema se diseñó para el control a bajo nivel del Sensor de Posición del Pedal del Acelerador (APPS). Su propósito principal es garantizar la seguridad del vehículo leyendo dos sensores redundantes y cortando la potencia del motor si se detecta una anomalía, cumpliendo con la normativa de seguridad de Fórmula SAE.

## Conexiones de Hardware

El microcontrolador espera la siguiente configuración de pines:

   Pin A3: Entrada analógica del Sensor APPS 1.
   
   Pin A2: Entrada analógica del Sensor APPS 2.
   
   Pin 7 (LED): Señal de alerta o corte. Debe ir conectada al relé del circuito de seguridad (Shutdown Circuit) o al controlador del motor.

---

## Parámetros de Calibración
Los valores crudos del convertidor analógico a digital (ADC) están calibrados para los rangos mecánicos específicos de los pedales actuales:

Sensor 1 (A3): 

Resistencia 1 conectada a vcc con valor de 6.6k ohms, la resistencia 2 conectada a gnd coin un valor de 1.1k ohms

Rango nominal al calibrar de 64 a 642.


Sensor 2 (A2): 

Resistencia 1 conectada a vcc con valor de 12k ohms, la resistencia 2 conectada a gnd coin un valor de 2.2k ohms

Rango nominal al calibrar de 93 a 516.

Umbral de Discrepancia: Máximo 10% de diferencia permitida entre ambos sensores.
Ventana de Falla: 100 milisegundos de tolerancia antes de bloquear el motor.

---

<img width="1525" height="553" alt="Screenshot 2026-07-23 060939" src="https://github.com/user-attachments/assets/a018bb16-b5ad-4d27-9106-897fcc32971d" />

https://www.tinkercad.com/things/3qEdyWaBwP8-daq?sharecode=i2WkwMANtqZJuGhwrc2wQNF1PIR_lNVUewsI70TCpMw

---
##  Procedimiento

El flujo del programa (bucle `loop`) realiza las siguientes tareas de seguridad en tiempo real:

1. Detección de Cortocircuito o Desconexión
La función `sensorFueraDeRango` evalúa si la lectura cruda de los sensores cae más de 15 puntos por debajo del mínimo esperado o por encima del máximo. Esto detecta inmediatamente cables rotos, desconexiones o cortocircuitos a tierra/VCC (Short to GND/VCC).

2. Mapeo y Restricción de Señal
Utilizando las funciones `map` y `constrain`, el código transforma los valores eléctricos crudos de cada sensor en un porcentaje de recorrido del pedal (0% a 100%). Se divide entre 10 al final para obtener un decimal de alta precisión en la telemetría.

3. Detección de Discrepancia APPS
El código calcula la diferencia absoluta entre el porcentaje del Sensor 1 y el Sensor 2. Si la diferencia es mayor al 10% (umbral límite definido), se activa el estado de falla inminente. 

4. Temporizador de Ventana de Falla
Para evitar falsos positivos por ruido eléctrico rápido, se guarda el registro de tiempo (`millis()`) cuando inicia la falla. Solo si la discrepancia se mantiene activa y supera los 100 milisegundos de forma ininterrumpida, el sistema toma acción.

5. Bloqueo de Seguridad (Latch)
Si la falla excede la ventana de tiempo estipulada, la variable `motorBloqueado` se vuelve verdadera (`true`) y el Pin 7 se activa (`HIGH`) para abrir el circuito de seguridad. El sistema entra en un estado de bloqueo permanente (latch) donde deja de leer los pedales y requiere un reinicio manual del microcontrolador (apagar y encender el coche) para volver a operar.
---

## Uso y Monitorización Serial

Para visualizar los datos en tiempo real y calibrar los sensores:

1. Se conecta la placa Arduino a tu computadora.
2. Se sube el código.
3. Se abre el *Monitor Serie* configurado a una tasa de *115200 baudios*.

La consola imprimirá un flujo constante de datos con el siguiente formato, útil para depuración o para enviar a la placa de telemetría (DAQ):

> `RAW1: 300 | RAW2: 250 | Canal1: 40.5% | Canal2: 38.2% | Dif: 2.3%`

Si ocurre una falla sostenida, la consola dejará de imprimir telemetría y mostrará repetidamente:
> `MOTOR BLOQUEADO. Reinicio manual requerido.`

# II. Diagnóstico de Telemetría y Detección de Anomalías.
**Archivo** `script_analysis_main.py`

Este script está enfocado en la limpieza de datos de sensores y la extracción de métricas dinámicas (Fuerzas G) y fallas estructurales o de seguridad.

**Procedimiento:**
1. Filtrado de Ruido (Filtro Butterworth): Los sensores inerciales en un monoplaza captan mucha vibración del chasis y motor. El código utiliza `scipy.signal` para crear un filtro paso bajo (frecuencia de corte 5 Hz, orden 4) para limpiar las aceleraciones laterales (`accel_y`) y longitudinales (`accel_x`).
2. Cálculo de Fuerzas G: Emplea el Teorema de Pitágoras con `numpy` (`np.sqrt(x^2 + y^2)`) para sacar la magnitud combinada (G Mag) de la señal cruda y de la señal filtrada para comparar los picos de Fuerza G.
3. Diagrama G-G: Genera un gráfico de dispersión (Scatter plot) usando `matplotlib`.
4. Detección de Caída del Sensor: Realiza una búsqueda eficiente de ceros absolutos (`accel_x == 0.0`), que en la realidad suele indicar una pérdida de conexión, caída de alimentación del sensor o saturación del bus de datos, imprimiendo el momento exacto en que falló.
5. Detección Aislada de Plausibilidad: Este bloque agrupa los errores (con `.shift().cumsum()`) para aislar los "eventos" (bloques donde el error ocurre sin interrupción). Si el bloque de error dura más de 10 registros de tiempo consecutivos, lanza una alerta por consola advirtiendo entre qué segundos ocurrió la falla de plausibilidad de los pedales.

## II.I. Dashboard Interactivo de Telemetría (Streamlit)
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
   python script_analysis_main.py
   ```

2. Para lanzar el Dashboard interactivo:
   Se ejecuta este comando en una terminal:
   ```bash
   streamlit run dashboard_telemetria.py
   ```
