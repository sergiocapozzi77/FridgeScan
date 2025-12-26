#include <Arduino.h>
#include <WiFi.h>
#include "product_api.h"

// Define UART pins for the barcode reader (adjust as needed for your wiring)
#define BARCODE_RX_PIN 16 // Connect to TX of GM865
#define BARCODE_TX_PIN 17 // Connect to RX of GM865
#define BARCODE_BAUD 9600 // Default baud rate for GM865, confirm in datasheet

// WiFi credentials (replace with your own)
const char *ssid = "CommunityFibre10Gb_1206C";
const char *password = "4kF3zadv5@";

// put function declarations here:
void setup();
void loop();
bool isValidBarcode(const String &barcode);

void setup()
{
  // Initialize serial for debugging
  Serial.begin(115200);
  while (!Serial)
  {
    ; // Wait for serial port to connect (for some boards)
  }
  Serial.println("Barcode Reader Initialized");

  // Initialize UART for barcode reader
  Serial1.begin(BARCODE_BAUD, SERIAL_8N1, BARCODE_RX_PIN, BARCODE_TX_PIN);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop()
{
  // Check if data is available from the barcode reader
  if (Serial1.available())
  {
    // Read the barcode data until a newline (assuming GM865 sends \n or \r\n)
    String barcode = Serial1.readStringUntil('\n');
    // Remove any trailing \r if present
    barcode.trim();

    if (barcode.length() > 0)
    {
      Serial.print("Barcode scanned: ");
      Serial.println(barcode);

      // Validate barcode format (optional, based on GM865 output)
      if (isValidBarcode(barcode))
      {
        // Fetch product info from API
        fetchProductInfo(barcode);
      }
      else
      {
        Serial.println("Invalid barcode format");
      }
    }
  }
}

// Helper function to validate barcode format (example implementation)
bool isValidBarcode(const String &barcode)
{
  // Example: Ensure barcode is numeric and within a certain length range
  for (char c : barcode)
  {
    if (!isDigit(c))
      return false;
  }
  return barcode.length() >= 8 && barcode.length() <= 13; // Adjust length as needed
}