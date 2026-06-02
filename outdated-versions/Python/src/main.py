"""
Copyright (c) 2021 - University of Liège
Anthony Cioppa (anthony.cioppa@uliege.be), University of Liège (ULiège), Montefiore Institute, TELIM.
All rights reserved - patented technology, software available under evaluation license (see LICENSE)
"""

from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from dataloader import Video, CDNet_categories
import median_pool as mp
from model import ViBe
from tqdm import tqdm
import numpy as np
import evaluate
import torch
import cv2
import os

def CDNet_motion_detection(dataset_path, algorithm, device="cpu", median=None):
    """
    Loads the CDNet 2014 dataset downloaded from https://changedetection.net/
    and computes the change detection masks using the algorithm (ViBe)
    :param dataset_path: path to the CDNet 2014 dataset
    :param algorithm: the instance of ViBe
    :param device: referes to the device to use (typically "cpu" or "cuda")
    :param median: is the kernel size of the median filtering to use just before the evaluation
    :return: The mean F1 score over the entire dataset
    """

    # Retrieve the folder structure of CDNet 2014
    categories, videos = CDNet_categories(dataset_path)

    # Set up the confusion matrices for each video
    confusion_matrices = evaluate.ConfusionMatricesHolder(device, categories, videos, median)

    # Loop over all categories that were retrieved
    for category in categories:

        # Loop over all videos
        for video in videos[category]:

            # Set the Dataloader for the video and groundtruth 
            video_input = Video(os.path.join(dataset_path, category, video, "input/in%06d.jpg"))
            video_groundtruth = Video(os.path.join(dataset_path, category, video, "groundtruth/gt%06d.png"))

            # loop over the video
            with tqdm(enumerate(zip(video_input, video_groundtruth)), total=len(video_input), ncols=120) as t:
                    for i, (image, gt) in t:

                        # Put the image and groundtruth to the device
                        image = image.to(device)
                        gt = gt.to(device)

                        # Initialize ViBe with the first image
                        if i == 0:
                            vibe.initialize(image)

                        # Segment the current image and update the background model
                        mask = vibe.segment(image)

                        # Evaluate the new mask and store the results in the confusion matric
                        confusion_matrices.confusion_matrix[category][video].evaluate(mask, gt[0])
            
            print(category + " - ", video)
            print("F1: ", confusion_matrices.confusion_matrix[category][video].F1())

    print("Mean F1 Score", confusion_matrices.meanF1(categories, videos))

    return confusion_matrices.meanF1(categories, videos)

def video_motion_detection(video_path, algorithmn, device="cpu", median=None, save_path=None):
    """
    Loads a video and computes the change detection masks using the algorithm (ViBe)
    :param video_path: path to the video file
    :param algorithm: the instance of ViBe
    :param device: referes to the device to use (typically "cpu" or "cuda")
    :param median: is the kernel size of the median filtering to use just before the evaluation
    :param save_path: path to save the change detection masks as png. If none, the masks are displayed
    :return: nothing
    """

    # Set up the dataloader of the video
    video = Video(video_path)

    # Set up the median pooling layer if required
    MedianPool = None
    if median is not None:
        MedianPool = mp.MedianPool2d(kernel_size=int(median), same=True)

    # Display the video if no savepath is spoecified
    if save_path is None:
        cv2.namedWindow("display")

    # Loop over the video
    with tqdm(enumerate(video), total=len(video), ncols=120) as t:
            for i, (image) in t:

                # Send the image to the device
                image = image.to(device)

                # Initialize ViBe with the first image
                if i == 0:
                    vibe.initialize(image)

                # Segment the current image and update the background model
                mask = vibe.segment(image)

                # Compute the median filter operation on the mask
                if MedianPool is not None:
                    mask = MedianPool.forward(mask.unsqueeze_(0).unsqueeze_(0)).squeeze_(0).squeeze_(0)

                # Display the image
                if save_path is None:
                    cv2.imshow("display", mask.type(torch.uint8).to("cpu").numpy()*255)
                    cv2.waitKey(1)

                # Save the detection mask in the save folder
                else:
                    mask = mask*255
                    frame_numpy = (mask.to("cpu").numpy()).astype("uint8")
                    frame_path = save_path + "/bin" + (str(i+1)).zfill(6) + ".png"
                    cv2.imwrite(frame_path, frame_numpy)

def timing(video_path, algorithmn, device="cpu"):

    start = torch.cuda.Event(enable_timing=True)
    end = torch.cuda.Event(enable_timing=True)
    resolutions = ["144p","224p","360p", "576p", "720p","1080p","2160p"]

    for video_name in resolutions:

        video = Video(video_path + video_name + ".mp4")

        timing = 0
        # Loop over the video
        with tqdm(enumerate(video), total=len(video), ncols=120) as t:
                for i, (image) in t:

                    # Send the image to the device
                    image = image.to(device)

                    # Initialize ViBe with the first image
                    if i == 0:
                        vibe.initialize(image)

                    torch.cuda.synchronize()
                    # Segment the current image and update the background model
                    start.record()
                    mask = vibe.segment(image)
                    end.record()
                    torch.cuda.synchronize()
                    timing += start.elapsed_time(end)
                    

        print(video_name, " : ", timing/len(video), "ms/frame")
        print(video_name, " : ", 1000/(timing/len(video)), "fps")

if __name__ == '__main__':

    # Load the arguments
    parser = ArgumentParser(description='ViBe', formatter_class=ArgumentDefaultsHelpFormatter)

    parser.add_argument('--path',   required=True, type=str, help='Path to the video or CDNet dataset' )
    parser.add_argument('--savepath',   required=False, type=str, default = None, help='Path to save the masks' )
    parser.add_argument('--cdnet',   required=False, action='store_true',  help='Evaluation on CDNet' )
    parser.add_argument('--timing',   required=False, action='store_true',  help='Timing on the crossing videos' )
    parser.add_argument('--device',   required=False, type=str, default = "cpu", help='Device on which to run the computations' )
    parser.add_argument('--median',   required=False, type=int,   default=9,     help='Kernel size for the median filter' )

    parser.add_argument('--numberOfSamples',   required=False, type=int,   default=30,     help='ViBe parameter: number of samples per pixel in the background model' )
    parser.add_argument('--matchingThreshold',   required=False, type=int,   default=10,     help='ViBe parameter: threshold value to match the color values' )
    parser.add_argument('--matchingNumber',   required=False, type=int,   default=2,     help='ViBe parameter: minimal number of matches for a background classification' )
    parser.add_argument('--updateFactor',   required=False, type=int,   default=8,     help='ViBe parameter: update factor for the background model update' )
    parser.add_argument('--neighborhoodRadius',   required=False, type=int,   default=1,     help='ViBe parameter: maximal neighbour radius' )

    args = parser.parse_args()

    # Create the instance of ViBe
    vibe = ViBe(args.device, 
        numberOfSamples = args.numberOfSamples, 
        matchingThreshold = args.matchingThreshold, 
        matchingNumber = args.matchingNumber, 
        updateFactor = args.updateFactor, 
        neighborhoodRadius = args.neighborhoodRadius
        )

    # Definition of the device
    device = torch.device(args.device)

    # Parsing of the median filter kernel size
    median = args.median
    if median <= 0:
        median=None
    
    if args.timing:
        timing(args.path, vibe, device)
    elif args.cdnet:
        # Compute the mean F1 score on CDNet 2014
        CDNet_motion_detection(args.path, vibe, device, median)
    else:
        # Compute the motion masks on a video
        video_motion_detection(args.path, vibe, device, median, args.savepath)