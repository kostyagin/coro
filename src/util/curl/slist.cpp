#include "./slist.h"

#include "src/util/check/check.h"

curl::SList::SList()
	: slist(NULL)
{
}

curl::SList::~SList() {
	curl_slist_free_all(slist);
}

curl::SList::operator struct curl_slist*() {
	return slist;
}

void curl::SList::Append(const char* string) {
	slist = curl_slist_append(slist, string);
	Check(slist != NULL);
}

bool curl::SList::Empty() const {
	return slist == NULL;
}