/* Copyright - Benjamin Laugraud - 2016
 * Copyright - Marc Van Droogenbroeck - 2016
 *
 * ViBe was originally covered by a patent that is now in the public domain.
 * See the accompanying LICENSE file for details.
 *
 * Color (8UC3 = 3-channel) ViBe example.
 *
 * Usage (identical to python-from-c++/main.py, minus --color which is a
 * no-op here since this binary IS the color variant):
 *
 *   ViBe_8UC3 [-h] [--color] [--no-display] [--output OUTPUT]
 *             [--no-median] [--samples SAMPLES] [--threshold THRESHOLD]
 *             [--matches MATCHES] [--update-factor UPDATE_FACTOR]
 *             [--seed SEED] [--max-frames MAX_FRAMES] VIDEO
 *
 * Modernization (2026):
 *   - Dropped the OPENCV_3 ifdef / legacy cv.h / highgui.h headers; always
 *     include <opencv2/opencv.hpp> (OpenCV 4.x).
 *   - Replaced deprecated CV_* enum macros and the legacy C API with cv::*.
 *   - Replaced raw new/delete with std::unique_ptr for exception safety.
 *   - ESC / Q closes the windows cleanly.
 *   - Full CLI parity with the Python driver via vibe_cli.hpp.
 */
#define VIBE_CLI_CHANNELS 3

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>

#include <opencv2/opencv.hpp>

#include <libvibe++/ViBe.h>
#include <libvibe++/distances/Manhattan.h>
#include <libvibe++/system/types.h>

#include "vibe_cli.hpp"

using namespace std;
using namespace cv;
using namespace ViBe;

int main(int argc, char** argv) {
  vibe_cli::Options opts;
  const int p = vibe_cli::parseArgs(argc, argv, opts);
  if (p == 0) { return EXIT_SUCCESS; }     // --help printed
  if (p < 0)  { return EXIT_FAILURE; }     // parse error

  /* Random seed. */
  if (opts.seed >= 0) {
    srand(static_cast<unsigned>(opts.seed));
  } else {
    srand(static_cast<unsigned>(time(nullptr)));
  }

  /* Parameterization of ViBe (3-channel / Manhattan). */
  using ViBeImpl = ViBeSequential<3, Manhattan<3>>;

  auto decoder = vibe_cli::makeSource(opts.video);
  if (!decoder->isOpened()) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::is_directory(opts.video, ec)) {
      cerr << "No matching frames in directory: " << opts.video << "\n"
           << "  (expected files like 'in0001.jpg', 'frame_010.png', ...)"
           << endl;
    } else {
      cerr << "Could not open video: " << opts.video << endl;
    }
    return EXIT_FAILURE;
  }

  cv::Mat frame;
  const int32_t height = decoder->height();
  const int32_t width  = decoder->width();
  const double  fps    = decoder->fps();

  vibe_cli::Sink sink;
  if (!sink.open(opts.output, fps, width, height)) {
    return EXIT_FAILURE;
  }

  std::unique_ptr<ViBeImpl> vibe;
  cv::Mat segmentationMap(height, width, CV_8UC1);

  int frames = 0;
  std::vector<double> coreTimesMs;
  if (opts.benchmark) { coreTimesMs.reserve(1024); }

  const auto t0 = std::chrono::steady_clock::now();

  while (decoder->read(frame)) {
    if (!vibe) {
      /* Instantiation of ViBe on the first frame. */
      vibe = std::make_unique<ViBeImpl>(height, width, frame.data);

      if (opts.threshold > 0) {
        vibe->setMatchingThreshold(opts.threshold);
      }
      if (opts.matches > 0) {
        vibe->setMatchingNumber(opts.matches);
      }
      if (opts.update_factor > 0) {
        vibe->setUpdateFactor(opts.update_factor);
      }

      std::cout << "[ViBe_8UC3] Parameters:" << std::endl;
      vibe->print(std::cout);
      std::cout << std::endl;
    }

    /* Segmentation and update. Time *only* this core-algorithm block when
     * --benchmark is active. */
    const auto cs = std::chrono::steady_clock::now();
    vibe->segmentation(frame.data, segmentationMap.data);
    vibe->update(frame.data, segmentationMap.data);
    const auto ce = std::chrono::steady_clock::now();
    if (opts.benchmark) {
      coreTimesMs.push_back(
        std::chrono::duration<double, std::milli>(ce - cs).count()
      );
    }

    /* Post-processing: 3x3 median filter. */
    if (!opts.no_median) {
      cv::medianBlur(segmentationMap, segmentationMap, 3);
    }

    sink.write(segmentationMap);

    if (!opts.no_display) {
      cv::imshow("Input video", frame);
      cv::imshow("Segmentation by ViBe", segmentationMap);
      const int key = cv::waitKey(1);
      if (key == 27 /* ESC */ || key == 'q') {
        break;
      }
    }

    ++frames;
    if (opts.max_frames > 0 && frames >= opts.max_frames) {
      break;
    }
  }

  const auto t1 = std::chrono::steady_clock::now();
  const double wall_ms =
    std::chrono::duration<double, std::milli>(t1 - t0).count();
  const double elapsed = wall_ms / 1000.0;
  const double realElapsed = elapsed > 0.0 ? elapsed : 1e-9;
  std::cout << "[ViBe_8UC3] Processed " << frames
            << " frames in " << elapsed << "s ("
            << (frames / realElapsed) << " FPS)" << std::endl;

  if (opts.benchmark) {
    const vibe_cli::BenchStats st =
      vibe_cli::computeStats(coreTimesMs, wall_ms);
    vibe_cli::printBenchStats(st, "ViBe_8UC3 / 3-channel / Manhattan<3>");
  }

  sink.release();
  if (!opts.no_display) {
    cv::destroyAllWindows();
  }
  // decoder is a std::unique_ptr<FrameSource>; its destructor at scope
  // exit releases the underlying cv::VideoCapture or file list cleanly.

  return EXIT_SUCCESS;
}
