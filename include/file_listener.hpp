#ifndef AMER_FILE_LISTENER_HPP
#define AMER_FILE_LISTENER_HPP

#include <map>
#include <experimental/filesystem>

#include "config.hpp"
#include "server.hpp"

namespace amer {
    class file_listener {
	using path = std::experimental::filesystem::path;
	using file_time_type = std::experimental::filesystem::file_time_type;

    public:
	file_listener (server& s, config cfg) : m_server{s}, m_config{cfg} {}
	void run();

    private:
	void do_run();

	config m_config;
	server& m_server;
	std::thread m_listener_thread;
	std::map<path, file_time_type> m_mod_times;
    };
}

#endif
