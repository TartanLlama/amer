#include <experimental/filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include "cxxopts.hpp"
#include "cmark.h"
#include "cpptoml.h"

namespace fs = std::experimental::filesystem;

void render_file(const fs::directory_entry& d) {
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

        std::stringstream markdown_stream;
        markdown_stream << file.rdbuf();
        auto markdown_string = markdown_stream.str();
        std::cout << cmark_markdown_to_html(markdown_string.c_str(), markdown_string.size(), 0);
    }
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("amer", "Static website generator");
    options.add_options()
        ("d,dir", "Site directory", cxxopts::value<std::string>())
        ("h,help", "Print help");
    options.parse(argc, argv);

    if (options.count("help")) {
        std::cout << options.help() << '\n';
    }
    
    std::string dir = options.count("d") ? options["d"].as<std::string>() : ".";
    
    for (auto&& f : fs::recursive_directory_iterator(dir)) {
        render_file(f);
    }
}
