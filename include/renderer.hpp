#ifndef AMER_RENDERER_HPP
#define AMER_RENDERER_HPP

#include <experimental/filesystem>
#include <iostream>

#include "config.hpp"
#include "koura.hpp"
#include "cpptoml.h"

namespace amer {
    class renderer {
    public:
        renderer (koura::context ctx, config cfg);

        auto render_content(const std::experimental::filesystem::directory_entry& is)
            -> std::experimental::filesystem::path;

        static void include_handler(koura::engine& eng, std::istream& in, std::ostream& out,
                                    koura::context& ctx, const std::any& data);

    private:
        auto parse_toml(std::istream& is) -> std::shared_ptr<cpptoml::table>;
        auto get_layout(koura::context& ctx) -> std::ifstream;
        auto render_to_stream(std::istream& is, std::ostream& os) -> koura::context;
        auto render_markdown_to_stream(std::istream& is, std::ostream& os) -> koura::context;

        koura::engine m_engine;
        koura::context m_context;
        config m_config;
    };
}

#endif
