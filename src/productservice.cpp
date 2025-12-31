#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <vector>

struct Product
{
    String name;
    int quantity;
    String category;
    String rowId;
};

class ProductService
{

private:
    const String Endpoint = "https://fra.cloud.appwrite.io/v1";
    const String ProjectId = "6954045e003c75c1c3bf";
    const String DatabaseId = "695404ac0021bf7d9707";
    const String CollectionId = "products";

    String apiKey;

public:
    ProductService(String appWriteKey)
        : apiKey(appWriteKey) {}

    // ---------- GET PRODUCTS ----------
    std::vector<Product> getProducts()
    {

        std::vector<Product> result;

        WiFiClientSecure client;
        client.setInsecure(); // (or load proper cert)

        HTTPClient http;

        String url = Endpoint + "/tablesdb/" + DatabaseId +
                     "/tables/" + CollectionId + "/rows";

        if (!http.begin(client, url))
            return result;

        http.addHeader("X-Appwrite-Project", ProjectId);
        http.addHeader("X-Appwrite-Key", apiKey);

        int code = http.GET();

        if (code == 200)
        {
            DynamicJsonDocument doc(8192);
            deserializeJson(doc, http.getString());

            JsonArray rows = doc["rows"].as<JsonArray>();

            for (JsonObject r : rows)
            {
                Product p;
                p.name = r["name"] | "";
                p.quantity = r["quantity"] | 0;
                p.category = r["category"] | "Other";
                p.rowId = r["$id"] | "";
                result.push_back(p);
            }
        }

        http.end();
        return result;
    }

    // ---------- ADD PRODUCT ----------
    bool addProduct(Product &product)
    {

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        String url = Endpoint + "/tablesdb/" + DatabaseId +
                     "/tables/" + CollectionId + "/rows";

        if (!http.begin(client, url))
            return false;

        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-Appwrite-Project", ProjectId);
        http.addHeader("X-Appwrite-Key", apiKey);

        String rowId = generateId();

        DynamicJsonDocument body(512);
        body["rowId"] = rowId;
        body["data"]["name"] = product.name;
        body["data"]["quantity"] = product.quantity;
        body["data"]["category"] = product.category;

        String payload;
        serializeJson(body, payload);

        int code = http.POST(payload);

        if (code == 201 || code == 200)
        {
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, http.getString());
            product.rowId = doc["$id"] | "";
            http.end();
            return true;
        }

        http.end();
        return false;
    }

    // ---------- DELETE ----------
    bool deleteProduct(String rowId)
    {

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        String url = Endpoint + "/tablesdb/" + DatabaseId +
                     "/tables/" + CollectionId + "/rows/" + rowId;

        if (!http.begin(client, url))
            return false;

        http.addHeader("X-Appwrite-Project", ProjectId);
        http.addHeader("X-Appwrite-Key", apiKey);

        int code = http.sendRequest("DELETE");

        http.end();
        return code == 204 || code == 200;
    }

    // ---------- UPDATE ----------
    bool updateProduct(Product product)
    {

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        String url = Endpoint + "/tablesdb/" + DatabaseId +
                     "/tables/" + CollectionId + "/rows/" + product.rowId;

        if (!http.begin(client, url))
            return false;

        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-Appwrite-Project", ProjectId);
        http.addHeader("X-Appwrite-Key", apiKey);

        DynamicJsonDocument body(512);
        body["data"]["quantity"] = product.quantity;
        body["data"]["category"] = product.category;

        String payload;
        serializeJson(body, payload);

        int code = http.sendRequest("PATCH", payload);

        http.end();
        return code == 200;
    }

    // ---------- ID GENERATOR ----------
    String generateId(int length = 20)
    {
        const char chars[] =
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789";

        String id = "";
        for (int i = 0; i < length; i++)
        {
            id += chars[random(strlen(chars))];
        }
        return id;
    }
};
