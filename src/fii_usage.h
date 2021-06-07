#ifndef FII_USAGE_H
#define FII_USAGE_H

namespace fii {
  const char *FII_HELP_STR = R"TEXT(Usage: fii [OPTIONS] CHECK_DIR1 [CHECK_DIR2]

checks if images contained in the CHECK_DIR1 folder have identical copies in the 
CHECK_DIR2 folder. If the optional argument CHECK_DIR2 is undefined, the command 
checks for identical copies within the CHECK_DIR1 folder.


The following options are available:

--export[=FILE]  : write results to HTML, CSV or JSON file
--nthread[=N]    : use only N threads instead of all available threads
--exhaustive-search : guarantee a false positive rate of 0 by exchaustively comparing all pixel values of initially identified identical images

Here are some example commands:
a) check if the YFCC dataset has images identical to ImageNet dataset
  find-identical-img /dataset/yfcc /dataset/ILSVRC/train/

b) check if the training subset of ImageNet dataset contains identical images
  find-identical-img /dataset/ILSVRC/train/


Authors : Abhishek Dutta, Andrew Zisserman
Contact : {adutta, az} @ robots.ox.ac.uk
Website : https://www.robots.ox.ac.uk/~vgg/software/fii
Code    : https://gitlab.com/vgg/fii
)TEXT";

}
#endif
