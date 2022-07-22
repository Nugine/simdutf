test:
    cmake -B build
    cmake --build build
    cd build
    ctest .

bench:
    cmake -B build
    cmake --build build
    ./build/benchmarks/benchmark --help

singleheader:
    python3 ./singleheader/amalgamate.py

demo: singleheader
    #!/bin/bash -ex
    cd singleheader
    c++ -o amalgamation_demo amalgamation_demo.cpp -std=c++17
    ./amalgamation_demo
