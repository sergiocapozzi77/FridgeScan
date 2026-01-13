#ifndef PRODUCT_API_H
#define PRODUCT_API_H

#include <Arduino.h>
#include "cache.h"

bool getProduct(const String &barcode, ProductCacheItem &out);

#endif