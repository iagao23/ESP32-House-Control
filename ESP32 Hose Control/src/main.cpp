#include <Arduino.h>
#include "driver/i2s.h"
#include <WiFi.h>
#include <HTTPClient.h>
// ================= PINOS I2S =================
#define I2S_SCK 14
#define I2S_WS  15
#define I2S_SD  32
#define I2S_PORT I2S_NUM_0

// ================= AUDIO =====================
#define SAMPLE_RATE     16000
#define FRAME_SIZE      256
#define BUFFER_FRAMES   40   // ~640 ms

#define SPEECH_THRESHOLD 2000

int32_t audioBuffer[FRAME_SIZE * BUFFER_FRAMES];
int bufferIndex = 0;
bool capturing = false;
// ================= HTTP REQUEST================
const char* WIFI_SSID = "SEU-WIFI";
const char* WIFI_PASS = "SUA-SENHA";
const char* lamp_ip = "192.168.1.21";

typedef enum
{
  CMD_NONE,
  CMD_LIGA,
  CMD_DESLIGA
} state;
state state_cmd;

void sendLampCmd(const char* cmd) {
  HTTPClient http;
  String url = "http://" + String(lamp_ip) + "/cm?cmnd=" + cmd;
  http.begin(url);
  http.GET();
  http.end();
}

void lampOn() {
  sendLampCmd("Power%20On");
}

void lampOff() {
  sendLampCmd("Power%20Off");
}
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Conectando ao WiFi");

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeout++;
    if (timeout > 30) {
      Serial.println("\nFalha WiFi");
      return;
    }
  }

  Serial.println("\nWiFi conectado");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

// ================= PROTOTIPOS =================
void setupI2S();
void processCommand();
void classify(uint64_t energy, int zeroCrossings);
void sendLampCmd(const char* cmd);
void lampOn();
void lampOff();

// ================= SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Voice Control - Inicializando");

  setupI2S();
  Serial.println("I2S pronto");

  connectWiFi();

  state_cmd = CMD_NONE;
}

// ================= LOOP ======================
void loop() {
  int32_t samples[FRAME_SIZE];
  size_t bytesRead;

  i2s_read(I2S_PORT, samples, sizeof(samples), &bytesRead, portMAX_DELAY);

  uint64_t sum = 0;
  for (int i = 0; i < FRAME_SIZE; i++) {
    int32_t s = samples[i] >> 14;   // normaliza 24 bits
    sum += (uint64_t)(s * s);
  }

  uint32_t rms = sqrt(sum / FRAME_SIZE);

  // ---- Detecção de início de fala ----
  if (rms > SPEECH_THRESHOLD && !capturing) {
    capturing = true;
    bufferIndex = 0;
    Serial.println(">>> FALANDO");
  }

  // ---- Captura de áudio ----
  if (capturing) {
    memcpy(&audioBuffer[bufferIndex], samples, sizeof(samples));
    bufferIndex += FRAME_SIZE;

    if (bufferIndex >= FRAME_SIZE * BUFFER_FRAMES) {
      capturing = false;
      Serial.println("<<< PROCESSANDO");
      processCommand();
    }
  }

  if (WiFi.status() == WL_CONNECTED) 
  {
    switch(state_cmd)
    {
      case CMD_NONE:
        break;
      case CMD_LIGA:
        lampOn();
        state_cmd = CMD_NONE;
        break;
      case CMD_DESLIGA:
        lampOff();
        state_cmd = CMD_NONE;
        break;
      default:
        break;
    }
  }

  delay(10);
}

// ================= I2S ======================
void setupI2S() {
i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = (i2s_comm_format_t)(
      I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB
  ),
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
  .dma_buf_count = 4,
  .dma_buf_len = FRAME_SIZE,
  .use_apll = false
};

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

// ================= PROCESSAMENTO ======================
void processCommand() {
  uint64_t energy = 0;
  int zeroCrossings = 0;

  for (int i = 1; i < bufferIndex; i++) {
    int32_t a = audioBuffer[i - 1];
    int32_t b = audioBuffer[i];

    energy += abs(a);

    if ((a > 0 && b < 0) || (a < 0 && b > 0)) {
      zeroCrossings++;
    }
  }

  Serial.printf("Energia: %llu | ZC: %d\n", energy, zeroCrossings);
  classify(energy, zeroCrossings);
}

// ================= CLASSIFICADOR ======================
void classify(uint64_t energy, int zc) {

  // ---- DESLIGA ----
  if (energy > 110000000000ULL) {
    Serial.println("COMANDO DETECTADO: DESLIGA");
    state_cmd = CMD_DESLIGA;
    return;
  }

  // ---- LIGA ----
  if (energy > 50000000000ULL && energy <= 110000000000ULL) {
    Serial.println("COMANDO DETECTADO: LIGA");
    state_cmd = CMD_LIGA;
    return;
  }

  Serial.println("COMANDO: DESCONHECIDO");
}


