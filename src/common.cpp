#include <v8.h>
#include "common.h"

const char* ToCString (const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}
