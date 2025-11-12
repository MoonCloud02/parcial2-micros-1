// Simulación de monitoreo de temperatura - Invernadero ESP32

const int LED_VERDE = 25;
const int LED_AMARILLO = 26;
const int LED_ROJO = 27;
const int BOTON1 = 14;
const int BOTON2 = 12;
const int LM35_PIN = 34;
const int DAC_PIN = 25;

// --- Variables globales ---
float temperatura = 0;
bool modoSimulacion = false;

float sumaTemperaturas = 0;
int contadorTotal = 0;
int contadorVentana = 0;
float ventanaTemps[5];

int boton1_estado = HIGH;
int boton2_estado = HIGH;
unsigned long tiempoDebounce1 = 0;
unsigned long tiempoDebounce2 = 0;
const unsigned long DEBOUNCE_DELAY = 50;

bool sistemaActivo = true;
  
void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BOTON1, INPUT);
  pinMode(BOTON2, INPUT);
  pinMode(LM35_PIN, INPUT);
  
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  Serial.begin(115200);
  Serial.println("=== SISTEMA DE TEMPERATURA - INVERNADERO ===");
  Serial.println("Comandos Serial:");
  Serial.println("  T=<valor> - Simular temperatura (ej: T=32.5)");
  Serial.println("  B1=1 - Simular presion Boton 1");
  Serial.println("  B2=1 - Simular presion Boton 2");
  Serial.println("Modo: Sensor real (envia T=<valor> para simular)\n");

}

void loop() {
  // --- Simulación del LM35 ---
  // En lugar de leer el ADC, generamos una temperatura aleatoria entre 25 y 50 °C
  temperatura = random(250, 500) / 10.0;  // genera un valor entre 25.0 y 50.0
  delay(1000);

  // --- Decisión según temperatura ---
  if (temperatura < 30) {
    Serial.println("LED Verde ENCENDIDO | Amarillo y Rojo APAGADOS");
  } 
  else if (temperatura >= 30 && temperatura <= 40) {
    Serial.println("LED Amarillo ENCENDIDO | Verde y Rojo APAGADOS");
  } 
  else {
    Serial.println("LED Rojo ENCENDIDO | Verde y Amarillo APAGADOS");
  }

  // --- Simulación de Botones ---
  // (Para simular en serial, se pueden enviar comandos manualmente desde el monitor)
  if (Serial.available() > 0) {
    char comando = Serial.read();

    if (comando == '1') {  // Simula Botón 1
      Serial.print("Temperatura actual: ");
      Serial.print(temperatura);
      Serial.println(" °C");
    }

    if (comando == '2') {  // Simula Botón 2
      Serial.println("Sistema reiniciado (todos los LEDs apagados)");
    }
  }

  Serial.println("--------------------------------------");
}


float leerTemperatura() {
  if (modoSimulacion) {
    return temperatura;
  }
  
  int cuentasADC = analogRead(LM35_PIN);
  float voltios = (cuentasADC / 4095.0) * 3.3;
  float tempC = voltios / 0.01;
  return tempC;
}

void controlarLEDs(float temp) {
  if (temp <= 25) {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_ROJO, LOW);
    Serial.println("Temperatura baja. Activando calefaccion.");
  } 
  else if (temp >= 26 && temp <= 49) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, HIGH);
    digitalWrite(LED_ROJO, LOW);
    Serial.println("Temperatura optima. Sistema en espera.");
  } 
  else {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_ROJO, HIGH);
    Serial.println("Temperatura alta. Desactivando calefaccion.");
  }
}

void procesarComandoSerial() {
  String comando = Serial.readStringUntil('\n');
  comando.trim();
  
  if (comando.startsWith("T=")) {
    temperatura = comando.substring(2).toFloat();
    modoSimulacion = true;
    Serial.print("Modo simulacion. T=");
    Serial.print(temperatura);
    Serial.println(" C");
  }
  else if (comando == "B1=1") {
    Serial.println("Boton 1 simulado");
    manejarBoton1();
  }
  else if (comando == "B2=1") {
    Serial.println("Boton 2 simulado");
    manejarBoton2();
  }
}