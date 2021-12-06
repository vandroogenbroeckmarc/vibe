/* Copyright - Benjamin Laugraud <blaugraud@ulg.ac.be> - 2016
 * Copyright - Marc Van Droogenbroeck <m.vandroogenbroeck@ulg.ac.be> - 2016
 *
 * ViBe is covered by a patent (see http://www.telecom.ulg.ac.be/research/vibe).
 *
 * Permission to use ViBe without payment of fee is granted for nonprofit
 * educational and research purposes only.
 *
 * This work may not be copied or reproduced in whole or in part for any
 * purpose.
 *
 * Copying, reproduction, or republishing for any purpose shall require a
 * license. Please contact the authors in such cases. All the code is provided
 * without any guarantee.
 *
 * This simple example program takes a path to a video sequence as an argument.
 * When it is executed, two windows are opened: one displaying the input
 * sequence, and one displaying the segmentation maps produced by ViBe. Note
 * that this program uses the monochromatic version of ViBe.
 */
#include <cstddef>
#include <ctime>
#include <iostream>

#ifndef    OPENCV_3
#include <cv.h>
#include <highgui.h>
#else
#include <opencv2/opencv.hpp>
#endif  /* OPENCV_3 */

#include <libvibe++/ViBe.h>
#include <libvibe++/distances/Manhattan.h>
#include <libvibe++/system/types.h>

using namespace std;
using namespace cv;
using namespace ViBe;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "A video file must be given as an argument to the program!";
    cerr << endl;

    return EXIT_FAILURE;
  }

  /* Parameterization of ViBe. */
  typedef ViBeSequential<1, Manhattan<1> >                                ViBe;

  /* Random seed. */
  srand(time(NULL));

  cv::VideoCapture decoder(argv[1]);
  cv::Mat frame;

  int32_t height = decoder.get(CV_CAP_PROP_FRAME_HEIGHT);
  int32_t width  = decoder.get(CV_CAP_PROP_FRAME_WIDTH);

  ViBe* vibe = NULL;
  cv::Mat bwFrame(height, width, CV_8UC1);
  cv::Mat segmentationMap(height, width, CV_8UC1);
  bool firstFrame = true;

  while (decoder.read(frame)) {
    cv::cvtColor(frame, bwFrame, CV_RGB2GRAY);

    if (firstFrame) {
      /* Instantiation of ViBe. */
      vibe = new ViBe(height, width, bwFrame.data);
      firstFrame = false;
    }

    /* Segmentation and update. */
    vibe->segmentation(bwFrame.data, segmentationMap.data);
    vibe->update(bwFrame.data, segmentationMap.data);

    /* Post-processing: 3x3 median filter. */
    medianBlur(segmentationMap, segmentationMap, 3);

    imshow("Input video", bwFrame);
    imshow("Segmentation by ViBe", segmentationMap);

    cvWaitKey(1);
  }

  delete vibe;

  cvDestroyAllWindows();
  decoder.release();

  return EXIT_SUCCESS;
}
