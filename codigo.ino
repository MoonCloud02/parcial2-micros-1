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
int boton1Anterior = LOW;
int boton2Anterior = LOW;
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
  if (!sistemaActivo) {
    return;
  }
  
  if (Serial.available() > 0) {
    procesarComandoSerial();
  }
  
  temperatura = leerTemperatura();
  
  // Registrar cada temperatura leída para el promedio total
  sumaTemperaturas += temperatura;
  contadorTotal++;
  
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");
  
  controlarLEDs(temperatura);
  
  int estadoB1 = digitalRead(BOTON1);
  if (estadoB1 == HIGH && boton1Anterior == LOW) {
    if (millis() - tiempoDebounce1 > DEBOUNCE_DELAY) {
      manejarBoton1();
      tiempoDebounce1 = millis();
    }
  }
  boton1Anterior = estadoB1;
  
  int estadoB2 = digitalRead(BOTON2);
  if (estadoB2 == HIGH && boton2Anterior == LOW) {
    if (millis() - tiempoDebounce2 > DEBOUNCE_DELAY) {
      manejarBoton2();
      tiempoDebounce2 = millis();
    }
  }
  boton2Anterior = estadoB2;
  
  Serial.println("--------------------------------------");
  delay(1000);
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

void manejarBoton1() {
  Serial.println("Tomando 5 muestras...");
  
  float promedio = 0;
  for (int i = 0; i < 5; i++) {
    float medicion = leerTemperatura();
    ventanaTemps[i] = medicion;
    promedio += medicion;
    
    Serial.print("  Muestra ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(medicion);
    Serial.println(" C");
    
    if (i < 4) {
      delay(500);  // Espera medio segundo entre muestras
    }
  }
  
  promedio /= 5.0;
  
  // Las muestras ya se están registrando en el loop, 
  // aquí solo mostramos el promedio de las 5 mediciones tomadas
  Serial.print("Promedio de 5 mediciones: ");
  Serial.print(promedio);
  Serial.println(" C");
}

void manejarBoton2() {
  if (contadorTotal > 0) {
    float promedioTotal = sumaTemperaturas / contadorTotal;
    Serial.print("Promedio total: ");
    Serial.print(promedioTotal);
    Serial.print(" C (");
    Serial.print(contadorTotal);
    Serial.println(" mediciones)");
  } else {
    Serial.println("No hay mediciones registradas");
  }
  
  Serial.println("Generando señal de aviso...");
  for (int i = 0; i < 3; i++) {
    dacWrite(DAC_PIN, 255);
    delay(200);
    dacWrite(DAC_PIN, 0);
    delay(200);
  }
  
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);
  
  Serial.println("Sistema finalizado");
  sistemaActivo = false;
}