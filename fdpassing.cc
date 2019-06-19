#include <nan.h>

using namespace v8;

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
