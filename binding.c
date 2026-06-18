#include <bare.h>
#include <bare/module.h>
#include <uv.h>

typedef struct {

  	js_env_t *env;

  	uv_tcp_t tcp;
  	uv_connect_t connect;

  	js_deferred_t *deferred;

  	char *request;
  	size_t request_len;

  	char *buffer;
  	size_t length;

  	char *response;
  	size_t response_len;

} tcp_request_t;

static void alloc_buffer( uv_handle_t *handle, size_t suggested, uv_buf_t *buf) 
{
  	buf->base = malloc(suggested);
  	buf->len = suggested;
}

static void on_read( uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) 
{
  	tcp_request_t *req = stream->data;

  	if (nread > 0) 
	{
    	req->response = realloc(req->response, req->response_len + nread);

    	memcpy(req->response + req->response_len, buf->base, nread);
    	req->response_len += nread;
	
    	free(buf->base);
    	return;
  	}

  	if (nread == 0) 
	{
    	free(buf->base);
    	return;
  	}
	
  	js_handle_scope_t *scope;
  	js_open_handle_scope(req->env, &scope);
	
  	js_value_t *result;
	
  	js_create_arraybuffer( req->env, req->response_len, (void **) &req->response, &result);
  	js_resolve_deferred( req->env, req->deferred, result);
  	js_close_handle_scope(req->env, scope);
  	uv_close((uv_handle_t *) stream, NULL);
	
  	free(buf->base);
}

static void on_write( uv_write_t *req, int status) 
{
  	tcp_request_t *ctx = req->data;

  	free(req);

  	if (status < 0) {
    	fprintf(stderr, "write error: %s\n", uv_strerror(status));

    	js_handle_scope_t *scope;
    	js_open_handle_scope(ctx->env, &scope);

    	js_value_t *error;

    	js_create_string_utf8( ctx->env, "write error", strlen("write error"), &error);
    	js_reject_deferred( ctx->env, ctx->deferred, error);
    	js_close_handle_scope(ctx->env, scope);
	
    	return;
  	}

  	ctx->response = NULL;
  	ctx->response_len = 0;
	
  	uv_read_start( (uv_stream_t *) &ctx->tcp, alloc_buffer, on_read);
}

static void on_connect( uv_connect_t *req, int status) 
{
  	tcp_request_t *ctx = req->data;

  	if (status < 0) 
	{
    	fprintf( stderr, "connect error: %s\n", uv_strerror(status));
    	return;
  	}

  	uv_buf_t buf = uv_buf_init( ctx->request, ctx->request_len);
  	uv_write_t *write_req = malloc(sizeof(uv_write_t));
  	write_req->data = ctx;
  	uv_write( write_req, (uv_stream_t *)&ctx->tcp, &buf, 1, on_write);
}


static js_value_t *init( js_env_t *env, js_value_t *exports);
static js_value_t *tcp_cat( js_env_t *env, js_callback_info_t *info) 
{
  	size_t argc = 3;
  	js_value_t *argv[3];

  	js_get_callback_info( env, info, &argc, argv, NULL, NULL);

  	if (argc != 3) 
  	{
    	return NULL;
  	}

  	size_t host_len;
  	js_get_value_string_utf8( env, argv[0], NULL, 0, &host_len);
  	
	char *host = malloc(host_len + 1);
  	js_get_value_string_utf8( env, argv[0], host, host_len + 1, NULL);
  	
	int32_t port;
  	js_get_value_int32( env, argv[1], &port);

  	size_t req_len;
  	js_get_value_string_utf8( env, argv[2], NULL, 0, &req_len);

  	char *request = malloc(req_len + 1);
  	js_get_value_string_utf8( env, argv[2], request, req_len + 1, NULL);

  	tcp_request_t *ctx = calloc( 1, sizeof(tcp_request_t));

  	ctx->env = env; 
  	ctx->request = request;
  	ctx->request_len = req_len;
  	ctx->response = NULL;
  	ctx->response_len = 0;
  	ctx->deferred = NULL;

  	js_value_t *promise;
  	js_create_promise( env, &ctx->deferred, &promise);

  	uv_loop_t *loop;

  	js_get_env_loop( env, &loop);

  	uv_tcp_init( loop, &ctx->tcp);
  	ctx->tcp.data = ctx;

  	struct sockaddr_in addr;
  	uv_ip4_addr( host, port, &addr);

  	ctx->connect.data = ctx;
  	uv_tcp_connect( &ctx->connect, &ctx->tcp, (const struct sockaddr *)&addr, on_connect);

  	return promise;
}


static js_value_t *init( js_env_t *env, js_value_t *exports) 
{
  	js_value_t *fn;

  	js_create_function( env, "tcpCat", 7, tcp_cat, NULL, &fn);
  	js_set_named_property( env, exports, "tcpCat", fn);

  	return exports;
}


BARE_MODULE( tcp_cat, init)
