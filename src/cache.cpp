#include "cache.h"

ProductCache productCache;

ProductCache::ProductCache()
{
    _capacity = 100;
    _filePath = "/product_cache.json";
    _index = 0;
    _size = 0;

    _items = new ProductCacheItem[_capacity];

    _mutex = xSemaphoreCreateMutex();
}

bool ProductCache::begin()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("❌ SPIFFS mount failed");
        return false;
    }

    return loadFromDisk();
}

bool ProductCache::loadFromDisk()
{
    xSemaphoreTake(_mutex, portMAX_DELAY);

    File f = SPIFFS.open(_filePath, "r");
    if (!f)
    {
        Serial.println("ℹ️ Cache file not found — starting empty");
        xSemaphoreGive(_mutex);
        return true;
    }

    DynamicJsonDocument doc(4096);

    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err)
    {
        Serial.println("❌ Cache JSON parse failed — clearing");
        _size = 0;
        _index = 0;
        xSemaphoreGive(_mutex);
        return false;
    }

    JsonArray arr = doc["items"];

    _size = min((size_t)arr.size(), _capacity);
    _index = doc["index"] | 0;

    for (size_t i = 0; i < _size; i++)
    {
        _items[i].category = arr[i]["category"].as<String>();
        _items[i].name = arr[i]["name"].as<String>();
        _items[i].barcode = arr[i]["barcode"].as<String>();
    }

    xSemaphoreGive(_mutex);
    return true;
}

bool ProductCache::saveToDisk()
{
    xSemaphoreTake(_mutex, portMAX_DELAY);

    DynamicJsonDocument doc(4096);

    doc["index"] = _index;
    JsonArray arr = doc.createNestedArray("items");

    for (size_t i = 0; i < _size; i++)
    {
        JsonObject o = arr.createNestedObject();
        o["category"] = _items[i].category;
        o["name"] = _items[i].name;
        o["barcode"] = _items[i].barcode;
    }

    File f = SPIFFS.open(_filePath, "w");
    if (!f)
    {
        Serial.println("❌ Failed to open cache file for write");
        xSemaphoreGive(_mutex);
        return false;
    }

    serializeJson(doc, f);
    f.close();

    xSemaphoreGive(_mutex);
    return true;
}

bool ProductCache::tryGet(const String &barcode, ProductCacheItem &out)
{
    xSemaphoreTake(_mutex, portMAX_DELAY);

    for (size_t i = 0; i < _size; i++)
    {
        if (_items[i].barcode == barcode)
        {
            out = _items[i];
            xSemaphoreGive(_mutex);
            return true;
        }
    }

    xSemaphoreGive(_mutex);
    return false;
}

void ProductCache::add(const ProductCacheItem &item)
{
    xSemaphoreTake(_mutex, portMAX_DELAY);

    _items[_index] = item;

    _index = (_index + 1) % _capacity;
    if (_size < _capacity)
        _size++;

    xSemaphoreGive(_mutex);

    saveToDisk(); // persist changes
}

size_t ProductCache::count() const
{
    return _size;
}
