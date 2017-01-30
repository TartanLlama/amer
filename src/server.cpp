#include <experimental/filesystem>
#include <iostream>
#include <algorithm>

#include "onion.hpp"
#include "response.hpp"
#include "url.hpp"
#include "extrahandlers.hpp"


#include "config.hpp"
#include "server.hpp"

namespace fs = std::experimental::filesystem;

namespace amer {
    void websocket_deleter::operator()(onion_websocket* socket) {
	onion_websocket_free(socket);
    }

    onion_connection_status websocket_callback(void* data, onion_websocket* ws, ssize_t data_ready_length) {
	auto connection_closed = data_ready_length == -1;
	if (connection_closed) {
	    static_cast<server*>(data)->close_websocket(ws);
	}
    }

    onion_connection_status websocket_handler::operator() (Onion::Request &req, Onion::Response &res) {
	auto sock = onion_websocket_new(req.c_handler(), res.c_handler());
	onion_websocket_set_callback(sock, websocket_callback);
	onion_websocket_set_userdata(sock, static_cast<void*>(&m_server), nullptr);
	m_server.add_websocket(sock);

	return OCS_PROCESSED;
    }

    void server::refresh() {
	for (auto&& sock : m_websockets) {
	    onion_websocket_printf(sock, "refresh");
	}
    }

    void server::run(const config& cfg, const std::vector<fs::path>& files) {
        ONION_INFO("Listening at https://localhost:8080");

        Onion::Onion server(O_POOL);
	server.setTimeout(-1);
        Onion::Url root(&server);

        for (auto file : files) {
            root.add(file.replace_extension("").string(), Onion::StaticHandler(file.string()));
        }

	root.add("refresh-socket", websocket_handler{*this});

        server.listen();
    }
}
