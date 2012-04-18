#ifndef NODE_CUBEB_COMMON_H
#define NODE_CUBEB_COMMON_H

#define DEFINE_V8_CONST(target, name, value, type) (target)->Set(v8::String::NewSymbol(name),\
	v8::type::New(value), static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete))

#define SET_V8_PROTOTYPE_GETTER(templ, name, callback)\
	templ->InstanceTemplate()->SetAccessor(v8::String::NewSymbol(name), callback)

#define check_malloc(varname, vartype)\
	vartype *varname;\
	varname = (vartype*) malloc(sizeof(*varname));\
	if (varname == NULL)

#define offset_of(type, member) \
	((intptr_t) ((char *) &(((type *) 8)->member) - 8))

#define container_of(ptr, type, member) \
	((type *) ((char *) (ptr) - offset_of(type, member)))

const char* ToCString (const v8::String::Utf8Value& value);

#endif
