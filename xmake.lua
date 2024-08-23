function mingw_special_settings()
    if is_plat("mingw") then
        add_ldflags("-lstdc++exp", "-lws2_32")
    end
end


set_languages("cxxlatest")

add_rules("mode.debug", "mode.release")
add_requires("asio", "nlohmann_json", "spdlog")

if is_mode("debug") then
    add_defines("DEBUG")
end

add_packages("asio", "nlohmann_json", "spdlog")
add_defines("JSON_USE_IMPLICIT_CONVERSIONS=0")
mingw_special_settings()


target("little-sb-client")
    set_kind("binary")
    add_files("src/little-sb-client.cpp", "src/application.cpp")

target("little-sb-server")
    set_kind("binary")
    add_files("src/little-sb-server.cpp", "src/server.cpp")
