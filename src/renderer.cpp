#include <fstream>

#include "cmark.h"

#include "renderer.hpp"

namespace fs = std::experimental::filesystem;

namespace {
    fs::path source_to_target_path (const amer::config& cfg, const fs::path& source) {
        auto path = source.string().substr(cfg.get_source_dir().string().size() + std::string{"content"}.size());
        return cfg.get_target_dir().string().append(path);
    }
}

namespace amer {
    std::shared_ptr<cpptoml::table> renderer::parse_toml(std::istream& is) {
        std::stringstream toml_stream;

        std::string str = "";

        auto stream_start_pos = is.tellg();
        std::getline(is, str);
        if (str != "+++") {
            is.seekg(stream_start_pos);
            return nullptr;
        }

        while (true) {
            std::getline(is, str);

            if (str == "+++") {
                break;
            }
            else {
                toml_stream << str << '\n';
            }
        }

        cpptoml::parser toml{toml_stream};
        return toml.parse();
    }

    std::ifstream renderer::get_layout() {
        auto path = m_config.get_source_dir() / "layouts" / "default.html";
        return std::ifstream{path.string()};
    }

    void renderer::render_to_stream(std::istream& is, std::ostream& os) {
        auto toml = parse_toml(is);

        if (toml) {
            for (auto&& entry : *toml) {
                m_context.add_entity(entry.first, entry.second->as<koura::text_t>()->get());
            }
        }

        koura::engine engine{};

        engine.render(is, os, m_context);

    }

    void renderer::render_markdown_to_stream(std::istream& is, std::ostream& os) {
        std::ostringstream markdown_stream;
        render_to_stream(is, markdown_stream);

        auto markdown_string = markdown_stream.str();

        os << cmark_markdown_to_html(markdown_string.c_str(), markdown_string.size(), 0);
    }

    fs::path renderer::render_content(const fs::directory_entry& dir_entry) {
        std::ifstream is {dir_entry.path().string()};

        std::ostringstream content_stream;
        render_markdown_to_stream(is, content_stream);

        koura::context layout_context{};
        layout_context.add_entity("content", content_stream.str());

        auto layout = get_layout();
        renderer rend {layout_context, m_config};

        auto target = source_to_target_path(m_config, dir_entry.path()).replace_extension("html");
        std::ofstream outfile {target};

        if (!m_config.get_standalone()) {
            std::ostringstream intermediate_stream;
            rend.render_to_stream(layout,intermediate_stream);
            auto intermediate_string = intermediate_stream.str();

            static std::string_view web_socket_script =
                "<script>\
                     var sock = new WebSocket('ws://localhost:8080/refresh-socket'); \
                     sock.onmessage = function (e) {                 \
                         location = location;                        \
                         sock.send('ok');                            \
                     }                                               \
                </script>";

            //HE COMES
            auto pos = intermediate_string.find("<html>");
            if (pos != std::string::npos) {
                intermediate_string.insert(pos+6, web_socket_script);
            }

            outfile << intermediate_string;

        }
        else {
            rend.render_to_stream(layout,outfile);
        }

        return target;
    }
}
