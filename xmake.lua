function mingw_special_settings()
    if is_plat("mingw") then
        add_ldflags("-lstdc++exp", "-lws2_32")
    end
end


set_languages("cxxlatest")

requirements = {
  "asio",
  "glfw",
  "imgui",
  "nlohmann_json",
  "spdlog",
}

add_rules("mode.debug", "mode.release")
add_includedirs("src/")
for _, package in pairs(requirements) do
    add_requires(package)
    add_packages(package)
end

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
