# uperf
x86 cpu micro arch test on windows
only support build with clang, ninja build on windows

# build
cmake -Bbuild -GNinja -S. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build