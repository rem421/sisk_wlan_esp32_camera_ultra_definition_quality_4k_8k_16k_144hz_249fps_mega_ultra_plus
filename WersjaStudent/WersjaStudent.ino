#include "esp_camera.h"
#include <WiFi.h>
#include "board_config.h"

// ===========================
// ETAP 1: WLAN + TCP/IP DIAGNOSTYKA
// ETAP 2: PARAMETRY KAMERY (PSRAM / jakość / bufor)
// ===========================

// TODO(1)(5): USE_STATIC_IP = 0 → DHCP, USE_STATIC_IP = 1 → static IP
#define USE_STATIC_IP /* TODO(1): 0 albo 1 */

// TODO(2): Dane WiFi (SSID + hasło)
const char *ssid     = /* TODO(2): "Twoje_SSID" */;
const char *password = /* TODO(2): "Twoje_haslo" */;

void startCameraServer();
void setupLedFlash();

#if USE_STATIC_IP
// TODO(6): Statyczna konfiguracja IP (na bazie parametrów z DHCP: IP/Mask/GW/DNS)
IPAddress local_IP(/* TODO(6): np. 192,168,0,222 */);
IPAddress gateway(/* TODO(6): np. 192,168,0,1 */);
IPAddress subnet(/* TODO(6): np. 255,255,255,0 */);
IPAddress dns1(/* TODO(6): DNS1 */);
IPAddress dns2(/* TODO(6): DNS2 */);
#endif

static const char* wifiModeToStr(wifi_mode_t m) {
  switch (m) {
    case WIFI_OFF:    return "OFF";
    case WIFI_STA:    return "STA";
    case WIFI_AP:     return "AP";
    case WIFI_AP_STA: return "AP+STA";
    default:          return "UNKNOWN";
  }
}

// TODO(3): Diagnostyka WLAN (IEEE 802.11)
static void printWifiDiag() {
  // Wypisz: Mode, SSID, BSSID(AP MAC), Channel, RSSI[dBm], STA MAC
  // Podpowiedź: WiFi.getMode(), WiFi.SSID(), WiFi.BSSIDstr(), WiFi.channel(), WiFi.RSSI(), WiFi.macAddress()
  /* TODO(3): uzupełnij funkcję */
}

// TODO(4): Diagnostyka IP (TCP/IP)
static void printIpDiag() {
  // Wypisz: IP, Mask, Gateway, DNS1, DNS2
  // Podpowiedź: WiFi.localIP(), WiFi.subnetMask(), WiFi.gatewayIP(), WiFi.dnsIP(0/1)
  /* TODO(4): uzupełnij funkcję */
}

static void applyCameraDefaults(camera_config_t &config) {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count     = 1;
}

static void tuneCameraForMemory(camera_config_t &config) {
  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.frame_size = FRAMESIZE_VGA;
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.fb_count = 1;
    config.jpeg_quality = 12;
  }
}

static bool connectWifi() {
  WiFi.mode(WIFI_STA);

#if USE_STATIC_IP
  bool ok = WiFi.config(local_IP, gateway, subnet, dns1, dns2);
  if (!ok) {
    Serial.println("WiFi.config() failed");
    return false;
  }
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  unsigned long t0 = millis();
  const unsigned long timeoutMs = 15000;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - t0 > timeoutMs) {
      Serial.println("\nWiFi connect TIMEOUT");
      return false;
    }
  }

  Serial.println("\nWiFi connected");
  Serial.print("Connect time [ms]: ");
  Serial.println(millis() - t0);

  printWifiDiag();
  printIpDiag();
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  applyCameraDefaults(config);
  tuneCameraForMemory(config);

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

#if defined(LED_GPIO_NUM)
  setupLedFlash();
#endif

  if (!connectWifi()) {
    return;
  }

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  delay(10000);
}
