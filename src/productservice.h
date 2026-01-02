#pragma once

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <vector>
#include "common.h"

class ProductService
{
private:
    const String Endpoint = "https://fra.cloud.appwrite.io/v1";
    const String ProjectId = "6954045e003c75c1c3bf";
    const String DatabaseId = "695404ac0021bf7d9707";
    const String CollectionId = "products";

    String apiKey;

    String generateId(int length = 20);

public:
    ProductService(String appWriteKey);

    std::vector<Product> getProducts(const std::vector<String> &queries = {});
    bool addOrUpdateProduct(Product &product);
    bool addProduct(Product &product);
    bool updateProduct(Product &product);
    bool deleteProduct(String rowId);
};

extern ProductService service;