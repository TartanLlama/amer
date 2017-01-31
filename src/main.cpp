#include <experimental/filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>

#include "cxxopts.hpp"
#include "koura.hpp"
#include "cpptoml.h"

#include "server.hpp"
#include "renderer.hpp"
#include "config.hpp"
#include "file_listener.hpp"

namespace fs = std::experimental::filesystem;
using namespace amer;


koura::context parse_site_config(const fs::path& file) {
    auto toml = cpptoml::parse_file(file.c_str());

    koura::context ctx{};
    koura::object_t site{};
    for (auto&& [key,value] : *toml) {
        site[key] = value->as<koura::text_t>()->get();
    }
    ctx.add_entity("site", site);

    return ctx;
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("amer", "Static website generator");
    options.add_options()
        ("d,dir", "Site directory", cxxopts::value<std::string>())
        ("t,target_dir", "Build directory", cxxopts::value<std::string>())
        ("c,config", "Configuration file", cxxopts::value<std::string>())
	("s,standalone", "Compile to be served separately from amer")
        ("h,help", "Print help");
    options.parse(argc, argv);

    if (options.count("help")) {
        std::cout << options.help() << '\n';
    }

    fs::path site_root = options.count("d") ? options["d"].as<std::string>() : ".";
    fs::path target = options.count("t") ? options["t"].as<std::string>() : "site";

    fs::path config_path = options.count("c") ? fs::path{options["c"].as<std::string>()} : site_root/"config.toml";

    fs::create_directory(target);

    auto ctx = parse_site_config(config_path);

    bool standalone = options.count("s");
    config cfg {site_root,target,standalone};

    renderer rend {ctx,cfg};

    std::vector<fs::path> files;
    for (auto&& f : fs::recursive_directory_iterator(site_root/"content")) {
        if (fs::is_regular_file(f)) {
            auto path = rend.render_content(f);
            files.push_back(path);
        }
    }

    if (!cfg.get_standalone()) {
        server s{};
        std::thread server_thread{
            [&s,&cfg,&files] { s.run(cfg,files); }
        };

        file_listener listener {s,cfg,rend};
        listener.run();

        while (true){}
    }
}
