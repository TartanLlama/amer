#ifndef AMER_CONFIG_HPP
#define AMER_CONFIG_HPP

namespace amer {
    class config {
        using path = std::experimental::filesystem::path;

    public:
        config (std::string source_dir, std::string target_dir, bool standalone) :
            m_source_dir{std::move(source_dir)}, m_target_dir{std::move(target_dir)},
	    m_standalone{standalone}
        {}

        const path& get_source_dir() const { return m_source_dir; }
        const path& get_target_dir() const { return m_target_dir; }
	bool get_standalone() const { return m_standalone; }

    private:
        path m_source_dir;
        path m_target_dir;
	bool m_standalone;
    };
}
#endif
