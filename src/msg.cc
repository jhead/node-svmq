#include <nan.h>
#include "functions.h"

using v8::FunctionTemplate;

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("get").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetMessageQueue)).ToLocalChecked());

  Nan::Set(target, Nan::New("snd").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(SendMessage)).ToLocalChecked());

  Nan::Set(target, Nan::New("rcv").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ReceiveMessage)).ToLocalChecked());

  Nan::Set(target, Nan::New("ctl").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ControlMessageQueue)).ToLocalChecked());

  Nan::Set(target, Nan::New("close").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(CloseMessageQueue)).ToLocalChecked());

}

NODE_MODULE(msg, Init)
