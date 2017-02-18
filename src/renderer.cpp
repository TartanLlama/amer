#include <fstream>

#include "cmark.h"

#include "renderer.hpp"
#include "paths.hpp"

namespace fs = std::experimental::filesystem;

namespace amer {
    renderer::renderer (koura::context ctx, config cfg)
        : m_engine{}, m_context{std::move(ctx)}, m_config{std::move(cfg)}
    {
        m_engine.register_custom_expression("include", include_handler, std::ref(m_config));
    }

    void renderer::include_handler (koura::engine& eng, std::istream& in, std::ostream& out,
                                    koura::context& ctx, const std::any& data) {
        koura::detail::eat_whitespace(in);
        std::string id = "";
        while (!std::isspace(in.peek())) {
            id += in.get();
        }

        koura::detail::eat_whitespace(in);

        if (in.get() != '%' || in.get() != '}') {
            throw koura::render_error{in};
        }

        auto& cfg = std::any_cast<std::reference_wrapper<config>>(data).get();

        auto path = cfg.get_source_dir() / "includes" / id;

        std::ifstream file {path.string()};

        renderer r {ctx, cfg};
        r.render_to_stream(file, out);
    }


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

    std::ifstream renderer::get_layout(koura::context& ctx) {
        std::string layout;

        auto page = ctx.get_entity("page").get_value<koura::object_t>();

        try {
            layout = page.at("layout").get_value<koura::text_t>();;
        } catch (std::out_of_range&) {
            layout = "default";
        }

        auto path = m_config.get_source_dir() / "layouts" / (layout + ".html");
        return std::ifstream{path.string()};
    }

    koura::context renderer::render_to_stream(std::istream& is, std::ostream& os) {
        auto toml = parse_toml(is);

        auto ctx = m_context;

        if (toml) {
            koura::object_t page;

            if (ctx.contains("page")) {
                page = ctx.get_entity("page").get_value<koura::object_t>();
            }

            for (auto&& [key, value] : *toml) {
                auto val = value->as<koura::text_t>()->get();
                page[key] = val;
            }

            ctx.add_entity("page", page);
        }

        m_engine.render(is, os, ctx);

        return ctx;
    }

    koura::context renderer::render_markdown_to_stream(std::istream& is, std::ostream& os) {
        std::ostringstream markdown_stream;
        auto ctx = render_to_stream(is, markdown_stream);

        auto markdown_string = markdown_stream.str();

        auto page = ctx.get_entity("page").get_value<koura::object_t>();
        if (page.count("layout")) {
            auto html = cmark_markdown_to_html(markdown_string.c_str(), markdown_string.size(), 0);

            koura::context layout_context {ctx};
            layout_context.add_entity("content", html);

            auto layout = get_layout(ctx);
            renderer rend {layout_context, m_config};

            ctx = rend.render_to_stream(layout, os);
        }
        else {
            os << cmark_markdown_to_html(markdown_string.c_str(), markdown_string.size(), 0);
        }

        return ctx;
    }

    fs::path renderer::render_content(const fs::directory_entry& dir_entry) {
        std::ifstream is {dir_entry.path().string()};

        std::ostringstream content_stream;
        auto ctx = render_markdown_to_stream(is, content_stream);

        koura::context layout_context{ctx};
        layout_context.add_entity("content", content_stream.str());

        auto layout = get_layout(ctx);
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
