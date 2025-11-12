// Simulación de monitoreo de temperatura en ESP32


const int LED_VERDE = 25;
const int LED_AMARILLO = 26;
const int LED_ROJO = 27;
const int BOTON1 = 14;
const int BOTON2 = 12;

// --- Variables globales ---
float temperatura = 0;
int boton1_estado = HIGH;
int boton2_estado = HIGH;

void setup() {
  Serial.begin(115200);
  Serial.println("=== SIMULACION SISTEMA DE TEMPERATURA - INVERNADERO ===");
  Serial.println("Presione Botón 1 para mostrar temperatura");
  Serial.println("Presione Botón 2 para reiniciar el sistema\n");
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
