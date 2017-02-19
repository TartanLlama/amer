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

    server::server() : m_server{O_POOL}, m_root{&m_server} {
        //This should really be -1, but it seems there is a bug
        //with infinite websocket timeouts, so just something large for now
        constexpr auto timeout = 1'000'000'000;

	m_server.setTimeout(timeout);
    }

    void server::register_path(std::string path, const std::experimental::filesystem::path& file) {
        path.replace(path.find("site/"), 5, ""); //TODO make this generic
        ONION_INFO("Registered path %s to %s", path.c_str(), file.c_str());
        m_root.add(std::move(path), Onion::StaticHandler(file.string()));
    }

    void server::register_redirect(std::string path, const std::experimental::filesystem::path& file) {
        path.replace(path.find("site/"), 5, ""); //TODO make this generic
        auto to = file.string();
        to = "/" + to.replace(to.find("site/"), 5, ""); //TODO make this generic
        ONION_INFO("Registered redirect %s to %s", path.c_str(), to.c_str());
        m_root.add(std::move(path), Onion::RedirectHandler(to));
    }

    void server::run(const config& cfg, const std::vector<fs::path>& files) {
        ONION_INFO("Listening at https://localhost:8080");

        for (auto file : files) {
            constexpr auto date_length = 11;
            auto stem = file.stem().string().substr();
            auto path = file.parent_path() / stem.substr(date_length);
            register_path(path, file.string());

            auto year = stem.substr(0,4);
            auto month = stem.substr(5,2);
            auto day = stem.substr(8,2);
            register_redirect(file.parent_path() / year / month / day / stem.substr(date_length), path);
        }

	m_root.add("refresh-socket", websocket_handler{*this});

        m_server.listen();
    }
}
