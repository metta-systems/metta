set -x

g++ -o mkinitfs mkinitfs.cpp -O0 -ggdb -I../lib -I../lib/klibc -I../lib/bstrlib -I../lib/oskit -I../lib/oskit/oskit -I../lib/oskit/oskit/x86 -DBSTRLIB_CANNOT_USE_STL -DBSTRLIB_CANNOT_USE_IOSTREAM -DBSTRLIB_DOESNT_THROW_EXCEPTIONS -DBSTRLIB_DONT_USE_VIRTUAL_DESTRUCTOR -DBSTRLIB_DONT_ASSUME_NAMESPACE -L../build/x86-release/ -lbstrlib -lklibc
