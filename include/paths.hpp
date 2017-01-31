#ifndef AMER_PATHS_HPP
#define AMER_PATHS_HPP

#include <experimental/filesystem>

#include "config.hpp"

namespace amer {
//TODO make these not awful

    inline auto source_to_target_path (const config& cfg, const std::experimental::filesystem::path& source)
        -> std::experimental::filesystem::path {
        auto path = source.string().substr(cfg.get_source_dir().string().size() + std::string{"content"}.size());
        return cfg.get_target_dir().string().append(path);
    }

    inline auto static_to_target_path (const config& cfg, const std::experimental::filesystem::path& source)
        -> std::experimental::filesystem::path {
        auto path = source.string().substr(cfg.get_source_dir().string().size() + std::string{"static"}.size());
        return cfg.get_target_dir().string().append(path);
    }
}

#endif
