# Find Identical Images (FII)
Identical images have the same image dimension (i.e. image width, image height, number of colour channels) and same pixel value in all corresponding pixel locations. FII is a command line tool to find all identical images in a folder. It can also find images that are common in two folders. See [https://www.robots.ox.ac.uk/~vgg/software/fii/](https://www.robots.ox.ac.uk/~vgg/software/fii/) for more details.

## Compiling from Source
FII can be compiled form source as shown below. FII only requires a standard C++ compiler (gcc, clang) and CMAKE to build from source code.

```
git clone git@gitlab.com:vgg/fii.git
cd fii && cmake -B build src/ && make -C build -j
make test -C build -j           # run tests (optional)
./build/fii --help
./build/fii /dataset/COCO2017/
```

## Usage
```
Usage: fii [OPTIONS] CHECK_DIR1 [CHECK_DIR2]

checks if images contained in the CHECK_DIR1 folder have identical copies in the
CHECK_DIR2 folder. If the optional argument CHECK_DIR2 is undefined, the command
checks for identical copies within the CHECK_DIR1 folder.


The following options are available:

--export[=DIR]     : export results (JSON, CSV, HTML) to this folder
--nthread[=N]      : use only N threads instead of all available threads
--check-all-pixels : check every pixel to prevent any false positive (slower)
```

## EXAMPLES
```
a) check if the YFCC dataset has images identical to ImageNet and Places dataset
  fii /dataset/yfcc /dataset/ILSVRC/train/ /dataset/Places/train/

b) check if the training subset of ImageNet dataset contains identical images
  fii /dataset/ILSVRC/train/
```

## How does FII work?
Identical images have same image dimension (i.e. image width, image height, number of colour channels) and same pixel value in all corresponding pixel locations. A pair of images may look identical on visual inspection but can have subtle difference in pixel values due to different image encoding scheme, watermarks, artefacts caused by file corruption, etc. To identify such identical images in a folder, we need to compare the raw pixel value of all unique image pairs in that folder. This is a computationally expensive process and FII uses the following two strategies to speed up this process.

First, FII groups all images in a folder based on their image dimension and performs comparison of pixel values on image pairs only within this group. FII does not need to compare images in a group with images in any other group with a different image dimension. In other words, all the images with a certain image dimension (e.g. 100x200x3) will never be identical with images of any other dimension (e.g. 101x201x3). This strategy of grouping images by their dimension allows FII to reduce the search space and therefore arrive at the results much faster. FII can perform this grouping operation very efficiently by [reading image dimensions](https://gitlab.com/vgg/fii/-/blob/master/src/fii_image_size.h) by loading only the image header and not the the full image. This ensures that the task of grouping of image by their dimension does not depend on image size and therefore is fast. For example, FII found 100027 unique image dimensions among 1281167 images in the [ImageNet 2012](https://www.image-net.org/challenges/LSVRC/2012/) dataset in 122 sec. using 8 threads in a 2020 machine (AMD 16 core 3.2GHz). However, this strategy of grouping images by their image dimension does not provide much improvement in processing speed for datasets that have large number of images with only few variations in image dimensions. For example, the training subset of [Places205](http://places.csail.mit.edu/downloadData.html) dataset contains 2448872 images with only the following two image dimensions: 256x256x3 (RGB colour images) and 256x256x1 (grayscale images) and therefore FII takes significantly longer (~6 hours using 24 threads in a 2020 machine) to process this dataset.

Second, FII processess all the images in a group using two passes (or, two stages). In the first pass, FII compares only a [small number of pixel locations](https://gitlab.com/vgg/fii/-/blob/master/src/fii.h#L57) (i.e. 225 pixel locations) spread all over the image to create a candidate list of potential identical images. This allows FII to discard a large number of images from more exhaustive comparison in the next pass. In the second pass, FII performs exhaustive comparison (i.e. compare all pixel values) of all potential identical images identified in the first pass. Comparison stops early if any of the pixel mismatch. This two pass approach not only speeds up the process but also reduces the computer memory required to perform comparisons on a large set of images.

The exhaustive comparison of pixels (i.e. the second pass) is enabled using the --check-all-pixels flag in the FII command. Without this flag, FII runs much faster and requires less memory but it may result in some false positives. For example, in the vggface2 dataset, we found that FII wrongly identified 5 images as being identical. On visual inspection of the difference image for these 5 image pairs, we found that these image pairs differ only in few pixel locations and the difference is often due to pixel level artefacts caused by [image editing or image watermark](https://www.robots.ox.ac.uk/~vgg/software/fii/check-all-pixels-example.html). So, users should run the FII command with --check-all-pixels flag if they do not want any false positives and if they can tolerate slower processing speed and larger memory requirement.

## Developer's Resources
The [source code](https://gitlab.com/vgg/fii) of FII is release under BSD 2-Clause "Simplified" License and is available at the: [https://gitlab.com/vgg/fii](https://gitlab.com/vgg/fii). See [For-Developers.md](For-Developers.md) file for more details.


## Citation

If you use this software, please cite it as follows:

Abhishek Dutta and Andrew Zisserman. 2021. Find Identical Images (FII). http://www.robots.ox.ac.uk/~vgg/software/fii/.

```
@misc{dutta2021fii,
author = "Dutta, A. and Zissermann, A.",
title = "Find Identical Image ({FII})",
year = "2021",
howpublished = "http://www.robots.ox.ac.uk/~vgg/software/fii/",
note = "Version: X.Y.Z, Accessed: INSERT_DATE_HERE"
}
``` 

## Contact
Contact [Abhishek Dutta](adutta@robots.ox.ac.uk) for any queries or feedback related to this application. Use the [gitlab issues portal](https://gitlab.com/vgg/fii/-/issues) (requires signup) to report issues related to this software.