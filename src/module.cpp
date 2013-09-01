#include <node.h>

#include "constants.h"
#include "context.h"
#include "stream.h"

void init (v8::Handle<v8::Object> target) {
	CubebConstants::Initialize(target);
	CubebContext::Initialize(target);
	CubebStream::Initialize(target);
}

NODE_MODULE(node_cubeb, init)
