#include "product_api.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void fetchProductInfo(String barcode)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return;
    }

    HTTPClient http;
    String url = "https://world.openfoodfacts.org/api/v0/product/" + barcode + ".json";

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        String payload = http.getString();

        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, payload);

        if (error)
        {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            http.end();
            return;
        }

        int status = doc["status"];

        if (status == 1)
        {
            String productName = doc["product"]["product_name"] | "Unknown";
            Serial.print("Product Name: ");
            Serial.println(productName);
        }
        else
        {
            Serial.println("Product not found");
            Serial.print("HTTP Error: ");
            Serial.println(httpResponseCode);
            /* Serial.println("Retrying...");
             delay(2000); // Wait before retrying
             fetchProductInfo(barcode); // Retry fetching product info*/
        }
    }
    else
    {
        Serial.print("HTTP Error: ");
        Serial.println(httpResponseCode);
    }

    http.end();

    // let scheduler breathe
    vTaskDelay(1);
}
