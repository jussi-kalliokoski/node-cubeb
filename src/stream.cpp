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
(cubeb *cctx, const char *nname, cubeb_sample_format sf, unsigned int cc, unsigned int sr, unsigned int bs, unsigned int lt, Persistent<Function> ddatacb, Persistent<Function> sstatecb) :
ctx(cctx), name(nname), channelCount(cc), sampleRate(sr), bufferSize(bs), latency(lt), statecb(sstatecb), datacb(ddatacb) {
	cubeb_stream_params params;

	params.format = sampleFormat;
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

	datacb.Dispose();
	statecb.Dispose();
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

	cubeb_sample_format sf = (cubeb_sample_format) args[2]->Int32Value();
	unsigned int cc = args[3]->Int32Value();
	unsigned int sr = args[4]->Int32Value();
	unsigned int bs = args[5]->Int32Value();
	unsigned int lt = args[6]->Int32Value();

	Local<Function> data_cb = Local<Function>::Cast(args[7]);
	Local<Function> state_cb = Local<Function>::Cast(args[8]);

	CubebStream *cubebstream = new CubebStream(
		nodecubeb->ctx,
		ToCString(str),
		sf, cc, sr, bs, lt,
		Persistent<Function>::New(data_cb),
		Persistent<Function>::New(state_cb)
	);

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

	Local<Value> argv[2];
//FIXME: this should be an ArrayBuffer referring to the `buffer`
	argv[0] = Integer::New(nframes);
	argv[1] = Integer::New(nframes);

	TryCatch try_catch;

	Local<Value> retval = cs->datacb->Call(Context::GetCurrent()->Global(), 2, argv);

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
		return CUBEB_ERROR;
	}

	return retval->Int32Value();
}

int CubebStream::StateCB(cubeb_stream *stream, void *user, cubeb_state state) {

	struct CubebStream::cb_user_data *u = (struct CubebStream::cb_user_data *)user;

	CubebStream *cs = u->stream;

	Local<Value> argv[1];
	argv[0] = Integer::New(state);

	TryCatch try_catch;

	Local<Value> retval = cs->statecb->Call(Context::GetCurrent()->Global(), 2, argv);

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
		return CUBEB_ERROR;
	}

	return retval->Int32Value();
}

Persistent<FunctionTemplate> CubebStream::constructor_template;
