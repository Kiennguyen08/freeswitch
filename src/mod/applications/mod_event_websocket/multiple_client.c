#include <libwebsockets.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the libwebsockets callback functions
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

static struct lws_protocols protocols[] = {{
											   "http-only",
											   callback_http,
											   0,
											   0,
										   },
										   {
											   "websocket",
											   callback_websocket,
											   0,
											   0,
										   },
										   {NULL, NULL, 0, 0}};

struct WebSocketClient {
	struct lws *wsi;
	char client_id[64]; // Unique identifier for the WebSocket client
	struct WebSocketClient *next;
};

struct WebSocketClient *clients = NULL;

// Function to add a WebSocket client to the list
void add_client(struct lws *wsi, const char *client_id)
{
	struct WebSocketClient *new_client = (struct WebSocketClient *)malloc(sizeof(struct WebSocketClient));
	new_client->wsi = wsi;
	strncpy(new_client->client_id, client_id, sizeof(new_client->client_id) - 1);
	new_client->client_id[sizeof(new_client->client_id) - 1] = '\0';
	new_client->next = clients;
	clients = new_client;
}

// Function to remove a WebSocket client from the list
void remove_client(struct lws *wsi)
{
	struct WebSocketClient **current = &clients;

	while (*current != NULL) {
		if ((*current)->wsi == wsi) {
			struct WebSocketClient *temp = *current;
			*current = (*current)->next;
			free(temp);
			break;
		} else {
			current = &(*current)->next;
		}
	}
}

// Function to forward data to a specific WebSocket client
void forward_to_client(const char *client_id, const char *data)
{
	struct WebSocketClient *current = clients;
	while (current != NULL) {
		if (strcmp(current->client_id, client_id) == 0) {
			lws_write(current->wsi, (unsigned char *)data, strlen(data), LWS_WRITE_TEXT);
			break; // Forwarded data to the specific client, exit loop
		}
		current = current->next;
	}
}

// ...

// Modify the callback_websocket function to manage client IDs
static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	char client_id[64];
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		// Handle new WebSocket connections
		// Extract the client ID from the connection request (if available)
		lws_hdr_copy(wsi, client_id, sizeof(client_id), WSI_TOKEN_GET_URI);
		add_client(wsi, client_id);
		break;
	case LWS_CALLBACK_RECEIVE:
		// Handle incoming WebSocket messages
		// Forward messages to a specific client using the client ID
		// For example: forward_to_client("client123", (const char *)in);
		break;
	case LWS_CALLBACK_CLOSED:
		// Handle WebSocket client disconnections
		remove_client(wsi);
		break;
	default:
		break;
	}

	return 0;
}
