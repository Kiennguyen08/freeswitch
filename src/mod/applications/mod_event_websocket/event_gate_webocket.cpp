#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

int main()
{
	server ws_server;
	boost::asio::io_service io_service;
	tcp::socket socket(io_service);

	try {
		// Connect to the FreeSWITCH event socket
		socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8021);

        ws_server.init_asio();
        ws_server.set_message_handler([&socket](websocketpp::connection_hdl hdl, server::message_ptr msg) {
			// Handle incoming WebSocket messages from clients.

			// Forward WebSocket messages to mod_event_socket
			std::string message = msg->get_payload();
			socket.send(boost::asio::buffer(message));
        });

        // Start the WebSocket server

        // Create a thread to listen for events from mod_event_socket and forward them to WebSocket clients.
        // You can use a separate thread to avoid blocking the WebSocket server.
        std::thread([&socket, &ws_server]() {
			char response[1024];
			while (true) {
				size_t length = socket.read_some(boost::asio::buffer(response, sizeof(response));
                std::string event(response, length);

                // Forward events to WebSocket clients
                ws_server.send_all(event);
			}
        }).detach();

        ws_server.listen(9002);  // WebSocket server listens on port 9002
        ws_server.start_accept();

        ws_server.run();
	} catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
