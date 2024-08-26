function mingw_special_settings()
    if is_plat("mingw") then
        add_ldflags("-lstdc++exp", "-lws2_32")
    end
end


set_languages("cxxlatest")

add_rules("mode.debug", "mode.release")
add_requires("asio", "imgui", "nlohmann_json", "spdlog")
add_packages("asio", "imgui", "nlohmann_json", "spdlog")
add_includedirs("src/")

mingw_special_settings()

target("lib")
    set_kind("static")
    add_files("src/command.cpp")
    add_defines("JSON_USE_IMPLICIT_CONVERSIONS=0")

target("little-sb-client")
    set_kind("binary")
    add_files("src/client/*.cpp")
    add_deps("lib")

target("little-sb-server")
    set_kind("binary")
    add_files("src/server/*.cpp")
    add_deps("lib")
