#include <experimental/filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include "cxxopts.hpp"
#include "cmark.h"
#include "cpptoml.h"
#include "koura/engine.hpp"

namespace fs = std::experimental::filesystem;

class config {
public:
    config (std::string source_dir, std::string target_dir) :
        m_source_dir{std::move(source_dir)}, m_target_dir{std::move(target_dir)}
    {}

    const fs::path& get_source_dir() const { return m_source_dir; }
    const fs::path& get_target_dir() const { return m_target_dir; }    
    
private:
    fs::path m_source_dir;
    fs::path m_target_dir;
};

fs::path source_to_target_path (const config& cfg, const fs::path& source) {
    return cfg.get_target_dir().string().append(source.string().substr(cfg.get_source_dir().string().size()));
}

void render_file(const config& cfg, const fs::directory_entry& d) {
    if (fs::is_regular_file(d)) {
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
        std::cout << "Title = " << table->get_as<std::string>("title").value_or("not found") << '\n';

        koura::engine engine{};
        koura::context ctx{};
        ctx.add_entity("what", koura::text_t{"world"});

        std::stringstream markdown_stream;
        engine.render(file, markdown_stream, ctx);

        auto markdown_string = markdown_stream.str();
        std::ofstream outfile {source_to_target_path(cfg, d.path()).replace_extension("html")};
        outfile << cmark_markdown_to_html(markdown_string.c_str(), markdown_string.size(), 0);
    }
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("amer", "Static website generator");
    options.add_options()
        ("d,dir", "Site directory", cxxopts::value<std::string>())
        ("t,target_dir", "Build directory", cxxopts::value<std::string>())        
        ("h,help", "Print help");
    options.parse(argc, argv);

    if (options.count("help")) {
        std::cout << options.help() << '\n';
    }
    
    fs::path dir = options.count("d") ? options["d"].as<std::string>() : ".";
    dir /= "content";
    fs::path target = options.count("t") ? options["t"].as<std::string>() : "site";

    fs::create_directory(target);
    
    config cfg {dir,target};
    for (auto&& f : fs::recursive_directory_iterator(dir)) {
        render_file(cfg,f);
    }
}
