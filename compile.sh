clang++ -O3 live.cpp -DNO_OUTPUT `llvm-config --cxxflags --ldflags --system-libs --libs core` -o live

clang++ -O3 live.cpp -DNO_OUTPUT -DLIVE_CONCURRENT `llvm-config --cxxflags --ldflags --system-libs --libs core` -o live-c