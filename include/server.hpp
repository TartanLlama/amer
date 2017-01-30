#ifndef AMER_SERVER_HPP
#define AMER_SERVER_HPP

#include <vector>
#include <experimental/filesystem>

#include "onion/websocket.h"
#include "onion.hpp"

namespace amer {
    class config;

    struct websocket_deleter {
	void operator()(onion_websocket* socket);
    };

    class server {
    public:
	void run(const config& cfg, const std::vector<std::experimental::filesystem::path>&);
	void add_websocket (onion_websocket* sock) {
	    m_websockets.emplace_back(sock);
	}
	void close_websocket (onion_websocket* sock) {
	    auto it = std::find(begin(m_websockets), end(m_websockets), sock);
	    if (it != end(m_websockets)) {
		m_websockets.erase(it);
	    }
	}
	void refresh();

    private:
	std::vector<onion_websocket*> m_websockets;
    };

    class websocket_handler {
    public:
	websocket_handler (server& s) : m_server{s} {}
	onion_connection_status operator() (Onion::Request &req, Onion::Response &res);
    private:
	server& m_server;
    };
}

#endif
