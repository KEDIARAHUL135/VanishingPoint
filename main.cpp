#define _USE_MATH_DEFINES

#include "opencv2/opencv.hpp"
#include <iostream>
#include <filesystem>
#include <cmath>


namespace fs = std::filesystem;		// ISO C++17 Standard (/std:c++17)


float REJECT_DEGREE_TH = 4.0;


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


std::vector<std::vector<double>> FilterLines(std::vector<cv::Vec4i> Lines)
{
	std::vector<std::vector<double>> FinalLines;

	for (int i = 0; i < Lines.size(); i++)
	{
		cv::Vec4i Line = Lines[i];
		int x1 = Line[0], y1 = Line[1];
		int x2 = Line[2], y2 = Line[3];
		
		double m, c;

		// Calculating equation of the line : y = mx + c
		if (x1 != x2)
			m = (double)(y2 - y1) / (double)(x2 - x1);
		else
			m = 100000000.0;
		c = y2 - m * x2;
		
		// theta will contain values between - 90 -> + 90.
		double theta = atan(m) * (180.0 / M_PI);

		/*# Rejecting lines of slope near to 0 degree or 90 degree and storing others
        if REJECT_DEGREE_TH <= abs(theta) <= (90 - REJECT_DEGREE_TH):
            l = math.sqrt( (y2 - y1)**2 + (x2 - x1)**2 )    # length of the line
            FinalLines.append([x1, y1, x2, y2, m, c, l])*/
		// Rejecting lines of slope near to 0 degree or 90 degree and storing others
		if (REJECT_DEGREE_TH <= abs(theta) && abs(theta) <= (90.0 - REJECT_DEGREE_TH))
		{
			double l = pow((pow((y2 - y1), 2) + pow((x2 - x1), 2)), 0.5);	// length of the line
			std::vector<double> FinalLine{ (double)x1, (double)y1, (double)x2, (double)y2, m, c, l };
			FinalLines.push_back(FinalLine);
		}
	}

	// Removing extra lines
	// (we might get many lines, so we are going to take only longest 15 lines 
	// for further computation because more than this number of lines will only
	// contribute towards slowing down of our algo.)
	if (FinalLines.size() > 15)
	{
		std::sort(FinalLines.begin(), FinalLines.end(), 
				  [](const std::vector< double >& a, 
					 const std::vector< double >& b) 
					{ return a[6] > b[6]; });
		
		std::vector<std::vector<double>> FinalLines2;
		FinalLines = std::vector<std::vector<double>>(FinalLines.begin(), FinalLines.begin() + 15);
	}

	return FinalLines;
}


std::vector<std::vector<double>> GetLines(cv::Mat Image)
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
	cv::HoughLinesP(EdgeImage, Lines, 1, CV_PI / 180, 50, 15);
	
	// Check if lines found and exit if not.
	if (Lines.size() == 0)
	{
		std::cout << "Not enough lines found in the image for Vanishing Point detection." << std::endl;
		exit(3);
	}

	//Filtering Lines wrt angle
	std::vector<std::vector<double>> FilteredLines;
	FilteredLines = FilterLines(Lines);

	return FilteredLines;
}


int* GetVanishingPoint(std::vector<std::vector<double>> Lines)
{
	// We will apply RANSAC inspired algorithm for this.We will take combination
	// of 2 lines one by one, find their intersection point, and calculate the
	// total error(loss) of that point.Error of the point means root of sum of
	// squares of distance of that point from each line.
	int* VanishingPoint = new int[2];
	VanishingPoint[0] = -1; VanishingPoint[1] = -1;

	double MinError = 1000000000.0;

	for (int i = 0; i < Lines.size(); i++)
	{
		for (int j = i + 1; j < Lines.size(); j++)
		{
			double m1 = Lines[i][4], c1 = Lines[i][5];
			double m2 = Lines[j][4], c2 = Lines[j][5];

			if (m1 != m2)
			{
				double x0 = (c1 - c2) / (m2 - m1);
				double y0 = m1 * x0 + c1;

				double err = 0;
				for (int k = 0; k < Lines.size(); k++)
				{
					double m = Lines[k][4], c = Lines[k][5];
					double m_ = (-1 / m);
					double c_ = y0 - m_ * x0;

					double x_ = (c - c_) / (m_ - m);
					double y_ = m_ * x_ + c_;

					double l = pow((pow((y_ - y0), 2) + pow((x_ - x0), 2)), 0.5);

					err += pow(l, 2);
				}

				err = pow(err, 0.5);

				if (MinError > err)
				{
					MinError = err;
					VanishingPoint[0] = (int)x0;
					VanishingPoint[1] = (int)y0;
				}
			}
		}
	}

	return VanishingPoint;
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
		std::vector<std::vector<double>> Lines;
		Lines = GetLines(Image);

		// Get vanishing point
		int* VanishingPoint = GetVanishingPoint(Lines);

		// Checking if vanishing point found
		if (VanishingPoint[0] == -1 && VanishingPoint[1] == -1)
		{
			std::cout << "Vanishing Point not found. Possible reason is that not enough lines are found in the image for determination of vanishing point." << std::endl;
			continue;
		}

		// Drawing linesand vanishing point
		for (int i = 0; i < Lines.size(); i++)
		{
			std::vector<double> Line = Lines[i];
			cv::line(Image, cv::Point((int)Line[0], (int)Line[1]), cv::Point((int)Line[2], (int)Line[3]), cv::Scalar(0, 255, 0), 2);
		}
		cv::circle(Image, cv::Point(VanishingPoint[0], VanishingPoint[1]), 10, cv::Scalar(0, 0, 255), -1);

		// Showing the final image
		cv::imshow("OutputImage", Image);
		cv::waitKey(0);
	}

	return 0;
}