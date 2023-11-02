#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Define the event socket connection information
#define EVENT_SOCKET_HOST "127.0.0.1"
#define EVENT_SOCKET_PORT 8021
static struct lws *wsi_global;

// Function to send a command to the event socket
void send_command(int socket, const char *command)
{
	// Send the command to the event socket
	write(socket, command, strlen(command));
}

// Function to receive data from the event socket
int receive_data(int socket, char *buffer, int buffer_size) { return read(socket, buffer, buffer_size); }

static void *event_socket_thread(void *arg)
{
	// Connect to the FreeSWITCH event socket
	int event_socket = socket(AF_INET, SOCK_STREAM, 0);
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
		// For example:
		// send_command(event_socket, "api status\n");

		// Receive data from the event socket
		int bytes_received = receive_data(event_socket, buffer, sizeof(buffer));
		if (bytes_received <= 0) {
			perror("Event socket receive_data() failed");
			break; // Handle disconnection or errors
		}

		// Process and forward the received data to WebSocket clients
		// Here, you should send the data over WebSocket to the connected clients
		if (wsi_global) {
			// Data received from the event socket
			char *data = buffer;
			size_t data_len = bytes_received;
			// Forward the data to the WebSocket client
			lws_write(wsi_global, (unsigned char *)data, data_len, LWS_WRITE_TEXT);
		}
	}

	// Close the event socket connection
	close(event_socket);

	return NULL;
}
