#ifndef NODE_CUBEB_COMMON_H
#define NODE_CUBEB_COMMON_H

#define DEFINE_V8_CONST(target, name, value, type) (target)->Set(v8::String::NewSymbol(name),\
	v8::type::New(value), static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete))


const char* ToCString (const v8::String::Utf8Value& value);

#endif
