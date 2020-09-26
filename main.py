import os
import cv2
import math
import numpy as np


# Threshold by which lines will be rejected wrt the horizontal
REJECT_DEGREE_TH = 4.0


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
        

def FilterLines(Lines):
    FinalLines = []
    
    for Line in Lines:
        [[x1, y1, x2, y2]] = Line

        # Calculating equation of the line: y = mx + c
        if x1 != x2:
            m = (y2 - y1) / (x2 - x1)
        else:
            m = 100000000
        c = y2 - m*x2
        # theta will contain values between -90 -> +90. 
        theta = abs(math.degrees(math.atan(m)))

        # Rejecting lines of slope near to 0 degree or 90 degree and storing others
        if REJECT_DEGREE_TH <= abs(theta) <= (90 - REJECT_DEGREE_TH):
            l = math.sqrt( (y2 - y1)**2 + (x2 - x1)**2 )    # length of the line
            FinalLines.append([x1, y1, x2, y2, m, c, l])

    return FinalLines
    


def GetLines(Image):
    # Converting to grayscale
    GrayImage = cv2.cvtColor(Image, cv2.COLOR_BGR2GRAY)
    # Blurring image to reduce noise.
    BlurGrayImage = cv2.GaussianBlur(GrayImage, (5, 5), 1)
    # Generating Edge image
    EdgeImage = cv2.Canny(BlurGrayImage, 40, 255)

    # Finding Lines in the image
    Lines = cv2.HoughLinesP(EdgeImage, 1, np.pi / 180, 50, 10, 10)
    
    # Filtering Lines wrt angle
    Lines = FilterLines(Lines)
    
    # Removing extra lines 
    # (we might get many lines, so we are going to take only longest 15 lines 
    # for further computation because more than this number of lines will only 
    # contribute towards slowing down of our algo.)
    if len(Lines) > 15:
        Lines = sorted(Lines, key=lambda x: x[-1], reverse=True)
        Lines = Lines[:15]
    
    return Lines
    

if __name__ == "__main__":
    Images, ImageNames = ReadImage("InputImages")            # Reading all input images
    
    for i in range(len(Images)):
        Image = Images[i]

        # Getting the lines form the image
        Lines = GetLines(Image)

        cv2.imshow("Ia", Image)
        cv2.waitKey(0)
