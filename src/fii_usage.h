#ifndef FII_USAGE_H
#define FII_USAGE_H

namespace fii {
  const char *FII_USAGE_STR = R"TEXT(
Usage: find-identical-img [OPTIONS] TARGET_DIR CHECK_DIR1 CHECK_DIR2 ...

Here are some examples:
a) check if the YFCC dataset has images identical to ImageNet and Places dataset
  find-identical-img /dataset/yfcc /dataset/ILSVRC/train/ /dataset/Places/train/

b) check if the training subset of ImageNet dataset contains identical images
  find-identical-img /dataset/ILSVRC/train/
)TEXT";

  const char *FII_HELP_STR = R"TEXT(## NAME
find-identical-img : find all identical images in a folder.


## SYNOPSIS
The shell command

$ find-identical-img [OPTIONS] TARGET_DIR CHECK_DIR1 CHECK_DIR2 ...

checks if identical copies of images contained in the TARGET_DIR folder
is present in the CHECK_DIR1, CHECK_DIR2, etc. If the CHECK_DIRs are
not defined, the shell command checks for identical copies within the
TARGET_DIR folder.


## COMMON OPTIONS
The following options are available for the find-identical-img shell command:

--export [ -e ]  : write results to a file (default: fii-export.html)


## EXAMPLES
a) check if the YFCC dataset has images identical to ImageNet and Places dataset
  find-identical-img /dataset/yfcc /dataset/ILSVRC/train/ /dataset/Places/train/

b) check if the training subset of ImageNet dataset contains identical images
  find-identical-img /dataset/ILSVRC/train/


## AUTHOR
Abhishek Dutta <https://abhishekdutta.org>

)TEXT";

}
#endif
