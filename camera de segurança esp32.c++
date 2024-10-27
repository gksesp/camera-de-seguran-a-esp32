#include "esp_camera.h"
#include <WiFi.h>

// Defina as credenciais da rede Wi-Fi
const char* ssid = "camera wifi";
const char* password = "aezakmi1";

// Endereço IP e porta para acesso à câmera
IPAddress local_IP(192, 168, 1, 200);  // Altere conforme necessário
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiServer server(80);

void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sccb_sda = 26;
  config.pin_sccb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Inicializa a câmera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Erro ao iniciar a câmera: 0x%x", err);
    return;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Conecte-se à rede Wi-Fi
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado à rede Wi-Fi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Inicia o servidor
  server.begin();
  startCamera();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Espera pela requisição HTTP do cliente
  while (!client.available()) {
    delay(1);
  }

  String req = client.readStringUntil('\r');
  client.flush();

  if (req.indexOf("/jpg") != -1) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Erro ao capturar imagem");
      client.println("HTTP/1.1 500 Internal Server Error");
      return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: image/jpeg");
    client.println("Content-Length: " + String(fb->len));
    client.println();
    client.write(fb->buf, fb->len);
    esp_camera_fb_return(fb);
  } else {
    client.println("HTTP/1.1 404 Not Found");
  }
  client.stop();
}
