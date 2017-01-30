#include <thread>
#include <chrono>
#include <iostream>

#include "file_listener.hpp"

namespace fs = std::experimental::filesystem;

namespace amer {
    void file_listener::run() {
	m_listener_thread = std::thread{[this] { this->do_run(); }};
    }

    void file_listener::do_run() {
	while (true) {
	    for (auto&& f : fs::recursive_directory_iterator(m_config.get_source_dir()/"content")) {
		try {
		    auto new_write_time = fs::last_write_time(f);
		    if (!m_mod_times.count(f) || new_write_time != m_mod_times[f]) {
			m_mod_times[f] = new_write_time;
			m_server.refresh();
		    }
		} catch(...) {}
	    }
	    std::this_thread::sleep_for(std::chrono::seconds(1));
	}
    }
}
