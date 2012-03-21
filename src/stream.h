#ifndef NODE_CUBEB_STREAM_H
#define NODE_CUBEB_STREAM_H

#include <node.h>
#include <node_buffer.h>
#include <cubeb/cubeb.h>

#include "context.h"

class CubebStream : public node::ObjectWrap {
	static v8::Persistent<v8::FunctionTemplate> constructor_template;
public:
	struct cb_user_data {
		CubebStream *stream;
	};

	v8::Handle<v8::Value> JSObject;

	cubeb *ctx;
	cubeb_stream *stream;

	char const *name;
	unsigned int channelCount;
	unsigned int sampleRate;
	unsigned int bufferSize;
	unsigned int latency;

	int error_code;

	struct cb_user_data *user_data;

	CubebStream(cubeb *cctx, const char *nname, unsigned int cc, unsigned int sr, unsigned int bs, unsigned int lt);
	~CubebStream();

	void stop();

	static void Initialize (v8::Handle<v8::Object> target);

	static v8::Handle<v8::Value> New (const v8::Arguments &args);

	static long DataCB(cubeb_stream *stream, void *user, void *buffer, long nframes);
	static int StateCB(cubeb_stream *stream, void *user, cubeb_state state);
};

#endif
