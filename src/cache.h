#ifndef CACHE_H
#define CACHE_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "common.h"

class ProductCache
{
public:
    ProductCache();

    bool begin(); // init + load from SPIFFS
    bool tryGet(const String &id, ProductCacheItem &out);
    void add(const ProductCacheItem &item);
    size_t count() const;

private:
    size_t _capacity;
    size_t _index;
    size_t _size;
    String _filePath;

    ProductCacheItem *_items;

    SemaphoreHandle_t _mutex;

    bool loadFromDisk();
    bool saveToDisk();
};

extern ProductCache productCache;
#endif
