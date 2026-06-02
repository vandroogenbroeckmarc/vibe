/**
  * @file your-main-file-sequential.c
  * \brief Shows how to use ViBe in your own C/C++ project
  *
  
  This file contains an example of a main functions that uses the ViBe algorithm
  implemented in vibe-background-sequential.{o, h}. You should read vibe-background-sequential.h for
  more information.

  Full documentation is available online at:
     http://www.ulg.ac.be/telecom/research/vibe/doc

  vibe-background.o was compiled by <tt>gcc</tt> using the following command
  \verbatim
  $> gcc -std=c99 -O3 -Wall -Werror -pedantic -Wno-unused-function -Wno-unused-parameter -Wno-deprecated -Wno-deprecated-declarations -Wno-sign-compare -Wno-unused-but-set-parameter -c vibe-background-sequential.c
  \endverbatim

  This file can be compiled using the following command
  \verbatim
  $> gcc -o main -std=c99 -O3 -Wall -Werror -pedantic your-main-file.c vibe-background-sequential.o
  \endverbatim

  * @date July 2014
  * @author Marc Van Droogenbroeck 
*/

#include "vibe-background-sequential.h"

static int32_t get_image_width(void *stream)
{
  /* Put your own code here. */
  return(640);
}

static int32_t get_image_height(void *stream)
{
  /* Put your own code here. */
  return(480);
}

static int32_t *acquire_image_C1R(void *stream, uint8_t *image_data, int32_t width, int32_t height)
{
  /* Put your own code here. */
  memset(image_data, 127, width * height); // Fills the image with the 127 value.
  return(0);
}

static int32_t *acquire_image_C3R(void *stream, uint8_t *image_data, int32_t width, int32_t height)
{
  /* Put your own code here. */
  memset(image_data, 127, (3 * width) * height); // Fills the image with the 127 value.
  return(0);
}

/* Simulates a condition to stop after 100 frames. */
static int32_t finished(void *stream)
{
  /* Put your own code here. */
  static int32_t counter = 0;
  return(!(counter++ < 100));
}

int main(int argc, char **argv)
{
  /* Your video stream. */
  void *stream = NULL;
  
  /* Get the dimensions of the images of your stream. */
  int32_t width = get_image_width(stream);
  int32_t height = get_image_height(stream);

  /* Allocates memory to store the input images and the segmentation maps. */
  uint8_t *image_data = NULL;
  uint8_t *segmentation_map = (uint8_t*)malloc(width * height);

  /* The pointer to store the ViBe's model. */
  vibeModel_Sequential_t *model = NULL;

  // ------------ This is for mono-channel images ( == C1R images) -------------
  image_data = (uint8_t*)malloc(width * height);

  /* Acquires your first image. */
  acquire_image_C1R(stream, image_data, width, height);

  /* Get a model data structure. */
  model = (vibeModel_Sequential_t *)libvibeModel_Sequential_New();

  /* Allocates the model and initialize it with the first image. */
  libvibeModel_Sequential_AllocInit_8u_C1R(model, image_data, width, height);
  
  /* Processes all the following frames of your stream: results are stored in "segmentation_map". */
  while (!finished(stream)) {
    fprintf(stderr, ".");
    acquire_image_C1R(stream, image_data, width, height);

    /* Segmentation step: produces the output mask. */
    libvibeModel_Sequential_Segmentation_8u_C1R(model, image_data, segmentation_map);

    /* Next, we update the model. This step is optional. */
    libvibeModel_Sequential_Update_8u_C1R(model, image_data, segmentation_map);

    /* segmentation_map is the binary output map that you would like to display, save or 
       use in your own application. Put your own code hereafter. */
  }

  fprintf(stderr, "\n");

  /* Cleanup allocated memory. */
  libvibeModel_Sequential_Free(model);
  free(image_data);

  // ----------- This is for three-channel images ( == C3R images) -------------
  /* Data is stored as RGBRGBRGB... or BGRBGRBGR... Three consecutives bytes per pixel thus. */
  image_data = (uint8_t*)malloc((3 * width) * height);

  /* Acquires your first image. */
  acquire_image_C3R(stream, image_data, width, height);

  /* Get a model data structure. */
  model = (vibeModel_Sequential_t*)libvibeModel_Sequential_New();

  /* Allocates the model and initialize it with the first image. */
  libvibeModel_Sequential_AllocInit_8u_C3R(model, image_data, width, height);

  /* Processes all the following frames of your stream: results are stored in "segmentation_map". */
  while (!finished(stream)) {
    fprintf(stderr, ".");
    acquire_image_C3R(stream, image_data, width, height);

    /* Segmentation step: produces the output mask. */
    libvibeModel_Sequential_Segmentation_8u_C3R(model, image_data, segmentation_map);

    /* Next, we update the model. This step is optional. */
    libvibeModel_Sequential_Update_8u_C3R(model, image_data, segmentation_map);

    /* segmentation_map is the binary output map that you would like to display, save or 
       use in your own application. Put your own code hereafter. */
  }

  fprintf(stderr, "\n");

  /* Cleanup allocated memory. */
  libvibeModel_Sequential_Free(model);
  free(image_data);

  // ---------------------------- General cleanup ------------------------------

  free(segmentation_map);
  return(0);
}
