for i in `conan_client --server http://localhost:10000 --list |grep "[.*]"|sed "s/\[//g" |sed "s/\]//g"`; do conan_client --server http://localhost:10000 --onto_del $i; done
