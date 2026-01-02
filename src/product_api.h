#ifndef PRODUCT_API_H
#define PRODUCT_API_H

#include <Arduino.h>
#include "cache.h"

bool fetchProductInfo(String barcode, ProductCacheItem &out);

#endif