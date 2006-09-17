for i in `conan_client --server http://localhost:10000 --list |grep think|grep -o http.*[0-9]`; do 
    conan_client --server http://localhost:10000 --onto_del $i; 
done
