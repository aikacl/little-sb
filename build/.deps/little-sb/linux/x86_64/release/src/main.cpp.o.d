{
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-std=c++26"
        }
    },
    files = {
        "src/main.cpp"
    },
    depfiles_gcc = "main.o: src/main.cpp src/application.h src/player.h src/random.h  src/state.h\
"
}