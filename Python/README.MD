# ViBe - Pytorch

This repository contains a Pytorch implementation of the ViBe background subtraction algorithm with CPU and GPU support. 

<p align="center">
  <img src="../img/Python_logo.png" height="50"> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;
  <img src="../img/Pytorch_logo.png" height="50"> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;
  <img src="../img/Nvidia_logo.png" height="75"> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;
</p>

## Getting Started

The following instructions will help you install the required libraries and the dataset to run the code. The code runs in <code>python 3</code> and was tested on a laptop with a RTX2070Super GPU.

### Create environment

To create and setup the conda environment, simply follow these steps:

```bash
conda create -n vibe python=3.8
conda activate vibe
conda install pytorch=1.6 cudatoolkit=10.1 -c pytorch
pip install opencv-python
```

### Evaluation on CDNet 2014

First, you will need to download and extract the zip folder containing the CDNet 2014 dataset available on [changedetection.net](changedetection.net).

Then, to compute the mean F1 score on the dataset using GPU computations, simply run the following command line


```bash
python main.py --path path/to/CDNet2014/ --cdnet --device cuda
```

<p align="center">
  <img src="../img/BGS.png" width="480">
</p>

### Computing the masks on any video

To compute motion detection masks on a single video using GPU computations, simply run the following command line:


```bash
python main.py --path path/to/video/ --save_path path/to/save/masks/ --device cuda
```

The masks are saved in the the path passed to the <code>--save_path</code> argument.

### Timing on your device

Finally, we provide some code and data to evaluate the computation time of ViBe on your own device.
First download the data through this [link]().

Then run the following command line:

```bash
python main.py --path path/to/timing/data/ --timing --device cuda
```

Here are the results of our tests on two of our device for different resolutions:

|Framerate (FPS)|144p|224p|360p|576p|720p (HD)| 1080p (Full HD)| 2160p (4K)|
|---------------|----|----|----|----|----|----|----|
|DGX-1 (GPU Tesla V100)|488|387|266|150|106|55|15|
|Laptop (GPU RTX2070SuperMaxQ)|669|411|219|99|64|30|Out of memory|
|DGX-1 (CPU)|184|43|21|9|5|2|0.5|
|Laptop (CPU)|126|49|18|7|5|2|0.5|

It can be seen that this python implementation in pytorch is really suited for GPU.

## Authors

See the [AUTHORS](AUTHORS) file for details.


## License

Custom non-permissive.
This work is protected by several patents.

See the [LICENSE](LICENSE) file for details.

The implementation of the median pooling layer is taken from [rwightman](https://gist.github.com/rwightman/f2d3849281624be7c0f11c85c87c1598). It is used only as post-processing of the masks produced by ViBe for evaluation on CDNet2014.

## Acknowledgments

* Anthony Cioppa is funded by the FRIA, Belgium and ARIAC/TRAIL.
