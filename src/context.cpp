#include <cstdlib>
#include <node.h>
#include <node_buffer.h>
#include <cubeb/cubeb.h>

#include "context.h"
#include "common.h"

using namespace v8;
using namespace node;

CubebContext::CubebContext (const char *nname) : name(nname) {
	error_code = cubeb_init(&ctx, name);
}

CubebContext::~CubebContext () {
	if (ctx != NULL) {
		cubeb_destroy(ctx);
	}
}

void CubebContext::Initialize (Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
	constructor_template->SetClassName(String::NewSymbol("CubebContext"));

	target->Set(String::NewSymbol("Context"), constructor_template->GetFunction());
}

Handle<Value> CubebContext::New (const Arguments &args) {
	HandleScope scope;

	if (!args[0]->IsString()) {
		return ThrowException(Exception::TypeError(String::New("The name must be a string")));
	}

	String::Utf8Value str(args[0]);

	CubebContext *nodecubeb = new CubebContext(ToCString(str));

	if (nodecubeb->error_code != CUBEB_OK) {
		return ThrowException(Exception::ReferenceError(String::New("Error initializing cubeb library")));
	}

	nodecubeb->Wrap(args.This());

	return args.This();
}

Persistent<FunctionTemplate> CubebContext::constructor_template;
