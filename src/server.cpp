#include <experimental/filesystem>
#include <iostream>

#include "onion.hpp"
#include "response.hpp"
#include "url.hpp"
#include "extrahandlers.hpp"

#include "config.hpp"
#include "server.hpp"

namespace fs = std::experimental::filesystem;

namespace amer {
    void run(const config& cfg, const std::vector<fs::path>& files) {
        ONION_INFO("Listening at https://localhost:8080");

        Onion::Onion server(O_POOL);
        Onion::Url root(&server);

        for (auto file : files) {
            std::cout << file.string() << std::endl;
            root.add(file.replace_extension("").string(), Onion::StaticHandler(file.string()));
        }

        server.listen();
    }
}
