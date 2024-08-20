function mingw_special_settings()
    if is_plat("mingw") then
        add_ldflags("-lstdc++exp", "-lws2_32")
    end
end

set_languages("cxxlatest")
add_requires("spdlog", "asio")


target("little-sb-client")
    set_kind("binary")
    add_files("src/little-sb-client.cpp")
    add_packages("spdlog", "asio")
    mingw_special_settings()


target("little-sb-server")
    set_kind("binary")
    add_files("src/little-sb-server.cpp")
    add_packages("spdlog", "asio")
    mingw_special_settings()
