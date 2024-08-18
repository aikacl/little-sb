set_languages("cxxlatest")

target("little-sb")
set_kind("binary")
add_files("src/main.cpp")
add_includedirs("third-party/asio-1.30.2/include")
