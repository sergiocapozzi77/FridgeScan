#pragma once

#include <Arduino.h>

struct ProductCacheItem
{
    String name;
    String category;
    String barcode;
};

struct Product : ProductCacheItem
{
    int quantity;
    String rowId;
};