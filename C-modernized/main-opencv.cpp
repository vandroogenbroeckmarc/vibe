/**
 * @file main-opencv.cpp
 * @date July 2014 (modernized 2026)
 * @brief An exemplative main file for the use of ViBe and OpenCV.
 *
 * Modernization (2026):
 *   - Dropped the OpenCV 1.x legacy headers (<opencv/cv.h>, <opencv/highgui.h>)
 *     — both were removed in OpenCV 4. Use the umbrella <opencv2/opencv.hpp>.
 *   - Used cv::COLOR_BGR2GRAY / cv::CAP_PROP_* enums instead of CV_* macros
 *     in comments/examples.
 *   - frameNumber is a plain local; the old file-scope static was unnecessary.
 */
#include <cstdlib>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "vibe-background-sequential.h"

using namespace cv;
using namespace std;

/** Function headers. */
static void processVideo(const char* videoFilename);

/** Displays instructions on how to use this program. */
static void help()
{
  cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use ViBe with OpenCV                            " << endl
    << "Usage:"                                                                     << endl
    << "./main-opencv <video filename>"                                             << endl
    << "for example: ./main-opencv video.avi"                                       << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

/**
 * Main program. It shows how to use the grayscale version (C1R) and the RGB version (C3R).
 */
int main(int argc, char* argv[])
{
  help();

  if (argc != 2) {
    cerr << "Incorrect input"  << endl;
    cerr << "exiting..."        << endl;
    return EXIT_FAILURE;
  }

  namedWindow("Frame");
  namedWindow("Segmentation by ViBe");

  processVideo(argv[1]);

  destroyAllWindows();
  return EXIT_SUCCESS;
}

/**
 * Processes the video. The code of ViBe is included here.
 *
 * @param videoFilename  The name of the input video file.
 */
static void processVideo(const char* videoFilename)
{
  VideoCapture capture(videoFilename);

  if (!capture.isOpened()) {
    cerr << "Unable to open video file: " << videoFilename << endl;
    std::exit(EXIT_FAILURE);
  }

  int frameNumber   = 1;
  Mat frame;
  Mat segmentationMap;
  int keyboard      = 0;   /* ESC or 'q' quits. */

  vibeModel_Sequential_t* model = nullptr;

  while ((char)keyboard != 'q' && keyboard != 27) {
    if (!capture.read(frame)) {
      cerr << "Unable to read next frame." << endl;
      cerr << "Exiting..."                 << endl;
      break;
    }

    if ((frameNumber % 100) == 0) {
      cout << "Frame number = " << frameNumber << endl;
    }

    /* Applying ViBe.
     * If you want to use the grayscale version of ViBe (which is much faster!):
     *   (1) replace every C3R by C1R in this file,
     *   (2) uncomment the next line.
     */
    /* cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY); */

    if (frameNumber == 1) {
      segmentationMap = Mat(frame.rows, frame.cols, CV_8UC1);
      model = libvibeModel_Sequential_New();
      libvibeModel_Sequential_AllocInit_8u_C3R(model, frame.data, frame.cols, frame.rows);
    }

    /* ViBe: segmentation + update. */
    libvibeModel_Sequential_Segmentation_8u_C3R(model, frame.data, segmentationMap.data);
    libvibeModel_Sequential_Update_8u_C3R     (model, frame.data, segmentationMap.data);

    /* Optional post-processing (3x3 median filter). */
    medianBlur(segmentationMap, segmentationMap, 3);

    imshow("Frame",                 frame);
    imshow("Segmentation by ViBe",  segmentationMap);

    ++frameNumber;

    keyboard = waitKey(1);
  }

  capture.release();
  libvibeModel_Sequential_Free(model);
}
