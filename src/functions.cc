#include <nan.h>
#include <iostream>
#include <sys/msg.h>

using v8::Function;
using v8::Local;
using v8::Number;
using v8::Value;
using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::New;
using Nan::Null;

#ifndef MSGMAX
#define MSGMAX 4056
#endif

#ifndef linux
struct msgbuf {
  long mtype;
  char mtext[1];
};
#endif

const size_t bsize = sizeof(struct msgbuf);

const char *ConcatString(std::string one, const char *two) {
  return one.append(two).c_str();
}

Local<Value> CreateError(std::string message, int errcode) {
  message.append(": ");

  Nan::MaybeLocal<Value> val = Nan::Error(ConcatString(message, strerror(errcode)));

  return val.ToLocalChecked();
}

class SendMessageWorker : public AsyncWorker {

  public:
    SendMessageWorker(Callback *callback, int id, char *data, size_t dataLength, long type, int flags)
      : AsyncWorker(callback), id(id), data(data), dataLength(dataLength), type(type), flags(flags) { }
    ~SendMessageWorker() { }

    void Execute() {
      struct msgbuf* message =
        (struct msgbuf*)malloc(bsize + dataLength);

      message->mtype = type;
      memcpy(message->mtext, data, dataLength);

      ret = msgsnd(id, message, dataLength, flags);
      error = errno;

      free(message);
    }

    void HandleOKCallback () {
      Local<Value> argv[] = { Null() };

      if (ret == -1) {
        argv[0] = CreateError("Failed to send message", error);
      }

      callback->Call(1, argv);
    }

  private:
    int id;
    char *data;
    size_t dataLength;
    long type;
    int flags;
    int ret;
    int error;

};

class ReceiveMessageWorker : public AsyncWorker {

  public:
    ReceiveMessageWorker(Callback *callback, int id, size_t bufferLength, long type, int flags)
      : AsyncWorker(callback), id(id), bufferLength(bufferLength), type(type), flags(flags) { }
    ~ReceiveMessageWorker() { }

    void Execute() {
      message =
        (struct msgbuf*) malloc(bsize + bufferLength);

      bufferLength = msgrcv(id, message, bufferLength, type, flags);
      error = errno;
    }

    void HandleOKCallback () {
      Local<Value> argv[] = {
        Null(),
        Null()
      };

      if (bufferLength == (size_t) -1) {
        argv[0] = CreateError("Failed to receive message", error);
      } else {
        argv[1] = Nan::CopyBuffer(message->mtext, bufferLength).ToLocalChecked();
      }

      free(message);

      callback->Call(2, argv);
    }

  private:
    int id;
    struct msgbuf* message;
    size_t bufferLength;
    long type;
    int flags;
    int error;
};

NAN_METHOD(GetMessageQueue) {
  key_t key = (key_t) info[0]->Int32Value();
  int flags = info[1]->Int32Value();

  int queue = msgget(key, flags);

  if (queue == -1) {
    Nan::ThrowError(CreateError("Failed to get queue", errno));
  }

  info.GetReturnValue().Set(New(queue));
}

NAN_METHOD(ControlMessageQueue) {
  int id = info[0]->Int32Value();
  int cmd = info[1]->Int32Value();

  msqid_ds *buf;

  if (cmd != IPC_STAT && cmd != IPC_SET) {
    buf = nullptr;
  }
#ifdef linux
  else if (cmd != MSG_STAT) {
    buf = nullptr;
  }
#endif
  else {
    buf = new msqid_ds;
  }

  int ret = msgctl(id, cmd, buf);

  size_t bufSize = sizeof(msqid_ds);
  char rawBuf[bufSize];
  memcpy(rawBuf, buf, bufSize);

  if (ret == 0) {
    info.GetReturnValue().Set(Nan::NewBuffer(rawBuf, bufSize).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(New(ret));
  }
}

NAN_METHOD(CloseMessageQueue) {
  int id = info[0]->Int32Value();

  int ret = msgctl(id, IPC_RMID, nullptr);

  if (ret != 0) {
    Nan::ThrowError(CreateError("Failed to close queue", errno));
  }

  info.GetReturnValue().Set(Nan::True());
}

NAN_METHOD(SendMessage) {
  int id = info[0]->Int32Value();
  char* bufferData = node::Buffer::Data(info[1]);
  size_t bufferLength = (size_t)  node::Buffer::Length(info[1]);
  long type = (long) info[2]->Int32Value();
  int flags = info[3]->Int32Value();
  Callback *callback = new Callback(info[4].As<Function>());

  AsyncQueueWorker(new SendMessageWorker(callback, id, bufferData, bufferLength, type, flags));
}

NAN_METHOD(ReceiveMessage) {
  int id = info[0]->Int32Value();
  size_t bufferLength = (size_t) info[1]->Int32Value();
  long type = (long) info[2]->Int32Value();
  int flags = info[3]->Int32Value();
  Callback *callback = new Callback(info[4].As<Function>());

  AsyncQueueWorker(new ReceiveMessageWorker(callback, id, bufferLength, type, flags));
}
