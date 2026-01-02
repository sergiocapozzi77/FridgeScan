#include "product_api.h"
#include "productservice.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "cache.h"

String mapUkSupermarketCategory(const JsonArray &tags)
{
    for (JsonVariant tag : tags)
    {
        String t = tag.as<String>();
        t.toLowerCase();

        // ---- Fruit & Veg ----
        if (t.indexOf("vegetable") >= 0 || t.indexOf("vegetables") >= 0 ||
            t.indexOf("veg") >= 0 || t.indexOf("fruit") >= 0)
            return "Fruit & Veg";

        // ---- Meat & Fish ----
        if (t.indexOf("meat") >= 0 || t.indexOf("poultry") >= 0 ||
            t.indexOf("beef") >= 0 || t.indexOf("chicken") >= 0 ||
            t.indexOf("fish") >= 0 || t.indexOf("seafood") >= 0)
            return "Meat & Fish";

        // ---- Dairy & Eggs ----
        if (t.indexOf("dairy") >= 0 || t.indexOf("milk") >= 0 ||
            t.indexOf("cheese") >= 0 || t.indexOf("yogurt") >= 0 ||
            t.indexOf("egg") >= 0)
            return "Dairy & Eggs";

        // ---- Bakery ----
        if (t.indexOf("bread") >= 0 || t.indexOf("bakery") >= 0 ||
            t.indexOf("pastry") >= 0)
            return "Bakery";

        // ---- Frozen ----
        if (t.indexOf("frozen") >= 0)
            return "Frozen";

        // ---- Drinks ----
        if (t.indexOf("beverage") >= 0 || t.indexOf("drink") >= 0 ||
            t.indexOf("juice") >= 0 || t.indexOf("water") >= 0)
            return "Drinks";

        // ---- Snacks ----
        if (t.indexOf("snack") >= 0 || t.indexOf("crisps") >= 0 ||
            t.indexOf("chocolate") >= 0 || t.indexOf("sweets") >= 0)
            return "Snacks";

        // ---- Cereal & Breakfast ----
        if (t.indexOf("cereal") >= 0 || t.indexOf("breakfast") >= 0 ||
            t.indexOf("oats") >= 0)
            return "Cereal & Breakfast";

        // ---- Tins & Jars ----
        if (t.indexOf("canned") >= 0 || t.indexOf("tinned") >= 0 ||
            t.indexOf("jarred") >= 0)
            return "Tins & Jars";

        // ---- Pasta, Rice & Grains ----
        if (t.indexOf("pasta") >= 0 || t.indexOf("rice") >= 0 ||
            t.indexOf("grain") >= 0 || t.indexOf("noodle") >= 0)
            return "Pasta, Rice & Grains";

        // ---- Condiments & Sauces ----
        if (t.indexOf("sauce") >= 0 || t.indexOf("condiment") >= 0 ||
            t.indexOf("spread") >= 0 || t.indexOf("spreads") >= 0)
            return "Condiments & Sauces";

        // ---- Household (cleaning, etc.) ----
        if (t.indexOf("household") >= 0 || t.indexOf("cleaning") >= 0)
            return "Household";
    }

    return "Other";
}

bool fetchProductInfo(String barcode, ProductCacheItem &out)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return false;
    }

    bool success = false;

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
            return success;
        }

        int status = doc["status"];

        if (status == 1)
        {
            String productName = doc["product"]["product_name"] | "";
            JsonArray tags = doc["product"]["categories_tags"].as<JsonArray>();

            Serial.print("Product found: ");
            Serial.println(productName);

            String category = "Other";
            if (!tags.isNull())
            {
                category = mapUkSupermarketCategory(tags);
            }

            if (!productName.isEmpty())
            {
                Product p;
                p.name = productName;
                p.quantity = 1;
                p.category = category;
                p.barcode = barcode;

                out.name = productName;
                out.category = category;
                out.barcode = barcode;
                success = true;

                service.addOrUpdateProduct(p);
            }
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

    return success;
}

bool getProduct(const String &barcode, ProductCacheItem &out)
{
    ProductCacheItem cached;

    if (productCache.tryGet(barcode, cached))
    {
        Serial.println(">> Product from cache (local)");
        return true;
    }

    Serial.println(">> Product from server");

    bool success = fetchProductInfo(barcode, out);

    if (success)
    {
        productCache.add(out);
    }

    return success;
}
