# Default recipe to show help
default:
    @just --list

# Configure debug build
configure-debug:
    CC=clang CXX=clang++ cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug

# Configure release build
configure-release:
    CC=clang CXX=clang++ cmake -B build/release -DCMAKE_BUILD_TYPE=Release

# Build debug version
build-debug: configure-debug
    cmake --build build/debug --parallel

# Build release version
build-release: configure-release
    cmake --build build/release --parallel

# Run debug version (from bin/ directory)
run-debug: build-debug
    cd build/debug/bin && ./blocks

# Run release version (from bin/ directory)
run-release: build-release
    cd build/release/bin && ./blocks

# Clean and rebuild debug
clean-build-debug:
    rm -rf build/debug
    just build-debug

# Clean and rebuild release
clean-build-release:
    rm -rf build/release
    just build-release

# Complete clean build (both debug and release)
clean-build-all:
    rm -rf build/debug build/release
    just build-debug
    just build-release

# Run debug with RenderDoc capture
renderdoc-debug: build-debug
    cd build/debug/bin && renderdoccmd capture ./blocks

# Run release with RenderDoc capture
renderdoc-release: build-release
    cd build/release/bin && renderdoccmd capture ./blocks

# Clean everything
clean:
    rm -rf build/debug build/release
