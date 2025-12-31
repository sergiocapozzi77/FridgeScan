#include <Arduino.h>
#include <WiFi.h>
#include "product_api.h"
#include "productservice.cpp"

// ===== UART PINS FOR ESP32-S3 =====
#define BARCODE_RX_PIN 7 // ESP32-S3 GPIO7  -> GM865 TX
#define BARCODE_TX_PIN 6 // ESP32-S3 GPIO6  -> GM865 RX
#define BARCODE_BAUD 9600

// ===== Wi-Fi (replace with yours) =====
const char *ssid = "CommunityFibre10Gb_1206C";
const char *password = "4kF3zadv5@";

// Queue to pass barcodes to the worker task
QueueHandle_t barcodeQueue;

ProductService service("standard_b303af6fe35d0302c1405b926b41bef69847c4df0fec3faf0a6244fefe1a1f090539d7599a3af53d2f30abfd98987161b6454fe462429fc5c589a28d3f68a372f130c1b162c685eeee21e4cf2c8d7a003df8375eea7813a4142d958d95a7c39419aa1b2edd1844faa3897ef68b14ad1b8dd951b747c21653ce03ce650b835044");

void fetchTask(void *param)
{
  String barcode;

  for (;;)
  {
    if (xQueueReceive(barcodeQueue, &barcode, portMAX_DELAY) == pdTRUE)
    {
      fetchProductInfo(barcode);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.println("Booting…");

  // ---- Barcode UART ----
  Serial1.begin(BARCODE_BAUD, SERIAL_8N1, BARCODE_RX_PIN, BARCODE_TX_PIN);
  Serial.println("Serial1 (GM865) ready");

  // ---- Wi-Fi ----
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Create queue (10 strings max)
  barcodeQueue = xQueueCreate(10, sizeof(String));

  // Create background fetch task
  xTaskCreatePinnedToCore(
      fetchTask,
      "FetchTask",
      10000,
      NULL,
      1,
      NULL,
      1 // run on core 1
  );
}

void loop()
{
  Serial.println("Adding product");
  Product p;
  p.name = "Marmite";
  p.quantity = 2;
  p.category = "Dairy";

  bool result = service.addProduct(p);

  Serial.print("Added product " + result ? "successfully. ID: " + p.rowId : "failed");

  if (Serial1.available())
  {
    String barcode = Serial1.readStringUntil('\n');
    barcode.trim();

    if (barcode.length())
    {
      Serial.print("Scanned: ");
      Serial.println(barcode);

      // Push to queue (non-blocking)
      xQueueSend(barcodeQueue, &barcode, 0);

      Serial.print("Ready for next scan\n");
    }
  }
}
