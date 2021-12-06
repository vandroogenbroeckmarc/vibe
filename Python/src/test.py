from model import ViBe
import torch
import cv2
import numpy as np
import time 
from dataloader import Video, CDNet_categories
from tqdm import tqdm
import os
import evaluate

device = torch.device("cuda:0")
video_path = "a.mkv"
dataset = "/"
display = False
vibe = ViBe(device)
dataset = "../../dataset2014/dataset/"
median=9

"""
video = Video(video_path)
if display:
    # Display the video
    cv2.namedWindow("display")


with tqdm(enumerate(video), total=len(video), ncols=120) as t:
        for i, (image) in t:
            image = image.to(device)
            if i == 0:
                vibe.initialize(image)
            mask = vibe.segment(image)
            if display:
                cv2.imshow("display", mask.type(torch.uint8).to("cpu").numpy()*255)
                cv2.waitKey(1)
"""
# Get the names of the categories and the videos


"""
start = torch.cuda.Event(enable_timing=True)
end = torch.cuda.Event(enable_timing=True)
while ret:
    # Time = 0

    image = torch.from_numpy(frame).transpose(0,2).transpose(1,2).type(torch.float).to(device)

    #torch.cuda.synchronize(device)
    #start.record()
    mask = vibe.segment(image)
    #end.record()
    #torch.cuda.synchronize(device)
    #print("In update", start.elapsed_time(end))
    
    if save_path is None:
        cv2.imshow("display", mask.type(torch.uint8).to("cpu").numpy()*255)
        cv2.waitKey(1)
    
    ret, frame = video.read()
"""