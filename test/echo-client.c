#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define SERVER_ADDR  ("172.16.3.130")//("127.0.0.1")
#define SERVER_PORT  (7000)

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
		uv_timer_t *timer = (uv_timer_t*)client->data;
		if (timer)
		{
			uv_timer_stop(timer);
			uv_close((uv_handle_t *)timer, on_uv_close);
		}
		return;
	}
	if (nread > 0)
	{
		printf("echo: [%s]\n", buf.base);
	}
	
	if (buf.base != NULL)
		free(buf.base);
}

void timeout_cb(uv_timer_t* timer, int status)
{
	static int tick = 0;
	char string[64];
	
	sprintf(string,"Hello world %d", tick++);
	int length = strlen(string) + 1;
	
	uv_stream_t *client = (uv_stream_t*)timer->data;
	uv_write_t *write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
	uv_buf_t buf = uv_buf_init((char*)malloc(length), length);
	memcpy(buf.base, string, length);
	
	write_req->data = (void*)buf.base;
	uv_write(write_req, client, &buf, 1, echo_write);
}

void on_connection(uv_connect_t* connection, int status) 
{
	if (status == -1) 
	{
		uv_close((uv_handle_t*)connection->handle, on_uv_close);
		free(connection);
		return;
	}
	uv_stream_t *client = connection->handle;
	uv_read_start(client, alloc_buffer, echo_read);
	
	uv_timer_t* timer = (uv_timer_t*)malloc(sizeof(uv_timer_t));
	uv_timer_init(uv_default_loop(), timer);
	timer->data = client;
	client->data = timer;
	uv_timer_start(timer, (uv_timer_cb)timeout_cb, 1000, 1000);
	
	printf("connected.\n");
	char ip[64];
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	uv_tcp_getsockname((uv_tcp_t *)client, (struct sockaddr*)&addr, &addrlen);
	inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));  
	printf("local address:%s:%d.\n", ip, addr.sin_port);
	
	uv_tcp_getpeername((uv_tcp_t *)client, (struct sockaddr*)&addr, &addrlen);
	inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));  
	printf("server address:%s:%d.\n", ip, addr.sin_port);
}

int do_connect(const char *ipdaar, short port)
{
	uv_loop_t *loop = uv_default_loop();
	
	uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
	if (client == NULL) 
	{
		fprintf(stderr, "allocate error!\n");
		return 1;
	}
	uv_tcp_init(uv_default_loop(), client);
	printf("connecting...\n");
	
	struct sockaddr_in server_addr = uv_ip4_addr(ipdaar, port);
	uv_connect_t* connect_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
	int r = uv_tcp_connect(connect_req, client, server_addr, on_connection);
	if (r) 
	{
		fprintf(stderr, "Connect error!\n");
		return 1;
	}
}

int main() 
{
	do_connect(SERVER_ADDR, SERVER_PORT);
	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
