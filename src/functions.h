#include <nan.h>

NAN_METHOD(GetMessageQueue);
NAN_METHOD(ControlMessageQueue);
NAN_METHOD(CloseMessageQueue);
NAN_METHOD(SendMessage);
NAN_METHOD(ReceiveMessage);

const char *ConcatString(std::string, const char *);
v8::Local<v8::Value> CreateError(std::string, int);
