set cpp_files=main.cpp GraphicsStructs.cpp
set linker_libs=-lopengl32 -lglu32 -lfreeglut
set libraries=-I lib
set options=-ffloat-store
g++ %cpp_files% %linker_libs% %libraries% %options% %*