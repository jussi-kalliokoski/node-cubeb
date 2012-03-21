#include <cstdlib>
#include <node.h>
#include <node_buffer.h>
#include <cubeb/cubeb.h>

#include "stream.h"
#include "context.h"
#include "common.h"

using namespace v8;
using namespace node;

CubebStream::CubebStream
(cubeb *cctx, const char *nname, unsigned int cc, unsigned int sr, unsigned int bs, unsigned int lt) :
ctx(cctx), name(nname), channelCount(cc), sampleRate(sr), bufferSize(bs), latency(lt) {
	cubeb_stream_params params;

	params.format = CUBEB_SAMPLE_FLOAT32NE;
	params.rate = sampleRate;
	params.channels = channelCount;

	user_data = (cb_user_data*) malloc(sizeof(*user_data));
	if (user_data == NULL) {
		error_code = CUBEB_ERROR;
		return;
	}

	user_data->stream = this;

	error_code = cubeb_stream_init(ctx, &stream, name, params, latency, DataCB, StateCB, user_data);
}

CubebStream::~CubebStream () {
	stop();

	if (stream != NULL) {
		cubeb_stream_destroy(stream);
	}

	if (user_data != NULL) {
		free(user_data);
	}
}

void CubebStream::stop () {
}

void CubebStream::Initialize (Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
	constructor_template->SetClassName(String::NewSymbol("CubebStream"));

	target->Set(String::NewSymbol("Stream"), constructor_template->GetFunction());
}

Handle<Value> CubebStream::New (const Arguments &args) {
	HandleScope scope;

	CubebContext *nodecubeb = ObjectWrap::Unwrap<CubebContext>(args[0]->ToObject());

	if (!args[1]->IsString()) {
		return ThrowException(Exception::TypeError(String::New("The name must be a string")));
	}

	String::Utf8Value str(args[1]);

	unsigned int cc = args[2]->Int32Value();
	unsigned int sr = args[3]->Int32Value();
	unsigned int bs = args[4]->Int32Value();
	unsigned int lt = args[5]->Int32Value();

	CubebStream *cubebstream = new CubebStream(
		nodecubeb->ctx,
		ToCString(str),
		cc, sr, bs, lt
	);

	cubebstream->JSObject = args.This();

	if (cubebstream->error_code != CUBEB_OK) {
		return ThrowException(Exception::ReferenceError(String::New("Error initializing stream")));
	}

	cubebstream->Wrap(args.This());

	return args.This();
}

long CubebStream::DataCB (cubeb_stream *stream, void *user, void *buffer, long nframes) {
	HandleScope scope;

	struct CubebStream::cb_user_data *u = (struct CubebStream::cb_user_data *)user;

	CubebStream *cs = u->stream;

	Handle<Object> obj = cs->JSObject->ToObject();
	if (!obj->IsFunction()) {
		// FIXME: This isn't a good way to handle the problem.
		return CUBEB_ERROR;
	}

	Local<Function> cb = Local<Function>::Cast(obj->Get(String::New("ondata")));

	// TODO: Call the function with a Float32Array as an argument.

	return CUBEB_OK;
}

int CubebStream::StateCB(cubeb_stream *stream, void *user, cubeb_state state) {

	struct CubebStream::cb_user_data *u = (struct CubebStream::cb_user_data *)user;

	CubebStream *cs = u->stream;

	Handle<Object> obj = cs->JSObject->ToObject();
	if (!obj->IsFunction()) {
		// FIXME: This isn't a good way to handle the problem.
		return CUBEB_ERROR;
	}

	Local<Function> cb = Local<Function>::Cast(obj->Get(String::New("onstatechange")));

	// TODO: Call the function with the state as its argument and return the output of the function.
	
	return CUBEB_OK;
}

Persistent<FunctionTemplate> CubebStream::constructor_template;
