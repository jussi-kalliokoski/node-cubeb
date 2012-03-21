#include <node.h>

#include "context.h"
#include "stream.h"

extern "C" void init (v8::Handle<v8::Object> target) {
	v8::HandleScope scope;

	CubebContext::Initialize(target);
	CubebStream::Initialize(target);
}
