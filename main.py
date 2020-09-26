import os
import cv2
import numpy as np



def ReadImage(InputImageFolderPath):
    '''
    This function will read all the images from the Folder path provided 
    and store the images and their names in 2 lists and return it.
    '''
    Images = []                     # Input Images will be stored in this list.
    ImageNames = []                 # Names of input images will be stored in this list.
    
    # Getting all image's name present inside the folder.
    for ImageName in os.listdir(InputImageFolderPath):
        InputImage = cv2.imread(InputImageFolderPath + "/" + ImageName)     # Reading images one by one.
        Images.append(InputImage)                                           # Storing images.
        ImageNames.append(ImageName)                                        # Storing image's names.
    
    return Images, ImageNames
        

def GetLines(Image):
    # Converting to grayscale
    GrayImage = cv2.cvtColor(Image, cv2.COLOR_BGR2GRAY)
    # Blurring image to reduce noise.
    BlurGrayImage = cv2.GaussianBlur(GrayImage, (5, 5), 1)
    # Generating Edge image
    EdgeImage = cv2.Canny(BlurGrayImage, 40, 255)

    # Finding Lines in the image
    Lines = cv2.HoughLinesP(EdgeImage, 1, np.pi / 180, 100)


if __name__ == "__main__":
    Images, ImageNames = ReadImage("InputImages")            # Reading all input images
    
    for i in range(len(Images)):
        Image = Images[i]

        # Getting the lines form the image
        GetLines(Image)
