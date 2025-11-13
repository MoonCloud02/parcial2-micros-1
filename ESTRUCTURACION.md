# Estructuración del Problema - Sistema de Monitoreo de Temperatura

## 1. Análisis del Problema

### 1.1 Descripción General
Se requiere desarrollar un sistema de monitoreo de temperatura para un invernadero utilizando ESP32, que permita:
- Leer temperatura ambiente (25°C - 50°C) mediante sensor LM35
- Controlar 3 LEDs indicadores según umbrales de temperatura
- Registrar y calcular promedios de mediciones mediante botones
- Generar señales de aviso al finalizar el sistema
- Permitir simulación por comandos seriales

### 1.2 Requerimientos Funcionales

**RF1: Adquisición de Datos**
- Leer temperatura del sensor LM35 (10mV/°C)
- Rango de operación: 25°C - 50°C (0.25V - 0.50V)
- Conversión ADC de 12 bits (0-4095 cuentas)
- Frecuencia de muestreo: 1 Hz (cada segundo)

**RF2: Control de Indicadores LED**
- LED Verde: T ≤ 25°C (calefacción activa)
- LED Amarillo: 26°C ≤ T ≤ 49°C (temperatura óptima)
- LED Rojo: T ≥ 50°C (desactivar calefacción)
- Solo un LED encendido a la vez

**RF3: Registro y Promedio (Botón 1)**
- Tomar 5 muestras consecutivas al presionar
- Intervalo de 500ms entre muestras
- Calcular y mostrar promedio de las 5 mediciones
- Implementar antirrebote por software (50ms)

**RF4: Finalización del Sistema (Botón 2)**
- Calcular promedio total de todas las mediciones
- Mostrar cantidad total de mediciones registradas
- Generar señal de aviso por DAC (3 pulsos de 200ms)
- Apagar todos los LEDs
- Detener el sistema completamente

**RF5: Simulación por Serial**
- Comando `T=<valor>`: Simular temperatura
- Comando `B1=1`: Simular presión de Botón 1
- Comando `B2=1`: Simular presión de Botón 2
- Velocidad: 115200 baudios

### 1.3 Requerimientos No Funcionales

**RNF1: Hardware**
- Microcontrolador: ESP32 Dev Module
- Sensor: LM35 (sin acondicionamiento adicional)
- Voltaje de referencia: 3.3V
- Resolución ADC: 12 bits

**RNF2: Software**
- Entorno: Arduino IDE
- Lenguaje: C/C++ (Arduino)
- Comunicación: Serial UART a 115200 bps

**RNF3: Usabilidad**
- Mensajes claros en monitor serial
- Visualización en tiempo real
- Modo debug para simulación

## 2. Estrategia de Análisis

### 2.1 Descomposición del Sistema

El sistema se divide en los siguientes módulos funcionales:

```
┌─────────────────────────────────────────────┐
│         SISTEMA DE MONITOREO                │
└─────────────────────────────────────────────┘
              ↓
    ┌─────────┴─────────┐
    │                   │
┌───▼────┐      ┌───────▼──────┐
│ ENTRADA│      │    PROCESO   │
└───┬────┘      └───────┬──────┘
    │                   │
    ├─ LM35             ├─ Conversión ADC
    ├─ Botón 1          ├─ Evaluación umbrales
    ├─ Botón 2          ├─ Cálculo promedios
    └─ Serial           └─ Control de estado
                              │
                    ┌─────────┴─────────┐
                    │                   │
                ┌───▼────┐      ┌──────▼────┐
                │ SALIDA │      │  ALMAC.   │
                └───┬────┘      └──────┬────┘
                    │                  │
                    ├─ LEDs            ├─ sumaTemperaturas
                    ├─ Serial          ├─ contadorTotal
                    └─ DAC/Buzzer      └─ ventanaTemps[]
```

### 2.2 Análisis por Módulos

#### Módulo 1: Adquisición de Temperatura (LM35)

**Análisis del Sensor:**
- LM35 genera 10mV/°C
- Rango: 25°C - 50°C
- Voltaje de salida: 0.25V - 0.50V

**Análisis del ADC:**
- ADC1_CH6 (GPIO 34)
- 12 bits: 0 - 4095 cuentas
- Vref = 3.3V
- Atenuación: 11dB (rango completo 0-3.3V)

**Conversión matemática:**
```
Voltios = (cuentasADC / 4095) × 3.3V
Temperatura (°C) = Voltios / 0.01
```

**Ejemplo de cálculo:**
- 25°C → 0.25V → (0.25/3.3)×4095 ≈ 310 cuentas
- 50°C → 0.50V → (0.50/3.3)×4095 ≈ 620 cuentas

**Decisión de diseño:** 
No se requiere acondicionamiento adicional (amplificador operacional) ya que el rango de 0.25V-0.50V es detectable por el ADC del ESP32 con suficiente precisión.

#### Módulo 2: Control de LEDs (Umbrales)

**Análisis de umbrales:**
```
if (T ≤ 25)     → LED_VERDE   (GPIO 25)
if (26 ≤ T ≤ 49) → LED_AMARILLO (GPIO 26)
if (T ≥ 50)     → LED_ROJO    (GPIO 27)
```

**Máquina de estados de LEDs:**
```
Estado 1: TEMPERATURA_BAJA
  - LED Verde: ON
  - LED Amarillo: OFF
  - LED Rojo: OFF
  
Estado 2: TEMPERATURA_OPTIMA
  - LED Verde: OFF
  - LED Amarillo: ON
  - LED Rojo: OFF
  
Estado 3: TEMPERATURA_ALTA
  - LED Verde: OFF
  - LED Amarillo: OFF
  - LED Rojo: ON
```

**Decisión de diseño:**
Estructura condicional if-else para evaluar umbrales inclusivos, asegurando que solo un LED esté activo a la vez.

#### Módulo 3: Gestión de Botón 1 (Promedio de 5 Muestras)

**Análisis del requerimiento:**
- Entrada: Presión de botón con pull-down
- Acción: Tomar 5 muestras consecutivas
- Salida: Promedio de las 5 mediciones

**Estrategia de implementación:**
1. Detección de flanco ascendente (LOW → HIGH)
2. Antirrebote por software (50ms)
3. Bucle de 5 iteraciones:
   - Leer temperatura
   - Almacenar en array
   - Mostrar muestra individual
   - Delay 500ms entre muestras (excepto la última)
4. Calcular promedio aritmético
5. Mostrar resultado

**Algoritmo:**
```
Al presionar B1:
  promedio = 0
  Para i = 0 hasta 4:
    medicion[i] = leerTemperatura()
    mostrar(medicion[i])
    promedio += medicion[i]
    si i < 4:
      esperar(500ms)
  promedio = promedio / 5
  mostrar(promedio)
```

**Decisión de diseño:**
Tomar las 5 muestras en una sola presión del botón (en lugar de requerir 5 presiones), proporcionando mejor usabilidad y mediciones más coherentes temporalmente.

#### Módulo 4: Gestión de Botón 2 (Finalización)

**Análisis del requerimiento:**
- Entrada: Presión de botón con pull-down
- Procesamiento: Calcular promedio total
- Salida: Señal de aviso (DAC) + Apagar sistema

**Estrategia de implementación:**
1. Calcular: `promedioTotal = sumaTemperaturas / contadorTotal`
2. Mostrar en Serial con cantidad de mediciones
3. Generar 3 pulsos en DAC:
   - ON: dacWrite(255) por 200ms
   - OFF: dacWrite(0) por 200ms
   - Repetir 3 veces
4. Apagar todos los LEDs
5. Establecer `sistemaActivo = false`

**Decisión de diseño:**
Usar GPIO 26 (DAC2) para el buzzer, compartido con LED_AMARILLO ya que no hay conflicto temporal (el buzzer solo se activa al finalizar el sistema cuando todos los LEDs se apagan).

#### Módulo 5: Registro Continuo de Mediciones

**Análisis del problema:**
El Botón 2 debe mostrar el promedio de **todas** las mediciones, no solo las tomadas por el Botón 1.

**Estrategia de implementación:**
- Registrar cada lectura del loop principal
- Variables globales:
  - `sumaTemperaturas`: Acumulador de todas las temperaturas
  - `contadorTotal`: Cantidad total de mediciones
- Actualización en cada iteración del loop:
  ```cpp
  temperatura = leerTemperatura();
  sumaTemperaturas += temperatura;
  contadorTotal++;
  ```

**Decisión de diseño:**
Mantener registro continuo en el loop principal para capturar todas las lecturas (1 por segundo), más las lecturas adicionales del Botón 1 (5 muestras cada vez).

#### Módulo 6: Comunicación Serial (Simulación)

**Análisis de comandos:**
```
T=<valor>  → Activa modo simulación
          → Establece temperatura fija
          
B1=1       → Simula presión de Botón 1
          → Ejecuta manejarBoton1()
          
B2=1       → Simula presión de Botón 2
          → Ejecuta manejarBoton2()
```

**Estrategia de parsing:**
1. Verificar disponibilidad: `Serial.available()`
2. Leer hasta nueva línea: `readStringUntil('\n')`
3. Eliminar espacios: `trim()`
4. Identificar comando: `startsWith()` o comparación exacta
5. Extraer valor: `substring()`
6. Ejecutar acción correspondiente

**Decisión de diseño:**
Variable global `modoSimulacion` para alternar entre lectura real del sensor y valor simulado, permitiendo pruebas sin hardware.

### 2.3 Análisis de Variables de Estado

**Variables de Entrada:**
- `temperatura`: float - Temperatura actual (°C)
- `boton1Anterior`: int - Estado previo Botón 1
- `boton2Anterior`: int - Estado previo Botón 2
- `comandoSerial`: String - Comando recibido

**Variables de Proceso:**
- `modoSimulacion`: bool - Modo de operación
- `sistemaActivo`: bool - Estado del sistema
- `ventanaTemps[5]`: float[] - Buffer de 5 muestras

**Variables de Acumulación:**
- `sumaTemperaturas`: float - Suma total
- `contadorTotal`: int - Cantidad de mediciones

**Variables de Temporización:**
- `tiempoDebounce1`: unsigned long - Timer antirrebote B1
- `tiempoDebounce2`: unsigned long - Timer antirrebote B2
- `DEBOUNCE_DELAY`: const 50ms

### 2.4 Diagrama de Estados del Sistema

```
┌─────────────┐
│   INICIO    │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   SETUP     │
│ - Config IO │
│ - Init ADC  │
│ - Init UART │
└──────┬──────┘
       │
       ▼
┌─────────────────────┐
│  SISTEMA_ACTIVO     │◄────────┐
│  Loop Principal:    │         │
│  1. Leer Temp       │         │
│  2. Registrar       │         │
│  3. Control LEDs    │         │
│  4. Check Botones   │         │
└──┬──────────────┬───┘         │
   │              │             │
   │ B1 Presionado│             │
   ▼              │             │
┌─────────────┐   │             │
│ MUESTREO_5  │   │             │
│ - 5 lecturas│   │             │
│ - Promedio  │───┘             │
└─────────────┘                 │
                                │
              B2 Presionado     │
              ▼                 │
         ┌─────────────┐        │
         │ FINALIZACION│        │
         │ - Prom Total│        │
         │ - Aviso DAC │        │
         │ - Apagar    │        │
         └──────┬──────┘        │
                │               │
                ▼               │
         ┌─────────────┐        │
         │SISTEMA_INAC │────────┘
         │ (Bloqueado) │
         └─────────────┘
```

### 2.5 Análisis de Temporización

**Eventos periódicos:**
- Loop principal: 1000ms (1 Hz)
- Muestras B1: 500ms entre cada una
- Pulsos DAC: 200ms ON + 200ms OFF

**Eventos asíncronos:**
- Comandos Serial: Cualquier momento
- Presión de botones: Detección por polling cada loop

**Tiempos críticos:**
- Antirrebote: 50ms (mínimo entre detecciones)
- Muestreo B1: 2.5s total (5 × 500ms + procesamiento)

### 2.6 Manejo de Errores y Casos Especiales

**Caso 1: Temperatura fuera de rango**
- Solución: Aceptar cualquier valor, los umbrales se ajustan automáticamente

**Caso 2: Presión simultánea de botones**
- Solución: Prioridad a B1, B2 se evalúa después en el mismo loop

**Caso 3: Comando serial inválido**
- Solución: Ignorar silenciosamente, continuar operación normal

**Caso 4: División por cero en promedio**
- Solución: Verificar `contadorTotal > 0` antes de calcular

**Caso 5: Sistema inactivo**
- Solución: Return temprano en loop, evitando ejecución de código

## 3. Asignación de Recursos

### 3.1 Pines GPIO ESP32

| Pin | Función | Componente | Configuración |
|-----|---------|------------|---------------|
| GPIO 25 | Salida Digital | LED Verde | OUTPUT |
| GPIO 26 | Salida Digital + DAC | LED Amarillo / Buzzer | OUTPUT + DAC2 |
| GPIO 27 | Salida Digital | LED Rojo | OUTPUT |
| GPIO 14 | Entrada Digital | Botón 1 | INPUT Pull-Down |
| GPIO 12 | Entrada Digital | Botón 2 | INPUT Pull-Down |
| GPIO 34 | Entrada Analógica | LM35 | ADC1_CH6 |

### 3.2 Memoria

**Variables globales:** ~60 bytes
- floats: 28 bytes (7 × 4 bytes)
- ints: 16 bytes (4 × 4 bytes)
- unsigned long: 8 bytes (2 × 4 bytes)
- bools: 2 bytes (2 × 1 byte)

**Strings:** Dinámico (comandos serial)

**Stack:** Funciones simples sin recursión

### 3.3 Periféricos

- **ADC1:** Canal 6, 12 bits, atenuación 11dB
- **UART0:** 115200 bps, 8N1
- **DAC:** Canal 2 (GPIO 26), 8 bits (0-255)
- **GPIO:** 8 pines configurados

## 4. Validación de la Estrategia

### 4.1 Cumplimiento de Requisitos

✅ **RF1:** ADC configurado a 12 bits, conversión documentada
✅ **RF2:** Control de LEDs con lógica de umbrales inclusivos
✅ **RF3:** Botón 1 toma 5 muestras y calcula promedio
✅ **RF4:** Botón 2 muestra promedio total y finaliza con señal DAC
✅ **RF5:** Comandos serial T=, B1=, B2= implementados

### 4.2 Pruebas Contempladas

**Prueba 1: Umbrales de temperatura**
- T=20 → LED Verde
- T=30 → LED Amarillo
- T=55 → LED Rojo

**Prueba 2: Muestreo Botón 1**
- Enviar B1=1
- Verificar 5 muestras mostradas
- Verificar promedio calculado

**Prueba 3: Promedio total**
- Dejar correr sistema N segundos
- Enviar B1=1 M veces
- Enviar B2=1
- Verificar: contadorTotal = N + (M × 5)

**Prueba 4: Finalización**
- Enviar B2=1
- Verificar promedio total
- Verificar LEDs apagados
- Verificar sistema detenido

### 4.3 Escalabilidad y Mantenibilidad

**Modularidad:** Código dividido en funciones específicas
- `setup()`: Inicialización
- `loop()`: Ciclo principal
- `leerTemperatura()`: Adquisición
- `controlarLEDs()`: Salida visual
- `procesarComandoSerial()`: Interfaz usuario
- `manejarBoton1()`: Muestreo
- `manejarBoton2()`: Finalización

**Extensibilidad:**
- Fácil agregar más comandos serial
- Fácil modificar umbrales
- Fácil cambiar cantidad de muestras

**Documentación:**
- Comentarios en código
- README con instrucciones
- Diagramas de flujo y conexión

## 5. Conclusiones de la Estrategia

1. **Enfoque modular** permite desarrollo y prueba independiente de cada función
2. **Registro continuo** en loop garantiza que todas las mediciones se contabilicen
3. **Modo simulación** facilita pruebas sin hardware físico
4. **Antirrebote por software** evita lecturas erróneas de botones
5. **Uso eficiente de recursos** sin requerir componentes adicionales (op-amp)
6. **Interfaz serial** proporciona debugging y control flexible
7. **Estados claros** del sistema facilitan comprensión del flujo

La estrategia propuesta cumple con todos los requerimientos del parcial y proporciona una solución robusta, mantenible y extensible para el monitoreo de temperatura en invernadero.
