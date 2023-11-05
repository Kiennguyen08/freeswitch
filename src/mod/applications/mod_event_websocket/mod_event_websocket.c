/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2014, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Anthony Minessale II <anthm@freeswitch.org>
 * Neal Horman <neal at wanlink dot com>
 *
 *
 * mod_event_websocket.c -- Framework Demo Module
 *
 */
#include <arpa/inet.h>
#include <libwebsockets.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <sys/socket.h>
#include <unistd.h>

#define EXAMPLE_RX_BUFFER_BYTES (100)
#define EVENT_SOCKET_HOST "127.0.0.1"
#define EVENT_SOCKET_PORT 8021

/* Prototypes */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_event_websocket_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_event_websocket_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_event_websocket_load);

/* SWITCH_MODULE_DEFINITION(name, load, shutdown, runtime)
 * Defines a switch_loadable_module_function_table_t and a static const char[] modname
 */
SWITCH_MODULE_DEFINITION(mod_event_websocket, mod_event_websocket_load, mod_event_websocket_shutdown,
						 mod_event_websocket_runtime);

// #define _switch_stun_packet_next_attribute(attribute, end) (attribute && (attribute = (switch_stun_packet_attribute_t
// *) (attribute->value +  _switch_stun_attribute_padded_length(attribute))) && ((void *)attribute < end) && ((void
// *)(attribute +  _switch_stun_attribute_padded_length(attribute)) < end))
SWITCH_STANDARD_API(websocket_function) { return SWITCH_STATUS_SUCCESS; }

struct payload {
	unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + EXAMPLE_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
	size_t len;
} received_payload;

enum protocols { PROTOCOL_HTTP = 0, PROTOCOL_EXAMPLE, PROTOCOL_COUNT };

struct WebSocketClient {
	struct lws *wsi;
	char client_id[64]; // Unique identifier for the WebSocket client
	struct WebSocketClient *next;
};

struct WebSocketClient *clients = NULL;

static int event_socket = -1;

struct lws_context *context;

// Function to send a command to the event socket
void send_command(int socket, const char *command)
{
	// Send the command to the event socket
	write(socket, command, strlen(command));
}

// Function to receive data from the event socket
int receive_data(int socket, char *buffer, int buffer_size) { return read(socket, buffer, buffer_size); }

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

void forward_to_all_clients(const char *data)
{
	struct WebSocketClient *current = clients;
	while (current != NULL) {
		lws_write(current->wsi, (unsigned char *)data, strlen(data), LWS_WRITE_TEXT);
		current = current->next;
	}
}

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	return 0;
}

static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	char client_id[64];
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		// Handle new WebSocket connections
		// Extract the client ID from the connection request (if available)
		lws_hdr_copy(wsi, client_id, sizeof(client_id), WSI_TOKEN_GET_URI);
		add_client(wsi, client_id);
		printf("WebSocket connection established with client_id %s\n", client_id);
		break;
	case LWS_CALLBACK_RECEIVE:
		// Handle incoming WebSocket messages
		// Forward messages to a specific client using the client ID
		// For example: forward_to_client("client123", (const char *)in);
		perror("Received websocket message");
		fprintf(stdout, "Bytes received from Websocket message %.*s\n", (int)len, (char *)in);
		memcpy(&received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], in, len);
		received_payload.len = len;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Len Receive Data [%lu]", len);
		send_command(event_socket, (char *)&received_payload.data);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		lws_write(wsi, &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], received_payload.len, LWS_WRITE_TEXT);
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

static struct lws_protocols protocols[] = {
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,			   /* No per session data. */
		0,			   /* max frame size / rx buffer */
	},
	{
		"example-protocol",
		callback_websocket,
		0,
		0,
	},
	{NULL, NULL, 0, 0} /* terminator */
};

static void *event_socket_thread(void *arg)
{
	// Connect to the FreeSWITCH event socket
	if (event_socket == -1) {
		perror("Event socket socket() failed");
		return NULL;
	}

	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(EVENT_SOCKET_HOST);
	server.sin_family = AF_INET;
	server.sin_port = htons(EVENT_SOCKET_PORT);

	if (connect(event_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Event socket connect() failed");
		close(event_socket);
		return NULL;
	}

	// Handle communication with the event socket
	char buffer[1024];
	while (1) {
		// Your WebSocket server can send messages to the event socket using send_command

		// Receive data from the event socket
		int bytes_received = receive_data(event_socket, buffer, sizeof(buffer));
		fprintf(stdout, "Bytes received from TCP Payload %d\n", bytes_received);
		if (bytes_received > 0) {
			// perror("Event socket receive_data() failed");

			// Process and forward the received data to WebSocket clients
			// Here, you should send the data over WebSocket to the connected clients
			forward_to_all_clients(buffer);
		}
	}

	// Close the event socket connection
	close(event_socket);

	return NULL;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_event_websocket_load)
{
	switch_api_interface_t *api_interface;
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Hello World Kiennt Websocket!\n");
	SWITCH_ADD_API(api_interface, "websocket", "Websocket API", websocket_function, "syntax");
	// Initialize the socket
	event_socket = socket(AF_INET, SOCK_STREAM, 0);
	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_event_websocket_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_event_websocket_shutdown)
{
	/* Cleanup dynamically allocated config settings */
	return SWITCH_STATUS_SUCCESS;
}

/*
  If it exists, this is called in it's own thread when the module-load completes
  If it returns anything but SWITCH_STATUS_TERM it will be called again automatically
  Macro expands to: switch_status_t mod_event_websocket_runtime()
*/
SWITCH_MODULE_RUNTIME_FUNCTION(mod_event_websocket_runtime)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Runtime Event Websocket...\n");

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	// Set up the libwebsockets context
	info.port = 9002; // Port on which the WebSocket server will listen
	info.protocols = protocols;

	context = lws_create_context(&info);
	if (!context) {
		fprintf(stderr, "Error creating libwebsockets context\n");
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Error creating libwebsockets context\n");
		return 1;
	}
	fprintf(stdout, "Created libwebsockets context\n");
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Created libwebsockets context\n");

	pthread_t event_socket_thread_id;
	pthread_create(&event_socket_thread_id, NULL, event_socket_thread, NULL);

	while (1) {
		lws_service(context, 50); // 50 ms timeout for handling WebSocket connections
	}

	lws_context_destroy(context);

	return SWITCH_STATUS_TERM;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet
 */
