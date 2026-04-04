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

# Run debug version
run-debug: build-debug
    cd build/debug/bin && ./sdl_gpu_fps_starter

# Run release version
run-release: build-release
    cd build/release/bin && ./sdl_gpu_fps_starter

# Run debug with RenderDoc capture
renderdoc-debug: build-debug
    cd build/debug/bin && renderdoccmd capture ./sdl_gpu_fps_starter

# Run release with RenderDoc capture
renderdoc-release: build-release
    cd build/release/bin && renderdoccmd capture ./sdl_gpu_fps_starter

# Clean debug build and rebuild
clean-build-debug:
    rm -rf build/debug
    just build-debug

# Clean release build and rebuild
clean-build-release:
    rm -rf build/release
    just build-release

# Clean and rebuild both configurations
clean-build-all:
    rm -rf build/debug build/release
    just build-debug
    just build-release

# Clean everything
clean:
    rm -rf build/debug build/release
