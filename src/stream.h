#ifndef NODE_CUBEB_STREAM_H
#define NODE_CUBEB_STREAM_H

#include <node.h>
#include <node_buffer.h>
#include <cubeb/cubeb.h>
#include <uv.h>

#include "context.h"

class CubebStream : public node::ObjectWrap {
	static v8::Persistent<v8::FunctionTemplate> constructor_template;
public:
	struct cb_user_data {
		CubebStream *stream;
	};

	enum cs_work_type {
		kStateCallback,
		kDataCallback
	};

	struct cs_work_req {
		CubebStream *stream;
		uv_work_t w;
		enum cs_work_type type;
		void *user_data;
	};

	struct cs_statecb_userdata {
		cubeb_state state;
	};

	struct cs_datacb_userdata {
		long nframes;
		long buffer_status;
	};

	struct cs_buffer {
		char *buffer;
		long length;
		cs_buffer *next;
	};

	CubebContext *ctx;
	cubeb_stream *stream;

	char const *name;
	cubeb_sample_format sampleFormat;
	unsigned int channelCount;
	unsigned int sampleRate;
	unsigned int bufferSize;
	unsigned int latency;
	cubeb_state state;

	v8::Persistent<v8::Function> statecb;
	v8::Persistent<v8::Function> datacb;

	int error_code;

	struct cb_user_data *user_data;
	struct cs_buffer *first_buffer;
	struct cs_buffer *last_buffer;
	int active;

	CubebStream(CubebContext *cctx, const char *nname, cubeb_sample_format sf, unsigned int cc, unsigned int sr, unsigned int bs, unsigned int lt,
		v8::Persistent<v8::Function> ddatacb, v8::Persistent<v8::Function> sstatecb);
	~CubebStream();

	int stop();
	int start();
	void release();
	void requestFrames(long amount, long bufferStatus);

	static void Initialize (v8::Handle<v8::Object> target);

	static v8::Handle<v8::Value> New (const v8::Arguments &args);
	static v8::Handle<v8::Value> Start (const v8::Arguments &args);
	static v8::Handle<v8::Value> Stop (const v8::Arguments &args);
	static v8::Handle<v8::Value> Write (const v8::Arguments &args);
	static v8::Handle<v8::Value> Release (const v8::Arguments &args);

	static v8::Handle<v8::Value> GetState (v8::Local<v8::String> property, const v8::AccessorInfo &info);
	static v8::Handle<v8::Value> GetPosition (v8::Local<v8::String> property, const v8::AccessorInfo &info);

	static void UnrefBufferCB(char *data, void *hint);
	static long DataCB(cubeb_stream *stream, void *user, void *buffer, long nframes);
	static void StateCB(cubeb_stream *stream, void *user, cubeb_state state);

	static void DoWork (uv_work_t *work);
	static void AfterWork (uv_work_t *work, int status);
};

#endif
