/* Copyright - Marc Van Droogenbroeck - 2026
 *
 * ViBe CLI — shared between ViBe_8UC1.cpp and ViBe_8UC3.cpp.
 *
 * Matches, one-for-one, the CLI of the Python port
 * (python-from-c++/main.py), so users who learned the Python interface can
 * invoke the C++ example programs the same way:
 *
 *   [-h] [--color] [--no-display] [--output OUTPUT] [--no-median]
 *   [--samples SAMPLES] [--threshold THRESHOLD] [--matches MATCHES]
 *   [--update-factor UPDATE_FACTOR] [--seed SEED] [--max-frames MAX_FRAMES]
 *   VIDEO
 *
 * Design notes
 * ------------
 *  * Header-only, no external dependencies beyond OpenCV and the C++17
 *    standard library (std::filesystem).
 *  * --samples is accepted but not honored: libvibe++ hard-codes
 *    DEFAULT_NUMBER_OF_SAMPLES = 30 as a static const in ViBeBase, with no
 *    public setter and no runtime reallocation of the sample buffer. The
 *    CLI prints a warning when the flag is used. Every other option is
 *    fully wired through.
 *  * --color is accepted but is a no-op at the per-binary level: ViBe_8UC1
 *    is already the grayscale variant and ViBe_8UC3 the color one. The
 *    flag exists only so the two CLIs accept the exact same argv the
 *    Python driver does.
 */
#ifndef _LIB_VIBE_XX_MAIN_VIBE_CLI_HPP_
#define _LIB_VIBE_XX_MAIN_VIBE_CLI_HPP_

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include <opencv2/opencv.hpp>

namespace vibe_cli {

// ---------------------------------------------------------------------------
// Option struct
// ---------------------------------------------------------------------------

struct Options {
  std::string video;                 // positional argument
  std::string output;                 // --output / -o   (empty = disabled)
  bool        color         = false;  // --color         (no-op in per-binary)
  bool        no_display    = false;  // --no-display
  bool        no_median     = false;  // --no-median
  bool        benchmark     = false;  // --benchmark     (pure-algorithm timing)
  int         samples       = -1;     // --samples       (warning only)
  int         threshold     = -1;     // --threshold
  int         matches       = -1;     // --matches
  int         update_factor = -1;     // --update-factor
  long        seed          = -1;     // --seed          (-1 = time-based)
  int         max_frames    = -1;     // --max-frames
};

// ---------------------------------------------------------------------------
// Argument parser
// ---------------------------------------------------------------------------

inline void printUsage(const char* prog, std::ostream& os = std::cout) {
  os <<
    "Usage: " << prog << " [OPTIONS] VIDEO\n"
    "\n"
    "Run ViBe background subtraction on a video file\n"
    "(e.g. an MPEG .mpg file, .mp4, .avi, .mkv, ...).\n"
    "\n"
    "Positional arguments:\n"
    "  VIDEO                  Path to the input. EITHER a video file\n"
    "                         (.mp4, .avi, .mov, ...) OR a directory of\n"
    "                         JPEG/PNG frames named\n"
    "                         <radical><digits>.<jpg|jpeg|png>. The digit\n"
    "                         run is sorted numerically, so 'in9.jpg'\n"
    "                         precedes 'in10.jpg' even when padding widths\n"
    "                         differ.\n"
    "\n"
    "Optional arguments:\n"
    "  -h, --help             Show this help message and exit.\n"
    "  --color                Accepted for CLI compatibility with the Python\n"
    "                         port. This binary is already the "
#ifdef VIBE_CLI_CHANNELS
    << (VIBE_CLI_CHANNELS == 3 ? "3-channel color\n" : "1-channel grayscale\n") <<
#else
    "grayscale/color\n"
#endif
    "                         variant; the flag is a no-op here.\n"
    "  --no-display           Do not open any OpenCV windows (headless run).\n"
    "  -o, --output OUTPUT    Write the segmentation masks. If OUTPUT has a\n"
    "                         video extension (.mp4, .avi, .mkv, .mov, .mpg,\n"
    "                         .mpeg, .m4v) a video is encoded; if OUTPUT is\n"
    "                         a directory (or ends with '/') the masks are\n"
    "                         written as mask_NNNNNN.png files.\n"
    "  --no-median            Skip the 3x3 median filter post-processing.\n"
    "  --benchmark            Pure-algorithm timing mode. Implies\n"
    "                         --no-display and --no-median; any --output\n"
    "                         is ignored. At the end of the run prints a\n"
    "                         detailed report: total wall time, FPS, and\n"
    "                         per-frame (segmentation + update) min, max,\n"
    "                         mean, median, stdev, p95, p99 in ms.\n"
    "  --samples N            Override the number of samples per pixel\n"
    "                         (not honored by this binary; the compiled\n"
    "                         library fixes this to 30 — warning only).\n"
    "  --threshold N          Override the matching threshold (default 10).\n"
    "  --matches N            Override the number of matches required\n"
    "                         (default 2).\n"
    "  --update-factor N      Override the model subsampling factor\n"
    "                         (default 8).\n"
    "  --seed N               Seed for rand(). Default: time-based.\n"
    "  --max-frames N         Stop after processing this many frames\n"
    "                         (useful for timing / testing).\n"
    "\n"
    "Exit codes: 0 on success, non-zero on argument or runtime error.\n";
}

namespace detail {

inline bool parseInt(const std::string& s, int& out) {
  try {
    size_t pos = 0;
    long v = std::stol(s, &pos);
    if (pos != s.size()) { return false; }
    out = static_cast<int>(v);
    return true;
  } catch (...) { return false; }
}

inline bool parseLong(const std::string& s, long& out) {
  try {
    size_t pos = 0;
    long v = std::stol(s, &pos);
    if (pos != s.size()) { return false; }
    out = v;
    return true;
  } catch (...) { return false; }
}

inline bool needsValue(const std::string& opt) {
  static const std::unordered_set<std::string> kWithValue = {
    "--output", "-o",
    "--samples", "--threshold", "--matches",
    "--update-factor", "--seed", "--max-frames",
  };
  return kWithValue.count(opt) != 0;
}

} // namespace detail

// Parse argv into `opts`.
//   Return values:
//     > 0  -> success; run the program
//     == 0 -> --help was requested; caller should exit cleanly (code 0)
//     < 0  -> parse error; caller should exit with EXIT_FAILURE
inline int parseArgs(int argc, char** argv, Options& opts) {
  const char* prog = (argc > 0 && argv[0]) ? argv[0] : "ViBe";

  auto expectValue = [&](int& i, const std::string& opt) -> bool {
    if (i + 1 >= argc) {
      std::cerr << prog << ": error: option '" << opt
                << "' requires a value.\n";
      return false;
    }
    ++i;
    return true;
  };

  std::vector<std::string> positional;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      printUsage(prog);
      return 0;
    }

    // Long options of the form --key=value are also supported.
    std::string key = arg;
    std::string valueInline;
    bool hasInline = false;
    if (arg.rfind("--", 0) == 0) {
      auto eq = arg.find('=');
      if (eq != std::string::npos) {
        key = arg.substr(0, eq);
        valueInline = arg.substr(eq + 1);
        hasInline = true;
      }
    }

    auto getValue = [&](std::string& out, const std::string& opt) -> bool {
      if (hasInline) { out = valueInline; return true; }
      if (!expectValue(i, opt)) { return false; }
      out = argv[i];
      return true;
    };

    if (key == "--color") {
      opts.color = true;
    } else if (key == "--no-display") {
      opts.no_display = true;
    } else if (key == "--no-median") {
      opts.no_median = true;
    } else if (key == "--benchmark") {
      opts.benchmark = true;
    } else if (key == "-o" || key == "--output") {
      if (!getValue(opts.output, key)) { return -1; }
    } else if (key == "--samples") {
      std::string v;
      if (!getValue(v, key) || !detail::parseInt(v, opts.samples)) {
        std::cerr << prog << ": error: --samples expects an integer.\n";
        return -1;
      }
    } else if (key == "--threshold") {
      std::string v;
      if (!getValue(v, key) || !detail::parseInt(v, opts.threshold)) {
        std::cerr << prog << ": error: --threshold expects an integer.\n";
        return -1;
      }
    } else if (key == "--matches") {
      std::string v;
      if (!getValue(v, key) || !detail::parseInt(v, opts.matches)) {
        std::cerr << prog << ": error: --matches expects an integer.\n";
        return -1;
      }
    } else if (key == "--update-factor") {
      std::string v;
      if (!getValue(v, key) || !detail::parseInt(v, opts.update_factor)) {
        std::cerr << prog << ": error: --update-factor expects an integer.\n";
        return -1;
      }
    } else if (key == "--seed") {
      std::string v;
      if (!getValue(v, key) || !detail::parseLong(v, opts.seed)) {
        std::cerr << prog << ": error: --seed expects an integer.\n";
        return -1;
      }
    } else if (key == "--max-frames") {
      std::string v;
      if (!getValue(v, key) || !detail::parseInt(v, opts.max_frames)) {
        std::cerr << prog << ": error: --max-frames expects an integer.\n";
        return -1;
      }
    } else if (!arg.empty() && arg[0] == '-' && arg != "-") {
      std::cerr << prog << ": error: unknown option '" << arg << "'.\n"
                << "Use -h / --help for usage.\n";
      return -1;
    } else {
      positional.push_back(arg);
    }
  }

  if (positional.empty()) {
    std::cerr << prog << ": error: missing VIDEO argument.\n"
              << "Use -h / --help for usage.\n";
    return -1;
  }
  if (positional.size() > 1) {
    std::cerr << prog << ": error: expected exactly one VIDEO argument, got "
              << positional.size() << ".\n";
    return -1;
  }

  opts.video = positional.front();

  if (opts.samples > 0) {
    std::cerr << prog << ": warning: --samples is not honored by this "
              << "binary (libvibe++ fixes DEFAULT_NUMBER_OF_SAMPLES = 30 in "
              << "ViBeBase.h without a public setter). Value ignored.\n";
  }

  // --benchmark implies: no display, no median post-processing, no output.
  if (opts.benchmark) {
    opts.no_display = true;
    opts.no_median  = true;
    if (!opts.output.empty()) {
      std::cerr << prog << ": warning: --benchmark disables disk output; "
                << "ignoring --output " << opts.output << ".\n";
      opts.output.clear();
    }
  }

  return 1;
}

// ---------------------------------------------------------------------------
// FrameSource: video file OR directory of ordered JPEG/PNG frames
// ---------------------------------------------------------------------------
//
// Both binaries accept either a video file or a directory containing image
// frames named  <radical><digits>.<jpg|jpeg|png>  (case-insensitive). Frames
// are sorted by (radical, integer-value-of-digit-run) so that, regardless
// of zero-padding width, "in9.jpg", "in009.jpg" and "in0010.jpg" all sort
// numerically (9, 10, 10), not lexicographically.

class FrameSource {
  public:
    virtual ~FrameSource() = default;
    virtual bool   isOpened() const = 0;
    virtual bool   read(cv::Mat& frame) = 0;
    virtual int    width()  const = 0;
    virtual int    height() const = 0;
    virtual double fps()    const = 0;
};

class VideoFileSource final : public FrameSource {
  public:
    explicit VideoFileSource(const std::string& path) : cap_(path) {}
    bool isOpened() const override { return cap_.isOpened(); }
    bool read(cv::Mat& f) override  { return cap_.read(f); }
    int  width()  const override {
      return static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
    }
    int  height() const override {
      return static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));
    }
    double fps() const override { return cap_.get(cv::CAP_PROP_FPS); }

  private:
    mutable cv::VideoCapture cap_;
};

class ImageDirSource final : public FrameSource {
  public:
    explicit ImageDirSource(const std::string& dir) {
      namespace fs = std::filesystem;
      static const std::regex re(
        R"(^(.*?)(\d+)\.(jpg|jpeg|png)$)",
        std::regex::icase
      );
      if (!fs::is_directory(dir)) return;

      // entries: (radical, numeric-value, full-path)
      std::vector<std::tuple<std::string, long long, std::string>> entries;
      for (const auto& de : fs::directory_iterator(dir)) {
        if (!de.is_regular_file()) continue;
        const std::string name = de.path().filename().string();
        std::smatch m;
        if (std::regex_match(name, m, re)) {
          try {
            entries.emplace_back(
              m[1].str(),
              std::stoll(m[2].str()),
              de.path().string()
            );
          } catch (const std::exception&) {
            // Overflow on absurdly long digit runs -- ignore the file.
          }
        }
      }
      std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b) {
          if (std::get<0>(a) != std::get<0>(b))
            return std::get<0>(a) < std::get<0>(b);
          return std::get<1>(a) < std::get<1>(b);
        });

      files_.reserve(entries.size());
      for (auto& e : entries) files_.push_back(std::get<2>(e));

      if (!files_.empty()) {
        const cv::Mat sample = cv::imread(files_.front());
        if (!sample.empty()) {
          w_ = sample.cols;
          h_ = sample.rows;
        }
      }
    }

    bool isOpened() const override { return !files_.empty(); }

    bool read(cv::Mat& f) override {
      if (idx_ >= files_.size()) return false;
      f = cv::imread(files_[idx_++]);
      return !f.empty();
    }

    int width()  const override { return w_; }
    int height() const override { return h_; }
    double fps() const override { return 0.0; }  // not meaningful

  private:
    std::vector<std::string> files_;
    std::size_t              idx_ = 0;
    int                      w_ = 0;
    int                      h_ = 0;
};

inline std::unique_ptr<FrameSource> makeSource(const std::string& path) {
  namespace fs = std::filesystem;
  std::error_code ec;
  if (fs::is_directory(path, ec)) {
    return std::make_unique<ImageDirSource>(path);
  }
  return std::make_unique<VideoFileSource>(path);
}

// ---------------------------------------------------------------------------
// Sink: video writer OR folder-of-PNGs
// ---------------------------------------------------------------------------

class Sink {
  public:
    enum class Kind { None, Video, Folder };

    Sink() = default;

    bool open(const std::string& output,
              double fps, int width, int height,
              std::ostream& log = std::cerr) {
      kind_ = Kind::None;
      frame_idx_ = 0;
      folder_.clear();
      writer_.release();

      if (output.empty()) {
        return true;
      }

      namespace fs = std::filesystem;

      const bool looksLikeFolder =
        !output.empty() &&
        (output.back() == '/'
         || output.back() == fs::path::preferred_separator
         || fs::is_directory(fs::path(output)));

      if (looksLikeFolder) {
        std::error_code ec;
        fs::create_directories(output, ec);
        if (ec) {
          log << "[vibe_cli] could not create output folder '"
              << output << "': " << ec.message() << "\n";
          return false;
        }
        kind_ = Kind::Folder;
        folder_ = output;
        return true;
      }

      // Video mode: recognized extension?
      std::string ext;
      {
        auto pos = output.find_last_of('.');
        if (pos != std::string::npos) { ext = output.substr(pos); }
      }
      std::transform(ext.begin(), ext.end(), ext.begin(),
                     [](unsigned char c) { return std::tolower(c); });

      static const std::unordered_set<std::string> videoExts = {
        ".mp4", ".avi", ".mkv", ".mov", ".mpg", ".mpeg", ".m4v"
      };
      if (videoExts.count(ext) != 0) {
        int fourcc = (ext == ".mp4")
                       ? cv::VideoWriter::fourcc('m','p','4','v')
                       : cv::VideoWriter::fourcc('X','V','I','D');
        const double realFps = (fps > 0.0) ? fps : 25.0;
        writer_.open(output, fourcc, realFps,
                     cv::Size(width, height), /*isColor=*/false);
        if (!writer_.isOpened()) {
          log << "[vibe_cli] could not open video writer for '"
              << output << "'; falling back to PNG folder.\n";
          const std::string fallback = output.substr(0, output.size() - ext.size())
                                     + "_masks";
          std::error_code ec;
          fs::create_directories(fallback, ec);
          if (ec) {
            log << "[vibe_cli] could not create fallback folder '"
                << fallback << "': " << ec.message() << "\n";
            return false;
          }
          kind_ = Kind::Folder;
          folder_ = fallback;
          return true;
        }
        kind_ = Kind::Video;
        return true;
      }

      // Anything else — treat as a directory target.
      std::error_code ec;
      fs::create_directories(output, ec);
      if (ec) {
        log << "[vibe_cli] could not create output folder '"
            << output << "': " << ec.message() << "\n";
        return false;
      }
      kind_ = Kind::Folder;
      folder_ = output;
      return true;
    }

    void write(const cv::Mat& mask) {
      if (kind_ == Kind::Video) {
        writer_.write(mask);
      } else if (kind_ == Kind::Folder) {
        char name[64];
        std::snprintf(name, sizeof(name), "mask_%06d.png", frame_idx_);
        const std::filesystem::path p =
          std::filesystem::path(folder_) / name;
        cv::imwrite(p.string(), mask);
      }
      ++frame_idx_;
    }

    void release() {
      if (kind_ == Kind::Video) { writer_.release(); }
    }

    Kind kind() const { return kind_; }
    int  frames_written() const { return frame_idx_; }

  private:
    Kind             kind_      = Kind::None;
    cv::VideoWriter  writer_;
    std::string      folder_;
    int              frame_idx_ = 0;
};

// ---------------------------------------------------------------------------
// Benchmark statistics
// ---------------------------------------------------------------------------
//
// BenchStats holds aggregate timing for a sequence of per-frame core-ViBe
// durations (segmentation + update, in milliseconds). Produced by
// `computeStats`; formatted by `printBenchStats`.

struct BenchStats {
  int    n           = 0;      // sample count
  double total_ms    = 0.0;    // sum of all samples (core time only)
  double min_ms      = 0.0;
  double max_ms      = 0.0;
  double mean_ms     = 0.0;
  double median_ms   = 0.0;
  double stddev_ms   = 0.0;
  double p95_ms      = 0.0;
  double p99_ms      = 0.0;
  double wall_ms     = 0.0;    // end-to-end wall clock (set by caller)
  double fps_core    = 0.0;    // 1000 / mean_ms
  double fps_wall    = 0.0;    // n * 1000 / wall_ms
};

// Compute a BenchStats from a vector of per-frame durations (ms).
// `samples_ms` is taken by value so we can sort it in place.
inline BenchStats computeStats(std::vector<double> samples_ms, double wall_ms) {
  BenchStats s;
  s.n = static_cast<int>(samples_ms.size());
  s.wall_ms = wall_ms;
  if (s.n == 0) { return s; }

  std::sort(samples_ms.begin(), samples_ms.end());

  s.min_ms = samples_ms.front();
  s.max_ms = samples_ms.back();

  double sum = 0.0;
  for (double v : samples_ms) { sum += v; }
  s.total_ms = sum;
  s.mean_ms = sum / s.n;

  // Median (linear interpolation of central values for even n).
  if (s.n % 2 == 1) {
    s.median_ms = samples_ms[s.n / 2];
  } else {
    s.median_ms = 0.5 * (samples_ms[s.n / 2 - 1] + samples_ms[s.n / 2]);
  }

  // Sample standard deviation.
  double acc = 0.0;
  for (double v : samples_ms) {
    const double d = v - s.mean_ms;
    acc += d * d;
  }
  s.stddev_ms = (s.n > 1) ? std::sqrt(acc / (s.n - 1)) : 0.0;

  // Percentiles (nearest-rank method).
  auto pct = [&](double p) -> double {
    const int rank =
      std::max(1, static_cast<int>(std::ceil(p * s.n))) - 1;
    return samples_ms[std::min(rank, s.n - 1)];
  };
  s.p95_ms = pct(0.95);
  s.p99_ms = pct(0.99);

  s.fps_core = (s.mean_ms > 0.0) ? 1000.0 / s.mean_ms : 0.0;
  s.fps_wall = (wall_ms > 0.0)   ? 1000.0 * s.n / wall_ms : 0.0;

  return s;
}

// Pretty-printer for the benchmark report.
inline void printBenchStats(const BenchStats& s,
                            const std::string& label,
                            std::ostream& os = std::cout) {
  const auto flags = os.flags();
  const auto prec  = os.precision();
  os << std::fixed << std::setprecision(3);

  os << "\n"
     << "================================================================\n"
     << "ViBe benchmark report"
     << (label.empty() ? "" : std::string("  [") + label + "]") << "\n"
     << "================================================================\n";
  if (s.n == 0) {
    os << "  (no frames processed)\n";
  } else {
    os << "  Frames processed             : " << s.n                        << "\n"
       << "  Wall clock (end-to-end)      : " << s.wall_ms          << " ms\n"
       << "  Core time (sum segm.+update) : " << s.total_ms         << " ms\n"
       << "  Throughput (wall clock)      : " << s.fps_wall         << " fps\n"
       << "  Throughput (core time only)  : " << s.fps_core         << " fps\n"
       << "  Per-frame core time [ms]:\n"
       << "     min                       : " << s.min_ms           << "\n"
       << "     max                       : " << s.max_ms           << "\n"
       << "     mean                      : " << s.mean_ms          << "\n"
       << "     median                    : " << s.median_ms        << "\n"
       << "     stdev (sample)            : " << s.stddev_ms        << "\n"
       << "     p95                       : " << s.p95_ms           << "\n"
       << "     p99                       : " << s.p99_ms           << "\n";
  }
  os << "================================================================\n";

  os.flags(flags);
  os.precision(prec);
}

} // namespace vibe_cli

#endif /* _LIB_VIBE_XX_MAIN_VIBE_CLI_HPP_ */
