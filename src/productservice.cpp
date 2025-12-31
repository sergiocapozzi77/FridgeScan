#include "secrets.h"
#include "ProductService.h"

ProductService service(APPWRITE_API_KEY);

// ---------- CONSTRUCTOR ----------
ProductService::ProductService(String appWriteKey)
    : apiKey(appWriteKey)
{
}

// ---------- GET PRODUCTS ----------
std::vector<Product> ProductService::getProducts(const std::vector<String> &queries)
{
    std::vector<Product> result;
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = Endpoint + "/tablesdb/" + DatabaseId + "/tables/" + CollectionId + "/rows";

    if (!queries.empty())
    {
        url += "?";
        for (size_t i = 0; i < queries.size(); i++)
        {
            url += "queries[" + String(i) + "]=" + queries[i];
            if (i < queries.size() - 1)
                url += "&";
        }
    }

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

// ---------- ADD OR UPDATE ----------
bool ProductService::addOrUpdateProduct(Product &product)
{
    std::vector<String> query;
    String q = "{\"method\":\"equal\",\"attribute\":\"name\",\"values\":[\"" + product.name + "\"]}";
    query.push_back(q);

    std::vector<Product> existing = getProducts(query);

    if (!existing.empty())
    {
        Product existingProduct = existing[0];
        existingProduct.quantity += product.quantity;
        return updateProduct(existingProduct);
    }
    else
    {
        return addProduct(product);
    }
}

// ---------- ADD PRODUCT ----------
bool ProductService::addProduct(Product &product)
{
    Serial.print("Adding product: ");
    Serial.println(product.name);

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = Endpoint + "/tablesdb/" + DatabaseId + "/tables/" + CollectionId + "/rows";

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

// ---------- UPDATE PRODUCT ----------
bool ProductService::updateProduct(Product &product)
{
    Serial.print("Updating product: ");
    Serial.println(product.name);

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = Endpoint + "/tablesdb/" + DatabaseId + "/tables/" + CollectionId + "/rows/" + product.rowId;

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

// ---------- DELETE ----------
bool ProductService::deleteProduct(String rowId)
{
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = Endpoint + "/tablesdb/" + DatabaseId + "/tables/" + CollectionId + "/rows/" + rowId;

    if (!http.begin(client, url))
        return false;

    http.addHeader("X-Appwrite-Project", ProjectId);
    http.addHeader("X-Appwrite-Key", apiKey);

    int code = http.sendRequest("DELETE");

    http.end();
    return code == 204 || code == 200;
}

// ---------- ID GENERATOR ----------
String ProductService::generateId(int length)
{
    const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    String id = "";
    for (int i = 0; i < length; i++)
    {
        id += chars[random(strlen(chars))];
    }
    return id;
}