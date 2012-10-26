#ifndef NODE_CUBEB_H
#define NODE_CUBEB_H

#include <node.h>
#include <node_buffer.h>
#include <cubeb/cubeb.h>

class CubebContext : public node::ObjectWrap {
	static v8::Persistent<v8::FunctionTemplate> constructor_template;

public:
	cubeb *ctx;
	char const *name;
	int active_stream_count;
	int error_code;

	CubebContext (const char *nname);
	~CubebContext();

	static void Initialize (v8::Handle<v8::Object> target);

	static v8::Handle<v8::Value> New (const v8::Arguments &args);

	void addStream();
	void removeStream();
};

#endif
