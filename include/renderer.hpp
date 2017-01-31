#ifndef AMER_RENDERER_HPP
#define AMER_RENDERER_HPP

#include <experimental/filesystem>
#include "config.hpp"
#include "koura.hpp"

namespace amer {
    class renderer {
    public:
        renderer (koura::context ctx, config cfg) : m_context{std::move(ctx)}, m_config{std::move(cfg)} {}


        auto render_file(const std::experimental::filesystem::directory_entry& d)
            -> std::experimental::filesystem::path;

    private:
        koura::context m_context;
        config m_config;
    };
}

#endif
