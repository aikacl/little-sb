set_languages("cxxlatest")

add_requires("spdlog", "asio")

target("little-sb-client")
set_kind("binary")
add_files("src/little-sb-client.cpp")
add_packages("spdlog", "asio")

target("little-sb-server")
set_kind("binary")
add_files("src/little-sb-server.cpp")
add_packages("spdlog", "asio")
