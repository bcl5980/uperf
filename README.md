# uperf
x86/arm cpu micro arch test on windows/mac\
only support build with clang, ninja build on windows/mac

# build
cmake -Bbuild -GNinja -S. -DCMAKE_BUILD_TYPE=RelWithDebInfo\
cmake --build build