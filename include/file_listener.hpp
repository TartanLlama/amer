#ifndef AMER_FILE_LISTENER_HPP
#define AMER_FILE_LISTENER_HPP

#include <map>
#include <experimental/filesystem>

#include "config.hpp"
#include "renderer.hpp"
#include "server.hpp"

namespace amer {
    class file_listener {
	using path = std::experimental::filesystem::path;
	using file_time_type = std::experimental::filesystem::file_time_type;

    public:
	file_listener (server& s, config cfg, renderer& rend) :
            m_server{s}, m_config{std::move(cfg)}, m_renderer{rend} {}
	void run();

        void record_modification_time(const path& f, file_time_type new_write_time) {
            m_mod_times[f] = new_write_time;
        }

    private:
	void do_run();

        server& m_server;
	config m_config;
	renderer& m_renderer;

	std::thread m_listener_thread;
	std::map<path, file_time_type> m_mod_times;
    };
}

#endif
