# Parcial II - Microcontroladores I

Tiempo estimado: 2 h

Un agricultor desea monitorear la temperatura (25 °C–50 °C) de su invernadero mediante un sistema programado en Arduino IDE y el microcontrolador ESP32, utilizando 2 botones externos y la consola Serial. El sistema debe incluir 3 LEDs indicadores (Verde, Amarillo y Rojo). La temperatura es leída con un sensor LM35 y se toman decisiones en función de su valor.

## Requisitos del programa

### 1. Lectura de temperatura (LM35)

- El LM35 entrega **10 mV/°C**. Para 25–50 °C ⇒ 0.25–0.50 V.
- Diseñar el circuito de acondicionamiento si lo consideran necesario (op-amp para ampliar rango) o, sin op-amp, usar **ADC1**, resolución **12 bits**.
- Documentar la relación **cuentas ADC → voltios → °C**.

### 2. Validación de datos por Serial

- Permitir **simulación** de la lectura con comandos por Serial:
  - T=<valor> (°C, T=32.5)
  - B1=<0|1> y B2=<0|1> para simular botones
- Indicar en consola si se usa **sensor real o simulación**.

### 3. Evaluación de temperatura (umbrales incluyentes)

- T ≤ 25 °C → "Temperatura baja. Activando calefacción." → Encender **LED Verde** y apagar los otros LEDs.
- 26 °C ≤ T ≤ 49 °C → "Temperatura óptima. Sistema en espera." → Encender **LED Amarillo** y apagar los otros LEDs.
- T ≥ 50 °C → "Temperatura alta. Desactivando calefacción." → Encender **LED Rojo** y apagar los otros LEDs.

### 4. Cálculo de promedio (Botón 1)

- Al presionar **B1 (Pull-Down)**, se deben registrar temperaturas. Cada 5 mediciones consecutivas mostrar el **promedio (°C)** y reiniciar la ventana.

- Implementar **antirrebote** por código.

### 5. Finalización del sistema (Botón 2 + DAC)

- Al presionar **B2 (Pull-Down)**, mostrar el **promedio total** de temperaturas registradas y luego finalizar ordenadamente.

- Generar una **señal de aviso por DAC**. Si el buzzer es pasivo, se acepta **PWM usando el DAC**.


## Entregables:

- **Estructuración del problema** (estrategia de análisis).

- **Diagrama de conexión eléctrica** (LM35, botones con pull-Down, LEDs, DAC/buzzer) y **diagrama de flujo del algoritmo**.

- **Código fuente** (Arduino) con comentarios.

- **README** con pinout, modo de prueba (sensor/simulado) y comandos Serial.

- **Git**: commits por cada función/unidad lógica (ADC, antirrebote, umbrales, promedio, finalización/DAC, etc.).