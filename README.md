# uperf
x86/arm cpu micro arch test on windows/mac\
only support build with clang, ninja build on windows/mac

# build
cmake -Bbuild -GNinja -S. -DCMAKE_BUILD_TYPE=RelWithDebInfo\
cmake --build build

# run
There are 2 mode for currents tests.
Delay Mode:
    -delay 
    -prologue
    -epilogue

Period Mode:
    -inst_num
    -thrput_inst
    -thrput_fill

There are some default patterns defined in code.
    -case [0, 44] 
We also can use pattern file to add a new test
    -f pattern_filename
If we want to write asm in pattern, please make sure llvm-mc is already in the path

There is a python script to show the result by a graph. Need to install matplotlib.