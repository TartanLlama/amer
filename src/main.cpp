#include <experimental/filesystem>
#include <iostream>
#include <sstream>
#include <fstream>

#include "cxxopts.hpp"
#include "cmark.h"
#include "cpptoml.h"
#include "koura.hpp"

#include "server.hpp"
#include "config.hpp"

namespace fs = std::experimental::filesystem;
using namespace amer;

fs::path source_to_target_path (const config& cfg, const fs::path& source) {
    auto path = source.string().substr(cfg.get_source_dir().string().size() + std::string{"content"}.size());
    return cfg.get_target_dir().string().append(path);
}

koura::context parse_site_config(const fs::path& file) {
    auto toml = cpptoml::parse_file(file.c_str());

    koura::context ctx{};
    koura::object_t site{};
    for (auto&& entry : *toml) {
        site[entry.first] = entry.second->as<koura::text_t>()->get();
    }
    ctx.add_entity("site", site);

    return ctx;
}

fs::path render_file(koura::context& ctx, const config& cfg, const fs::directory_entry& d) {
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
                toml_stream << str;
            }
        }
    }

    cpptoml::parser toml{toml_stream};
    auto table = toml.parse();
    for (auto&& entry : *table) {
        ctx.add_entity(entry.first, entry.second->as<koura::text_t>()->get());
    }

    koura::engine engine{};

    std::stringstream markdown_stream;
    engine.render(file, markdown_stream, ctx);

    auto markdown_string = markdown_stream.str();
    auto target = source_to_target_path(cfg, d.path()).replace_extension("html");
    std::ofstream outfile {target};
    outfile << cmark_markdown_to_html(markdown_string.c_str(), markdown_string.size(), 0);

    return target;
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("amer", "Static website generator");
    options.add_options()
        ("d,dir", "Site directory", cxxopts::value<std::string>())
        ("t,target_dir", "Build directory", cxxopts::value<std::string>())
        ("c,config", "Configuration file", cxxopts::value<std::string>())
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

    config cfg {site_root,target};
    std::vector<fs::path> files;
    for (auto&& f : fs::recursive_directory_iterator(site_root/"content")) {
        auto path = render_file(ctx,cfg,f);
        files.push_back(path);
    }

    run(cfg,files);
}
