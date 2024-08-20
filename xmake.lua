set_languages("cxxlatest")

add_requires("spdlog")

target("little-sb-client")
set_kind("binary")
add_files("src/little-sb-client.cpp")
add_includedirs("third-party/asio-1.30.2/include")
add_packages("spdlog")

target("little-sb-server")
set_kind("binary")
add_files("src/little-sb-server.cpp")
add_includedirs("third-party/asio-1.30.2/include")
add_packages("spdlog")
