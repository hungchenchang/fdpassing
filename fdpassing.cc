#include <sys/socket.h>
#include <stdio.h>

//#define __NAN__

int sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
	ssize_t size;
	struct msghdr   msg;
	struct iovec    iov;
	union 
	{
		struct cmsghdr  cmsghdr;
		char control[CMSG_SPACE(sizeof (int))];
	} cmsgu;
	struct cmsghdr  *cmsg;
	
	iov.iov_base = buf;
	iov.iov_len = buflen;
	
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	
	msg.msg_control = cmsgu.control;
	msg.msg_controllen = sizeof(cmsgu.control);
	
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN(sizeof (int));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	
	printf ("passing fd %d\n", fd);
	*((int *) CMSG_DATA(cmsg)) = fd;
	
	size = sendmsg(sock, &msg, 0);
	return size;
}

#ifdef __NAN__

#include <nan.h>
#include <node.h>
using namespace v8;
using namespace Nan;

void fdTransfer(const Nan::FunctionCallbackInfo<v8::Value>& args)
{
  //printf("fdTransfer is caled args = %d\n", args.Length());

  if (args.Length() != 3) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (!args[0]->IsInt32() || !args[2]->IsInt32()) {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  int pipe = args[0]->IntegerValue();
  v8::Local<v8::String> str = args[1]->ToString();
  int buflen = str->Utf8Length();
  //printf("buflen = %d\n", buflen);
  char *buf = new char[buflen+4];
  str->WriteUtf8(buf, buflen);
  //printf("bu = %s\n", buf);
  int fd = args[2]->IntegerValue();
  int result = sock_fd_write(pipe, buf, buflen, fd);
  v8::Local<v8::Number> num = Nan::New(result);

  delete [] buf;
  args.GetReturnValue().Set(num);
}

void Init(Handle<Object> exports) {
  exports->Set(Nan::New("fdTransfer").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(fdTransfer)->GetFunction());
}

NODE_MODULE(fdpassing, Init)

#else

#define NAPI_VERSION 3
#include <node_api.h>

napi_value fdTransfer(napi_env env, napi_callback_info args) 
{
    napi_status status;

    size_t argc = 3;
    napi_value argv[3];

    status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
    if (status != napi_ok) return NULL;

    int pipe, fd;
    size_t buflen;
    status = napi_get_value_int32(env, argv[0], &pipe);
    if (status != napi_ok) return NULL;
    status  = napi_get_value_int32(env, argv[2], &fd);
    if (status != napi_ok) return NULL;
    status = napi_get_value_string_utf8(env, argv[1], NULL, 0, &buflen);
    if (status != napi_ok) return NULL;
    printf("buflen = %d\n", (int)buflen);
    char *buf = new char[buflen+4];
    status = napi_get_value_string_utf8(env, argv[1], buf, buflen+4, &buflen);
    if (status != napi_ok) return NULL;
    printf("bu = %s\n", buf);

    int result = sock_fd_write(pipe, buf, buflen, fd);
    delete [] buf;

    napi_value retvalue;
    status = napi_create_int64(env, result, &retvalue);
    if (status != napi_ok) return nullptr;
    return retvalue;
}


napi_value Init(napi_env env, napi_value exports) 
{
  napi_status status;
  napi_value fn;

  status = napi_create_function(env, nullptr, 0, fdTransfer, nullptr, &fn);
  if (status != napi_ok) return nullptr;

  status = napi_set_named_property(env, exports, "fdTransfer", fn);
  if (status != napi_ok) return nullptr;
  return exports;
}

NAPI_MODULE(fdpassing, Init)

#endif