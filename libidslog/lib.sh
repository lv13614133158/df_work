mkdir -p lib
gcc -shared -fPIC -o lib/libidslog.so   src/libidslog.c -I ./include  -lpthread