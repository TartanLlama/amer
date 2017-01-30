#ifndef AMER_SERVER_HPP
#define AMER_SERVER_HPP

#include <vector>
#include <experimental/filesystem>

namespace amer {
    class config;
    void run(const config& cfg, const std::vector<std::experimental::filesystem::path>&);
}

#endif
