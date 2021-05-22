# Find Identical Images (find-identical-img)
A command line tool to find all identical images (i.e. exact duplicate images
that have same pixel values) in a folder.

## Usage
```
The shell command

$ find-identical-img [OPTIONS] TARGET_DIR CHECK_DIR1 CHECK_DIR2 ...

checks if identical copies of images contained in the TARGET_DIR folder
is present in the CHECK_DIR1, CHECK_DIR2, etc. If the CHECK_DIRs are
not defined, the shell command checks for identical copies within the
TARGET_DIR folder.

The following options are available for the find-identical-img shell command:

--export [ -e ]  : write results to a file (default: fii-export.html)
```

## EXAMPLES
```
a) check if the YFCC dataset has images identical to ImageNet and Places dataset
  find-identical-img /dataset/yfcc /dataset/ILSVRC/train/ /dataset/Places/train/

b) check if the training subset of ImageNet dataset contains identical images
  find-identical-img /dataset/ILSVRC/train/
```

## Developer Resources
See [For-Developers.md](For-Developers.md) file.

## Other similar projects
 * https://github.com/idealo/imagededup

## Author
Abhishek Dutta <https://abhishekdutta.org>
