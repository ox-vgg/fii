# Notes For Developers

## Compile
```
git clone ...               # checkout code from git repository
cd find-identical-img/
mkdir build && cd build
cmake ../src
make
make test
./fii ../data/test/
```

## Check for Memory Leaks
```
valgrind --leak-check=full ./fii_image_size_test
valgrind --leak-check=full ./fii ../data/test/
```