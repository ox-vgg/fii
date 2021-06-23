# Notes For Developers

## Dependencies
Requires a compiler (e.g. gcc) that supports C++11.

## Compile
```
git clone git@gitlab.com:vgg/fii.git
cd fii && cmake -B build src/
make -C build -j
make test -C build -j           # run tests (optional)
```

## Check for Memory Leaks
```
valgrind --leak-check=full ./fii_image_size_test
valgrind --leak-check=full ./fii ../data/test/
```