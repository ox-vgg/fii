/*
load image size (width x height x nchannel) by loading and parsing the
image header only. Since image data is not loaded, this operation is
very fast (especially for very large images)

Author: Abhishek Dutta <http://abhishekdutta.org>

Revision History:
06-Dec-2020 : initial version based on stb_image.h @ b42009

*/

#ifndef FII_IMAGE_SIZE_H
#define FII_IMAGE_SIZE_H

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

void fii_image_size(const char *filename,
                    int *width,
                    int *height,
                    int *nchannel) {
  *width    = 0;
  *height   = 0;
  *nchannel = 0;

  FILE *f = stbi__fopen(filename, "rb");
  unsigned char *stb_op_result;
  if (!f) return;

  // source: stbi_load_from_file()
  stbi__context s;
  stbi__start_file(&s,f);

  // source: stbi__load_and_postprocess_8bit()
  // source: stbi__load_main()
  if (stbi__jpeg_test(&s)) {
    // source: stbi__jpeg_load()
    unsigned char* result;
    stbi__jpeg* j = (stbi__jpeg*) stbi__malloc(sizeof(stbi__jpeg));
    j->s = &s;
    stbi__setup_jpeg(j);
    // source: load_jpeg_image()
    // only load image size and NOT the image data
    int n, decode_n, is_rgb;
    j->s->img_n = 0;
    // source: stbi__decode_jpeg_image()
    int m;
    for (m = 0; m < 4; m++) {
      j->img_comp[m].raw_data = NULL;
      j->img_comp[m].raw_coeff = NULL;
    }
    j->restart_interval = 0;
    if(stbi__decode_jpeg_header(j, STBI__SCAN_header)) {
      *width    = j->s->img_x;
      *height   = j->s->img_y;
      *nchannel = j->s->img_n;
    }
    stbi__cleanup_jpeg(j);
    STBI_FREE(j);
  }
  if (stbi__png_test(&s)) {
    stbi__png p;
    p.s = &s;
    if (stbi__parse_png_file(&p, STBI__SCAN_header, 0)) {
      *width    = p.s->img_x;
      *height   = p.s->img_y;
      *nchannel = p.s->img_n;
    }
    STBI_FREE(p.out);      p.out      = NULL;
    STBI_FREE(p.expanded); p.expanded = NULL;
    STBI_FREE(p.idata);    p.idata    = NULL;
  }
  if (stbi__bmp_test(&s)) {
    // source: stbi__bmp_load()
    stbi__bmp_data info;
    info.all_a = 255;
    if (stbi__bmp_parse_header(&s, &info) != NULL) {
      unsigned int ma = info.ma;
      if (info.bpp == 24 && ma == 0xff000000) {
        s.img_n = 3;
      }
      else {
        s.img_n = ma ? 4 : 3;
      }
      *width    = s.img_x;
      *height   = s.img_y;
      *nchannel = s.img_n;
    }
  }

  // cleanup
  fclose(f);
}
#endif
