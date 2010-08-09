///////////////////////////////////////////////////////////////////////////////
///
/// $Id: euspng.c $
///
/// Copyright (c) 1987- JSK, The University of Tokyo.  All Rights Reserved.
///
/// This software is a collection of EusLisp code for robot applications,
/// which has been developed by the JSK Laboratory for the IRT project.
/// For more information on EusLisp and it's application to the robotics,
/// please refer to the following papers.
///
/// Toshihiro Matsui
/// Multithread object-oriented language euslisp for parallel and
///  asynchronous programming in robotics
/// Workshop on Concurrent Object-based Systems,
///  IEEE 6th Symposium on Parallel and Distributed Processing, 1994
///
/// Permission to use this software for educational, research
/// and non-profit purposes, without fee, and without a written
/// agreement is hereby granted to all researchers working on
/// the IRT project at the University of Tokyo, provided that the
/// above copyright notice remains intact.  
///

#pragma init (register_euspng)

#include <png.h>
#include "eus.h"

extern pointer ___euspng();
static register_euspng()
{ add_module_initializer("___euspng", ___euspng);}

pointer PNG_READ_IMAGE(register context *ctx, int n, register pointer *argv)
{
  char *file_name;
  pointer ret, image_ptr;
  ckarg(2);
  if (isstring(argv[0])) file_name = argv[0]->c.str.chars;
  else error(E_NOSTRING);

  FILE *fp = fopen(file_name, "rb");
  if (!fp) {
    error(E_OPENFILE);
    return(NIL);
  }

  png_structp png_ptr;
  png_infop info_ptr;
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  info_ptr = png_create_info_struct(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    fclose(fp);
    error(E_EOF);
    return(NIL);
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 0);
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, png_voidp_NULL);
  int width = info_ptr->width;
  int height = info_ptr->height;
  int bit_depth = info_ptr->bit_depth;
  int channels = info_ptr->channels;
  png_bytep * row_pointers = png_get_rows(png_ptr, info_ptr);
  int y, byte_per_scanline = png_get_rowbytes(png_ptr, info_ptr);

  image_ptr = makestring((char *)(row_pointers[0]),height*byte_per_scanline);
  vpush(image_ptr);
  for(y=0;y<height;y++){
    memcpy(&(image_ptr->c.str.chars[y*byte_per_scanline]), row_pointers[y], byte_per_scanline);
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
  fclose(fp);

  ret=cons(ctx,image_ptr,NIL);
  ret=cons(ctx,makeint(channels),ret);
  ret=cons(ctx,makeint(height),ret);
  ret=cons(ctx,makeint(width),ret);

  return (ret);
}

pointer PNG_WRITE_IMAGE(register context *ctx, int n, register pointer *argv)
{
  char *file_name, *image_ptr;
  int width, height, channels;
  ckarg(5);
  if (isstring(argv[0])) file_name = argv[0]->c.str.chars;
  else error(E_NOSTRING);
  width  = ckintval(argv[1]);
  height = ckintval(argv[2]);
  channels  = ckintval(argv[3]);
  image_ptr = argv[4]->c.str.chars;
  fprintf(stderr, "%d %d %d %x\n", width, height, channels, image_ptr);
  FILE *fp = fopen(file_name, "wb");
  if (!fp) {
    error(E_OPENFILE);
    return(NIL);
  }

  png_structp png_ptr;
  png_infop info_ptr;
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  info_ptr = png_create_info_struct(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    error(E_EOF);
    return(NIL);
  }

  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, //GRAY
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  int y, byte_per_scanline = png_get_rowbytes(png_ptr, info_ptr);
  fprintf(stderr, "%d\n", byte_per_scanline);
  for(y=0;y<height;y++){
    row_pointers[y] = &(image_ptr[y*byte_per_scanline]);
  }
  png_set_rows(png_ptr, info_ptr, row_pointers);

  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, png_voidp_NULL);
  png_write_end(png_ptr, info_ptr);

  free(row_pointers);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(fp);

  return (T);
}

pointer ___euspng(register context *ctx, int n, register pointer *argv)
{
    pointer mod=argv[0];

    defun(ctx, "PNG-READ-IMAGE",  mod, PNG_READ_IMAGE);
    defun(ctx, "PNG-WRITE-IMAGE", mod, PNG_WRITE_IMAGE);
}

