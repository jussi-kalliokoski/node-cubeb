#include <cstdlib>
#include <node.h>
#include <node_buffer.h>
#include <cubeb/cubeb.h>

#include "constants.h"

#define set_v8_constant(name, value, type) target->Set(String::NewSymbol(name), type::New(value))

using namespace v8;
using namespace node;

void CubebConstants::Initialize (Handle<Object> target) {
	HandleScope scope;

	set_v8_constant("SAMPLE_INT16LE", CUBEB_SAMPLE_S16LE, Integer);
	set_v8_constant("SAMPLE_INT16BE", CUBEB_SAMPLE_S16BE, Integer);
	set_v8_constant("SAMPLE_INT16NE", CUBEB_SAMPLE_S16NE, Integer);
	set_v8_constant("SAMPLE_FLOAT32LE", CUBEB_SAMPLE_FLOAT32LE, Integer);
	set_v8_constant("SAMPLE_FLOAT32BE", CUBEB_SAMPLE_FLOAT32BE, Integer);
	set_v8_constant("SAMPLE_FLOAT32NE", CUBEB_SAMPLE_FLOAT32NE, Integer);

	set_v8_constant("CUBEB_OK", CUBEB_OK, Integer);
	set_v8_constant("CUBEB_ERROR", CUBEB_ERROR, Integer);
	set_v8_constant("CUBEB_ERROR_INVALID_FORMAT", CUBEB_ERROR_INVALID_FORMAT, Integer);

	set_v8_constant("STATE_STARTED", CUBEB_STATE_STARTED, Integer);
	set_v8_constant("STATE_STOPPED", CUBEB_STATE_STOPPED, Integer);
	set_v8_constant("STATE_DRAINED", CUBEB_STATE_DRAINED, Integer);
	set_v8_constant("STATE_ERROR", CUBEB_STATE_ERROR, Integer);
}
