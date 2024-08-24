function mingw_special_settings()
    if is_plat("mingw") then
        add_ldflags("-lstdc++exp", "-lws2_32")
    end
end


set_languages("cxxlatest")

add_rules("mode.debug", "mode.release")
add_requires("asio", "nlohmann_json", "spdlog")
add_packages("asio", "nlohmann_json", "spdlog")

if is_mode("debug") then
    add_defines("DEBUG")
end

mingw_special_settings()

target("lib")
    set_kind("static")
    add_files("src/application.cpp", "src/command.cpp", "src/server.cpp")
    add_defines("JSON_USE_IMPLICIT_CONVERSIONS=0")

target("little-sb-client")
    set_kind("binary")
    add_files("src/little-sb-client.cpp")
    add_deps("lib")

target("little-sb-server")
    set_kind("binary")
    add_files("src/little-sb-server.cpp", "src/server-command-executor.cpp")
    add_deps("lib")
