#include <libwebsockets.h>

// Define a callback for handling WebSocket protocol events
static int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		// A new WebSocket connection has been established
		printf("WebSocket connection established\n");
		break;

	case LWS_CALLBACK_RECEIVE:
		// Received data from a WebSocket client
		// Handle the incoming data here
		printf("Received data from client: %.*s\n", (int)len, (char *)in);
		char response_data[] = "Hello, client!";
		int response_data_length = strlen(response_data);
		lws_write(wsi, response_data, response_data_length, LWS_WRITE_TEXT);
		break;

	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] = {
	{
		"callback-function", callback,
		0, // No per session data
		0, // No options
	},
	{NULL, NULL, 0, 0} // Terminating structure
};

int main(void)
{
	struct lws_context_creation_info info;
	struct lws_context *context;
	const char *iface = NULL;
	int port = 9000;
	int opts = 0;

	memset(&info, 0, sizeof(info));

	info.port = port;
	info.iface = iface;
	info.protocols = protocols;
	// info.extensions = lws_get_internal_extensions();
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;

	context = lws_create_context(&info);

	if (!context) {
		fprintf(stderr, "WebSocket context creation failed\n");
		return -1;
	}

	printf("WebSocket server started on port %d\n", port);

	while (1) { lws_service(context, 50); }

	lws_context_destroy(context);

	return 0;
}
