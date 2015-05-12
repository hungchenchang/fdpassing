#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define PIPE_NAME    ("/tmp/echo.pipe")
#define SERVER_PORT  (7000)

uv_pipe_t *psass_pipe = NULL;

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
	char *base = (char*)req->data;
	free(base);
	free(req);
}

void on_new_connection(uv_stream_t *server, int status) 
{
	if (status == -1) 
	{
		return;
	}

	uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), client);
	if (uv_accept(server, (uv_stream_t*) client) == 0) 
	{
		printf("incoming connect.\n");
		char ip[64];
		uv_write_t *write_req;
		
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		uv_tcp_getsockname(client, (struct sockaddr*)&addr, &addrlen);
		inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));  
		printf("local address:%s:%d.\n", ip, addr.sin_port);
		
		uv_tcp_getpeername(client, (struct sockaddr*)&addr, &addrlen);
		inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));  
		printf("client address:%s:%d.\n", ip, addr.sin_port);

		char string[] = "From middle..";
		int length = strlen(string) + 1;
		write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
		uv_buf_t buf = uv_buf_init((char*)malloc(length), length);
		memcpy(buf.base, string, length);
		write_req->data = (void*)buf.base;
		uv_write(write_req, (uv_stream_t*)client, &buf, 1, echo_write);
		
		write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
		uv_buf_t dummy_buf = uv_buf_init((char*)malloc(1), 1);
		dummy_buf.base[0] = '.';
		write_req->data = (void*)dummy_buf.base;
		uv_write2(write_req, (uv_stream_t*) psass_pipe, &dummy_buf, 1, (uv_stream_t*)client, echo_write);
		
		uv_close((uv_handle_t*) client, on_uv_close);
	}
	else 
	{
		uv_close((uv_handle_t*) client, on_uv_close);
	}
}

int do_listen(short port)
{
	uv_tcp_t *server = malloc(sizeof(uv_tcp_t));
	if (server == NULL) 
	{
		fprintf(stderr, "allocate error!\n");
		return 1;
	}
	uv_tcp_init(uv_default_loop(), server);
	
	struct sockaddr_in bind_addr = uv_ip4_addr("0.0.0.0", port);
	uv_tcp_bind(server, bind_addr);
	int r = uv_listen((uv_stream_t*)server, 128, on_new_connection);
	if (r) 
	{
		fprintf(stderr, "Listen error!\n");
		return 1;
	}
}

void on_connection(uv_connect_t* connection, int status) 
{
	if (status == -1) 
	{
		uv_close((uv_handle_t*)connection->handle, on_uv_close);
		free(connection);
		return;
	}
	printf("pipe connected.\n");
	psass_pipe = (uv_pipe_t *)connection->handle;
	do_listen(SERVER_PORT);
}

int do_connect(const char *pipe_name)
{
	uv_loop_t *loop = uv_default_loop();
	
	uv_pipe_t *client = malloc(sizeof(uv_pipe_t));
	if (client == NULL) 
	{
		fprintf(stderr, "allocate error!\n");
		return 1;
	}
	uv_pipe_init(uv_default_loop(), client, 1);
	printf("connecting...\n");
	
	uv_connect_t* connect_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
	uv_pipe_connect(connect_req, (uv_pipe_t*)client, pipe_name, on_connection);
	return 0;
}

int main() 
{
	do_connect(PIPE_NAME);
	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
