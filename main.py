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
        

if __name__ == "__main__":
    Images, ImageNames = ReadImage("InputImages")            # Reading all input images
    
    for i in Images:
        cv2.imshow("dv", i)
        cv2.waitKey(0)
    cv2.destroyAllWindows()
