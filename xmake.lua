function mingw_special_settings()
    if is_plat("mingw") then
        add_ldflags("-lstdc++exp", "-lws2_32")
    end
end

add_rules("mode.debug", "mode.release")
set_languages("cxxlatest")
add_requires("asio", "nlohmann_json", "spdlog")


target("little-sb-client")
    set_kind("binary")
    add_files("src/little-sb-client.cpp")
    add_packages("asio", "nlohmann_json", "spdlog")
    mingw_special_settings()


target("little-sb-server")
    set_kind("binary")
    add_files("src/little-sb-server.cpp")
    if is_mode("debug") then
        add_defines("DEBUG")
    end
    add_packages("asio", "nlohmann_json", "spdlog")
    mingw_special_settings()
