#include <unistd.h>
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
	close(fd);
	return size;
}


NAN_METHOD(fdTransfer) {
  NanScope();

  if (args.Length() != 3) {
    NanThrowTypeError("Wrong number of arguments");
    NanReturnUndefined();
  }

  if (!args[0]->IsInt32() || !args[2]->IsInt32()) {
    NanThrowTypeError("Wrong arguments");
    NanReturnUndefined();
  }

  int pipe = args[0]->IntegerValue();
  v8::Local<v8::String> str = args[1]->ToString();
  int buflen = str->Utf8Length();
  char *buf = new char[buflen+4];
  str->WriteUtf8(buf, buflen);
  int fd = args[2]->IntegerValue();
  int result = sock_fd_write(pipe, buf, buflen, fd);
  Local<Integer> num = NanNew(result);

  delete [] buf;
  NanReturnValue(num);
}

void Init(Handle<Object> exports) {
  exports->Set(NanNew("fdTransfer"), NanNew<FunctionTemplate>(fdTransfer)->GetFunction());
}

NODE_MODULE(fdpassing, Init)
