#include <fstream>

#include "cmark.h"
#include "cpptoml.h"

#include "renderer.hpp"

namespace fs = std::experimental::filesystem;

namespace {
    fs::path source_to_target_path (const amer::config& cfg, const fs::path& source) {
        auto path = source.string().substr(cfg.get_source_dir().string().size() + std::string{"content"}.size());
        return cfg.get_target_dir().string().append(path);
    }
}

namespace amer {
    fs::path renderer::render_file(const fs::directory_entry& d) {
        std::string str = "";
        std::ifstream file {d.path().c_str()};
        std::getline(file, str);

        std::stringstream toml_stream;

        if (str == "+++") {
            while (true) {
                std::getline(file, str);

                if (str == "+++") {
                    break;
                }
                else {
                    toml_stream << str << '\n';
                }
            }
        }

        cpptoml::parser toml{toml_stream};
        auto table = toml.parse();
        for (auto&& entry : *table) {
            m_context.add_entity(entry.first, entry.second->as<koura::text_t>()->get());
        }

        koura::engine engine{};

        std::stringstream markdown_stream;
        engine.render(file, markdown_stream, m_context);

        auto markdown_string = markdown_stream.str();
        auto target = source_to_target_path(m_config, d.path()).replace_extension("html");
        std::ofstream outfile {target};
        if (!m_config.get_standalone()) {
            outfile << "<script>\
                        var sock = new WebSocket('ws://localhost:8080/refresh-socket');	\
                        sock.onmessage = function (e) { \
                            location = location;\
                            sock.send('ok');\
                        }\
                    </script>";
        }
        outfile << cmark_markdown_to_html(markdown_string.c_str(), markdown_string.size(), 0);

        return target;
    }
}
