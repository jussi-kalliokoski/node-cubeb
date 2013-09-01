#include <cstdlib>
#include <node.h>
#include <node_buffer.h>
#include <uv.h>
#include <cubeb/cubeb.h>
#include <string.h>

#include "stream.h"
#include "context.h"
#include "common.h"

using namespace v8;
using namespace node;

CubebStream::CubebStream (CubebContext *cctx, const char *nname, cubeb_sample_format sf,
		unsigned int cc, unsigned int sr, unsigned int bs, unsigned int lt,
		Persistent<Function> ddatacb, Persistent<Function> sstatecb) :
ctx(cctx),
stream(NULL),
name(nname),
sampleFormat(sf),
channelCount(cc),
sampleRate(sr),
bufferSize(bs),
latency(lt),
state(CUBEB_STATE_ERROR),
statecb(sstatecb),
datacb(ddatacb),
first_buffer(NULL),
last_buffer(NULL),
active(0) {
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

	error_code = cubeb_stream_init(ctx->ctx, &stream, name, params, latency, DataCB, StateCB, user_data);

	ctx->addStream();
}

CubebStream::~CubebStream () {
	stop();

	if (stream != NULL) {
		cubeb_stream_destroy(stream);
	}

	if (user_data != NULL) {
		free(user_data);
	}

	release();

	datacb.Dispose();
	statecb.Dispose();

	ctx->removeStream();
}

int CubebStream::stop () {
	if (stream == NULL || !active || state == CUBEB_STATE_ERROR) return CUBEB_OK;

	int r = cubeb_stream_stop(stream);
	Unref();

	active = 0;

	return r;
}

int CubebStream::start () {
	if (stream == NULL || active) return CUBEB_OK;

	int r = cubeb_stream_start(stream);
	Ref();

	active = 1;

	return r;
}

void CubebStream::release () {
	cs_buffer *b = first_buffer;

	while (b != NULL && !b->length) {
		cs_buffer *bb = b;
		b = b->next;

		free(bb);
	}

	first_buffer = b;

	if (b == NULL) last_buffer = NULL;
}

void CubebStream::requestFrames (long amount, long bufferStatus) {
	check_malloc (req, cs_work_req) {
		fprintf(stderr, "FATAL ERROR: CubebStream work type allocation failed.\n");
		return;
	}

	check_malloc (user_data, cs_datacb_userdata) {
		fprintf(stderr, "FATAL ERROR: CubebStream userdata allocation failed.\n");
		return;
	}

	user_data->nframes = amount;
	user_data->buffer_status = bufferStatus;

	req->stream = this;
	req->user_data = (void*) user_data;
	req->type = kDataCallback;

	uv_queue_work(uv_default_loop(), &req->w, DoWork, AfterWork);
}

void CubebStream::Initialize (Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
	constructor_template->SetClassName(String::NewSymbol("CubebStream"));

	NODE_SET_PROTOTYPE_METHOD(constructor_template, "stop", Stop);
	NODE_SET_PROTOTYPE_METHOD(constructor_template, "start", Start);
	NODE_SET_PROTOTYPE_METHOD(constructor_template, "write", Write);
	NODE_SET_PROTOTYPE_METHOD(constructor_template, "release", Release);
	SET_V8_PROTOTYPE_GETTER(constructor_template, "state", GetState);
	SET_V8_PROTOTYPE_GETTER(constructor_template, "position", GetPosition);
	target->Set(String::NewSymbol("Stream"), constructor_template->GetFunction());
}

Handle<Value> CubebStream::New (const Arguments &args) {
	HandleScope scope;

	CubebContext *nodecubeb = ObjectWrap::Unwrap<CubebContext>(args[0]->ToObject());

	if (!args[1]->IsString()) {
		return ThrowException(Exception::TypeError(String::New("The name must be a string")));
	}

	String::Utf8Value str(args[1]);

	unsigned int ssf = args[2]->Int32Value();
	unsigned int cc = args[3]->Int32Value();
	unsigned int sr = args[4]->Int32Value();
	unsigned int bs = args[5]->Int32Value();
	unsigned int lt = args[6]->Int32Value();

	cubeb_sample_format sf;

/* Crappy hack to avoid casting to an enum */
	switch (ssf) {
	case CUBEB_SAMPLE_S16LE:
		sf = CUBEB_SAMPLE_S16LE; break;
	case CUBEB_SAMPLE_S16BE:
		sf = CUBEB_SAMPLE_S16BE; break;
	case CUBEB_SAMPLE_FLOAT32BE:
		sf = CUBEB_SAMPLE_FLOAT32BE; break;
	case CUBEB_SAMPLE_FLOAT32LE:
		sf = CUBEB_SAMPLE_FLOAT32LE; break;
	default:
		return ThrowException(Exception::Error(String::New("Invalid sample format")));
	}

	Local<Function> data_cb = Local<Function>::Cast(args[7]);
	Local<Function> state_cb = Local<Function>::Cast(args[8]);

	CubebStream *cubebstream = new CubebStream(
		nodecubeb,
		ToCString(str),
		sf, cc, sr, bs, lt,
		Persistent<Function>::New(data_cb),
		Persistent<Function>::New(state_cb)
	);

	if (cubebstream->error_code != CUBEB_OK) {
		return ThrowException(Exception::ReferenceError(String::New("Error initializing stream")));
	}

	DEFINE_V8_CONST(args.This(), "name", cubebstream->name, String);
	DEFINE_V8_CONST(args.This(), "sampleFormat", cubebstream->sampleFormat, Integer);
	DEFINE_V8_CONST(args.This(), "channelCount", cubebstream->channelCount, Integer);
	DEFINE_V8_CONST(args.This(), "sampleRate", cubebstream->sampleRate, Integer);
	DEFINE_V8_CONST(args.This(), "bufferSize", cubebstream->bufferSize, Integer);
	DEFINE_V8_CONST(args.This(), "latency", cubebstream->latency, Integer);

	cubebstream->Wrap(args.This());

	return args.This();
}

Handle<Value> CubebStream::GetState (Local<String> property, const AccessorInfo& info) {
	Local<Object> self = info.Holder();

	CubebStream *cs = ObjectWrap::Unwrap<CubebStream>(self);

	return Integer::New(cs->state);
}

Handle<Value> CubebStream::GetPosition (Local<String> property, const AccessorInfo& info) {
	Local<Object> self = info.Holder();

	CubebStream *cs = ObjectWrap::Unwrap<CubebStream>(self);

	uint64_t position[0];

	if (cubeb_stream_get_position(cs->stream, position) != CUBEB_OK) {
		position[0] = 0;
	}

	return Integer::New(position[0]);
}

Handle<Value> CubebStream::Stop (const Arguments &args) {
	HandleScope scope;

	CubebStream *cs = ObjectWrap::Unwrap<CubebStream>(args.This());

	if (cs->stop() != CUBEB_OK) {
		return ThrowException(Exception::Error(String::New("Could not stop the stream")));
	}

	return Undefined();
}

Handle<Value> CubebStream::Start (const Arguments &args) {
	HandleScope scope;

	CubebStream *cs = ObjectWrap::Unwrap<CubebStream>(args.This());

	if (cs->start() != CUBEB_OK) {
		return ThrowException(Exception::Error(String::New("Could not start the stream")));
	}

	return Undefined();
}

Handle<Value> CubebStream::Write (const Arguments &args) {
	HandleScope scope;

	if (!node::Buffer::HasInstance(args[0])) {
		return ThrowException(Exception::Error(
			String::New("Invalid arguments.")));
	}

	CubebStream *cs = ObjectWrap::Unwrap<CubebStream>(args.This());
	Local<Object> buf = args[0]->ToObject();

	check_malloc (csbuf, cs_buffer) {
		return ThrowException(Exception::Error(String::New("Could not allocate memory.")));
	}

	csbuf->length = node::Buffer::Length(buf);
	csbuf->buffer = (char*) malloc(csbuf->length);

	memcpy(csbuf->buffer, node::Buffer::Data(buf), csbuf->length);

	if (cs->last_buffer == NULL) {
		cs->first_buffer = csbuf;
		cs->last_buffer = csbuf;
	} else {
		cs->last_buffer->next = csbuf;
		cs->last_buffer = csbuf;
	}

	return Undefined();
}

Handle<Value> CubebStream::Release (const Arguments &args) {
	HandleScope scope;

	CubebStream *cs = ObjectWrap::Unwrap<CubebStream>(args.This());

	cs->release();

	return Undefined();
}

void CubebStream::UnrefBufferCB (char *data, void *hint) {}

long get_frame_size (CubebStream *cs) {
	int n = cs->channelCount;

	switch (cs->sampleFormat) {
	case CUBEB_SAMPLE_S16LE:
	case CUBEB_SAMPLE_S16BE:
		return 2 * n;
	case CUBEB_SAMPLE_FLOAT32LE:
	case CUBEB_SAMPLE_FLOAT32BE:
		return 4 * n;
	}

	return 1 * n;
}

long CubebStream::DataCB (cubeb_stream *stream, void *user, void *buffer, long nframes) {
	cb_user_data *u = (cb_user_data *)user;
	CubebStream *cs = u->stream;
	long size = get_frame_size(cs) * nframes;
	long n = 0;
	char *outbuf = (char *)buffer;

	cs_buffer *b = cs->first_buffer;

	while (b != NULL && n < size) {
		long c = b->length;

		if (!c) goto next;

		if (size - n < c) c = size - n;

		memcpy(outbuf, b->buffer, c);

		n += c;
		outbuf += c;
		b->buffer += c;
		b->length -= c;

		next: b = b->next;
	}

	long bufferStatus = (n - size) / get_frame_size(cs) - nframes;

	if (!cs->bufferSize) {
		cs->requestFrames(nframes, bufferStatus);
	} else {
		long remaining = -bufferStatus;

		while (remaining > 0) {
			cs->requestFrames(cs->bufferSize, bufferStatus);
			remaining -= cs->bufferSize;
		}
	}

	return nframes;
}

void CubebStream::StateCB (cubeb_stream *stream, void *user, cubeb_state state) {
	cb_user_data *u = (cb_user_data *)user;
	CubebStream *cs = u->stream;

	check_malloc (req, cs_work_req) {
		fprintf(stderr, "FATAL ERROR: CubebStream work type allocation failed.\n");
		return;
	}

	check_malloc (user_data, cs_statecb_userdata) {
		fprintf(stderr, "FATAL ERROR: CubebStream userdata allocation failed.\n");
		return;
	}

	cs->state = state;
	user_data->state = state;

	req->stream = cs;
	req->user_data = (void*) user_data;
	req->type = kStateCallback;

	uv_queue_work(uv_default_loop(), &req->w, DoWork, AfterWork);

	return;
}

void CubebStream::DoWork (uv_work_t *work) {
}

void CubebStream::AfterWork (uv_work_t *work, int status) {
	cs_work_req *u = container_of(work, cs_work_req, w);

	CubebStream *cs = u->stream;

	HandleScope scope;

	TryCatch try_catch;

	Handle<Value> argv[1];

	cs_statecb_userdata *uscd;
	cs_datacb_userdata *usdd;

	switch (u->type) {
	case kStateCallback:
		uscd = (cs_statecb_userdata *) u->user_data;

		argv[0] = Number::New(uscd->state);
		cs->statecb->Call(cs->handle_, 1, argv);
		break;
	case kDataCallback:
		usdd = (cs_datacb_userdata *) u->user_data;

		argv[0] = Number::New(usdd->nframes);
		cs->datacb->Call(cs->handle_, 1, argv);
		break;
	default:
		/* Something went wrong. Oh well... */
		return;
	}

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
		return;
	}
}

Persistent<FunctionTemplate> CubebStream::constructor_template;
