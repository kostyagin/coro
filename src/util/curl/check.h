#pragma once

#include <curl/curl.h>

namespace curl {

void CheckCode(CURLcode code);
void CheckCode(CURLMcode code);

}
