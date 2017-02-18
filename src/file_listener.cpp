#include <thread>
#include <chrono>
#include <iostream>

#include "file_listener.hpp"

namespace fs = std::experimental::filesystem;

static constexpr auto sleep_time = std::chrono::milliseconds(100)

namespace amer {
    void file_listener::run() {
	m_listener_thread = std::thread{[this] { this->do_run(); }};
    }

    void file_listener::do_run() {
	while (true) {
	    for (auto&& f : fs::recursive_directory_iterator(m_config.get_source_dir()/"content")) {
		try {
		    auto new_write_time = fs::last_write_time(f);

                    auto file_is_new = !m_mod_times.count(f);
                    auto file_has_been_updated = new_write_time != m_mod_times[f];
                    auto should_rerender = file_is_new || file_has_been_updated;

		    if (should_rerender) {
                        record_modification_time(f, new_write_time);
                        m_renderer.render_content(f);
			m_server.refresh();
		    }
		} catch(...) {}
	    }
	    std::this_thread::sleep_for(sleep_time);
	}
    }
}
