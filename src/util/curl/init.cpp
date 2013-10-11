#include "./init.h"

#include "src/util/check/check.h"

#include <curl/curl.h>

namespace curl {
namespace memory {

void* Malloc(size_t size) {
	return malloc(size);
}

void Free(void* ptr) {
	free(ptr);
}

void* Realloc(void* ptr, size_t size) {
	return realloc(ptr, size);
}

char* Strdup(const char *str) {
	return _strdup(str);
}

void* Calloc(size_t nmemb, size_t size) {
	return calloc(nmemb, size);
}

}
}

curl::Init::Init() {
#ifdef NDEBUG
	const CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
#else
	const CURLcode code = curl_global_init_mem(CURL_GLOBAL_ALL, curl::memory::Malloc, curl::memory::Free, curl::memory::Realloc, curl::memory::Strdup, curl::memory::Calloc);
#endif
	Check(code == 0);
}

curl::Init::~Init() {
	curl_global_cleanup();
}
 