"""
Copyright (c) 2021 - University of Liège
Anthony Cioppa (anthony.cioppa@uliege.be), University of Liège (ULiège), Montefiore Institute, TELIM.
All rights reserved - patented technology, software available under evaluation license (see LICENSE)
"""

from torch.utils.data import Dataset
import torch
import cv2
import os

class Video(Dataset):
	"""
	Video dataloader
	In a loop, accesses one by one the frames of the video
	"""

	def __init__(self, video_path):

		self.video = cv2.VideoCapture(video_path)

	def __getitem__(self,index):

		ret, frame = self.video.read()
		if ret:
			return torch.from_numpy(frame).transpose(0,2).transpose(1,2).type(torch.float)
		else:
			raise StopIteration

	def __len__(self):
		return int(self.video.get(cv2.CAP_PROP_FRAME_COUNT))


def CDNet_categories(dataset_dir):
	"""
	Stores the list of categories as string and the videos of each
	category in a dictionary.
	"""

	categories = sorted(os.listdir(dataset_dir), key=lambda v: v.upper())

	videos = dict()

	for category in categories:
		category_dir = os.path.join(dataset_dir, category)
		videos[category] = sorted(os.listdir(category_dir), key=lambda v: v.upper())

	return categories, videos