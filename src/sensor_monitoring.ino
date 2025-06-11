#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>

// Definições dos pinos
#define DHT_PIN 23        // Pino do DHT22
#define DHT_TYPE DHT22    // Tipo do sensor DHT
#define LDR_PIN 34        // Pino analógico do LDR
#define SDA_PIN 21        // Pino SDA do I2C
#define SCL_PIN 22        // Pino SCL do I2C

// Inicialização dos sensores
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_MPU6050 mpu;

// Variáveis para armazenar dados
float temperatura, umidade, luminosidade;
int16_t ax, ay, az, gx, gy, gz;
unsigned long ultimaLeitura = 0;
const unsigned long intervaloLeitura = 2000; // 2 segundos

void setup() {
  Serial.begin(115200);
  Serial.println("=== Sistema de Monitoramento Industrial ===");
  Serial.println("Inicializando sensores...");
  
  // Inicializar DHT22
  dht.begin();
  Serial.println("DHT22 inicializado");
  
  // Inicializar I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Inicializar MPU6050
  if (mpu.begin()) {
    Serial.println("MPU6050 inicializado com sucesso");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  } else {
    Serial.println("Erro ao inicializar MPU6050");
  }
  
  // Configurar pino do LDR
  pinMode(LDR_PIN, INPUT);
  
  Serial.println("Sistema pronto para coleta de dados!");
  Serial.println("Timestamp,Temperatura(C),Umidade(%),Luminosidade,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z");
}

void loop() {
  unsigned long tempoAtual = millis();
  
  // Realizar leitura a cada intervalo definido
  if (tempoAtual - ultimaLeitura >= intervaloLeitura) {
    coletarDados();
    exibirDados();
    ultimaLeitura = tempoAtual;
  }
  
  delay(100); // Pequeno delay para evitar sobrecarga
}

void coletarDados() {
  // Leitura do DHT22
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
  
  // Verificar se as leituras são válidas
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro na leitura do DHT22");
    temperatura = 0;
    umidade = 0;
  }
  
  // Leitura do LDR (fotoresistor)
  int valorLDR = analogRead(LDR_PIN);
  luminosidade = map(valorLDR, 0, 4095, 0, 100); // Converter para porcentagem
  
  // Leitura do MPU6050
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Converter para valores inteiros (mg para acelerômetro)
  ax = a.acceleration.x * 1000;
  ay = a.acceleration.y * 1000; 
  az = a.acceleration.z * 1000;
  gx = g.gyro.x * 1000;
  gy = g.gyro.y * 1000;
  gz = g.gyro.z * 1000;
}

void exibirDados() {
  // Timestamp em segundos
  unsigned long timestamp = millis() / 1000;
  
  // Exibir dados formatados para análise
  Serial.print(timestamp);
  Serial.print(",");
  Serial.print(temperatura, 2);
  Serial.print(",");
  Serial.print(umidade, 2);
  Serial.print(",");
  Serial.print(luminosidade, 2);
  Serial.print(",");
  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.print(az);
  Serial.print(",");
  Serial.print(gx);
  Serial.print(",");
  Serial.print(gy);
  Serial.print(",");
  Serial.println(gz);
  
  // Exibir dados legíveis no monitor
  Serial.println("--- Leitura dos Sensores ---");
  Serial.println("Ambiente:");
  Serial.printf("  Temperatura: %.2f°C\n", temperatura);
  Serial.printf("  Umidade: %.2f%%\n", umidade);
  Serial.printf("  Luminosidade: %.2f%%\n", luminosidade);
  
  Serial.println("Vibração/Movimento (MPU6050):");
  Serial.printf("  Aceleração (mg): X=%d, Y=%d, Z=%d\n", ax, ay, az);
  Serial.printf("  Giroscópio (mdps): X=%d, Y=%d, Z=%d\n", gx, gy, gz);
  
  // Análise básica para detecção de anomalias
  analisarDados();
  Serial.println("================================");
}

void analisarDados() {
  // Alertas simples baseados em thresholds industriais
  if (temperatura > 35.0) {
    Serial.println("⚠️  ALERTA: Temperatura elevada!");
  }
  
  if (umidade > 80.0) {
    Serial.println("⚠️  ALERTA: Umidade muito alta!");
  }
  
  if (luminosidade < 10.0) {
    Serial.println("⚠️  ALERTA: Luminosidade muito baixa!");
  }
  
  // Detecção de vibração excessiva
  float vibracao = sqrt(ax*ax + ay*ay + az*az);
  if (vibracao > 2000) { // Threshold em mg
    Serial.println("⚠️  ALERTA: Vibração excessiva detectada!");
  }
}

// Função auxiliar para simular variações realísticas (opcional)
float adicionarRuido(float valor, float percentual) {
  float ruido = (random(-100, 101) / 100.0) * percentual;
  return valor * (1 + ruido/100);
}