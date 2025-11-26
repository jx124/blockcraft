
#!/bin/sh

case $1 in
  --debug|-d)
    cmake -B build/debug -DCMAKE_BUILD_TYPE=debug
    cmake --build ./build/debug -j $(nproc) && cp -u ./build/debug/compile_commands.json .
    ;;
  --release|-r)
    cmake -B build/release -DCMAKE_BUILD_TYPE=release
    cmake --build ./build/release -j $(nproc) && cp -u ./build/release/compile_commands.json .
    ;;
  *)
    echo "Usage: ./build.sh [--release|-r|--debug|-d]"
    ;;
esac
