#include "opencv2/opencv.hpp"
#include <iostream>
#include <filesystem>


namespace fs = std::filesystem;		// ISO C++17 Standard (/std:c++17)


int ReadImage(std::string InputImagePath, std::vector<cv::Mat>& Images, std::vector<std::string>& ImageNames)
{
	// Checking if path is of file or folder.
	if (fs::is_regular_file(fs::status(InputImagePath)))	// If path is of file.
	{
		cv::Mat InputImage = cv::imread(InputImagePath);	// Reading the image.

		// Checking if image is read.
		if (InputImage.empty())
		{
			std::cout << "Image not read. Provide a correct path" << std::endl;
			exit(1);
		}

		Images.push_back(InputImage);            // Storing the image.
		ImageNames.push_back(InputImagePath);    // Storing the image's name.

	}

	// If path is of a folder contaning images.
	else if (fs::is_directory(fs::status(InputImagePath)))
	{
		// Getting all image's path present inside the folder.
		for (const auto& entry : fs::directory_iterator(InputImagePath))
		{
			// Reading images one by one.
			cv::Mat InputImage = cv::imread(entry.path().u8string());

			Images.push_back(InputImage);            // Storing the image.
			ImageNames.push_back(entry.path().filename().u8string());    // Storing the image's name.
		}
	}
	
	// If it is neither file nor folder(Invalid Path).
	else
	{
		std::cout << "\nEnter valid Image Path." << std::endl;
		exit(2);
	}
	return 0;
}


int GetLines(cv::Mat Image)
{
	cv::Mat GrayImage, BlurGrayImage, EdgeImage;
	// Converting to grayscale
	cv::cvtColor(Image, GrayImage, cv::COLOR_BGR2GRAY);
	// Blurring image to reduce noise.
	cv::GaussianBlur(GrayImage, BlurGrayImage, cv::Size(5, 5), 1);
	// Generating Edge image
	cv::Canny(BlurGrayImage, EdgeImage, 40, 255);

	// Finding Lines in the image
	std::vector<cv::Vec4i> Lines;
	cv::HoughLinesP(EdgeImage, Lines, 1, CV_PI / 180, 50, 10, 15);
	
	// Check if lines found and exit if not.
	if (Lines.size() == 0)
	{
		std::cout << "Not enough lines found in the image for Vanishing Point detection." << std::endl;
		exit(3);
	}

	//Filtering Lines wrt angle
	//code here

	return 0;
}


int main()
{
	std::vector<cv::Mat> Images;			// Input Images will be stored in this list.
	std::vector<std::string> ImageNames;	// Names of input images will be stored in this list.
	ReadImage("InputImages", Images, ImageNames);
	
	for (int i = 0; i < Images.size(); i++)
	{
		cv::Mat Image = Images[i].clone();

		// Getting the lines form the image
		GetLines(Image);
	}

	return 0;
}