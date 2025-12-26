#include "product_api.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void fetchProductInfo(String barcode)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String url = "https://world.openfoodfacts.org/api/v0/product/" + barcode + ".json";
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            Serial.println("API Response:");
            Serial.println(payload);

            // Parse JSON
            DynamicJsonDocument doc(4096);
            DeserializationError error = deserializeJson(doc, payload);

            if (error)
            {
                Serial.print("JSON parsing failed: ");
                Serial.println(error.c_str());
                return;
            }

            int status = doc["status"];
            if (status == 1)
            {
                // Product found
                String productName = doc["product"]["product_name"] | "Unknown";
                Serial.print("Product Name: ");
                Serial.println(productName);

                // Calories per 100g
                float calories = doc["product"]["nutriments"]["energy-kcal_100g"] | 0.0;
                if (calories > 0)
                {
                    Serial.print("Calories per 100g: ");
                    Serial.println(calories);
                }
                else
                {
                    Serial.println("Calories info not available");
                }

                // Other dietary details
                String ingredients = doc["product"]["ingredients_text"] | "";
                if (ingredients.length() > 0)
                {
                    Serial.print("Ingredients: ");
                    Serial.println(ingredients);
                }

                String allergens = doc["product"]["allergens"] | "";
                if (allergens.length() > 0)
                {
                    Serial.print("Allergens: ");
                    Serial.println(allergens);
                }

                // Add more fields as needed, e.g., fat, carbs, etc.
                float fat = doc["product"]["nutriments"]["fat_100g"] | 0.0;
                if (fat > 0)
                {
                    Serial.print("Fat per 100g: ");
                    Serial.println(fat);
                }

                float carbs = doc["product"]["nutriments"]["carbohydrates_100g"] | 0.0;
                if (carbs > 0)
                {
                    Serial.print("Carbohydrates per 100g: ");
                    Serial.println(carbs);
                }

                float protein = doc["product"]["nutriments"]["proteins_100g"] | 0.0;
                if (protein > 0)
                {
                    Serial.print("Protein per 100g: ");
                    Serial.println(protein);
                }
            }
            else
            {
                Serial.println("Product not found in database");
            }
        }
        else
        {
            Serial.print("HTTP Error: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }
    else
    {
        Serial.println("WiFi not connected");
    }
}