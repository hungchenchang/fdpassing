#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define PIPE_NAME    ("/tmp/echo.pipe")

void on_uv_close(uv_handle_t* handle)
{
	free(handle);
}

uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size) 
{
	return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void echo_write(uv_write_t *req, int status) 
{
	if (status == -1) 
	{
		//fprintf(stderr, "Write error!\n");
	}
	char *base = (char*) req->data;
	free(base);
	free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf) 
{
	if (nread < 0) 
	{
		fprintf(stderr, "close by peer!\n");
		uv_close((uv_handle_t*)client, on_uv_close);
		return;
	}

	if (nread > 0) 
	{
		int i;
		char *ptr = buf.base;
		for (i = 0; i < nread; i++)
		{
			printf("%02x ", (int)*ptr++);
			if ((i % 32) == 31)
				printf("\n");
		}
		printf("\n");
		//printf("echo [%s]\n", buf.base);
		/*
		uv_write_t *write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
		uv_buf_t buf2 = uv_buf_init((char*)malloc(nread), nread);
		memcpy(buf2.base, buf.base, nread);
		write_req->data = (void*)buf2.base;
		uv_write(write_req, client, &buf2, 1, echo_write);
		*/
	}

	if (buf.base != NULL)
		free(buf.base);
}


void pipe_read(uv_pipe_t* ipc_pipe, ssize_t nread, uv_buf_t buf, uv_handle_type type) 
{
	if (nread < 0) 
	{
		fprintf(stderr, "pipe close by peer!\n");
		uv_close((uv_handle_t*)ipc_pipe, on_uv_close);
		return;
	}

	if (nread > 0) 
	{
		printf("pipe receive %d bytes\n", (int)nread);
		printf("pipe receive %s.\n", (char*)buf.base);
		uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
		uv_tcp_init(uv_default_loop(), (uv_tcp_t*)client);
		if (uv_accept((uv_stream_t*)ipc_pipe, (uv_stream_t*)client) == 0)
		{
			uv_read_start((uv_stream_t*)client, alloc_buffer, echo_read);
			/*
			char string[] = "From server..";
			int length = strlen(string) + 1;
			uv_write_t *write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
			uv_buf_t buf = uv_buf_init((char*)malloc(length), length);
			memcpy(buf.base, string, length);
			write_req->data = (void*)buf.base;
			uv_write(write_req, (uv_stream_t*)client, &buf, 1, echo_write);
			*/
		}
	}
}

void on_new_connection(uv_stream_t *server, int status) 
{
	if (status == -1) 
	{
		return;
	}

	uv_pipe_t *client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
	uv_pipe_init(uv_default_loop(), client, 1);
	if (uv_accept(server, (uv_stream_t*) client) == 0) 
	{
		printf("pipe connected.\n");
		uv_read2_start((uv_stream_t*) client, alloc_buffer, pipe_read);
	}
	else 
	{
		uv_close((uv_handle_t*) client, on_uv_close);
	}
}

int do_listen(char *pipe_name)
{
	uv_pipe_t *server = malloc(sizeof(uv_pipe_t));
	if (server == NULL) 
	{
		fprintf(stderr, "allocate error!\n");
		return 1;
	}
	uv_pipe_init(uv_default_loop(), server, 1);
	uv_pipe_bind(server, pipe_name);
	int r = uv_listen((uv_stream_t*)server, 128, on_new_connection);
	if (r) 
	{
		fprintf(stderr, "Listen error!\n");
		return 1;
	}
}

int main() 
{
	do_listen(PIPE_NAME);
	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
