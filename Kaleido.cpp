/*
 * Kaleido.cpp
 *
 *  Created on: Mar 19, 2016
 *      Author: David Corrigan
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>


using namespace std;
using namespace cv;


int main(){


	int BLOCK_SIZE_Col = 16;
	int BLOCK_SIZE_Row = 16;


	// open the video file for reading
	VideoCapture cap("test.mp4");

	if ( !cap.isOpened() )  // if not success, exit program
	{
		 cout << "Cannot open the video file" << endl;
		 return -1;
	}else{
		cout<<"Video File Opened."<<endl;
	}

	//get the frames per seconds of the video
	double fps = cap.get(CV_CAP_PROP_FPS);
	cout << "Frame per seconds : " << fps << endl;

	//create a window called "MyVideo" to be used later
	//namedWindow("MyVideo",CV_WINDOW_AUTOSIZE);

	//used to keep track of the number of frames
	int frame_number = 0;

	//Run loop while video still has frames
	while(1)
	{
		/*
		 *  BGR is the original frame which will converted to Lab and split into 4 sub-frames
		 *  (two fusion pairs)
		 */
		Mat BGR;

		// read the next frame from video
		bool bSuccess = cap.read(BGR);

		//if not success or end of frames, break loop
		if (!bSuccess)
		{
			cout << "End of File." << endl;
			break;
		}

		//get width and height of the frames
		cout<<frame_number<<BGR.size()<<endl;

		//increase the number of frames to show how many original frames that have been processed
		frame_number++;


		/*
		 * CIE XYZ is a matrix with format (x,z) Y luminance
		 */
		Mat XYZ;

		//convert BGR to XYZ
		cvtColor(BGR,XYZ, COLOR_BGR2XYZ);

		// Split the channels for XYZ Matrix
		vector<Mat> XYZ_channels;
		split(XYZ, XYZ_channels);

		namedWindow( "Display window XYZ", WINDOW_AUTOSIZE );// Create a window for display.
		imshow( "Display window XYZ", XYZ );


		//Show three channels "grayscale"
		namedWindow( "X Channel", WINDOW_AUTOSIZE );// Create a window for display.
		//imshow( "X Channel", XYZ_channels[0] );

		namedWindow( "Y Channel", WINDOW_AUTOSIZE );// Create a window for display.
		//imshow( "Y Channel", XYZ_channels[1] );

		namedWindow( "Z Channel", WINDOW_AUTOSIZE );// Create a window for display.
		//imshow( "Z Channel", XYZ_channels[2] );  // Show our image inside it.
		waitKey(0);

		int delta = 40;

		cout<<"Size: "<<XYZ.size()<<endl;

		//create the fusion pair matrices 8bit 3 channel with the same size as the original
		Mat fusion_pair_1 = Mat::zeros( XYZ.size(), XYZ.type() );
		Mat fusion_pair_2 = Mat::zeros( XYZ.size(), XYZ.type() );


		 /* Cloning the image to another for visualization later, if you do not want to visualize the result just comment every line related to visualization */
		 cv::Mat maskImg = XYZ.clone();

		 /* Checking if the clone image was cloned correctly */

		 if(!maskImg.data || maskImg.empty())
			 cout<< "Problem Loading Image" << endl;


		bool even_block = false;
		vector <Mat> blocks;

		namedWindow( "small Image", WINDOW_AUTOSIZE );

		// check if divisors fit to image dimensions
		if(XYZ.cols % BLOCK_SIZE_Col == 0 && XYZ.rows % BLOCK_SIZE_Row == 0)
	   {
			for(int y = 0; y < XYZ.cols; y += XYZ.cols / BLOCK_SIZE_Col)
			{
				for(int x = 0; x < XYZ.rows; x += XYZ.rows / BLOCK_SIZE_Row)
				{

					//creating the block
					cv::Rect rect = cv::Rect(y, x, (XYZ.cols / BLOCK_SIZE_Col), (XYZ.rows / BLOCK_SIZE_Row));

					//split image into blocks
					blocks.push_back(XYZ(rect).clone());


					//show part of image being operated on
					rectangle(maskImg, Point(y, x), Point(y + (maskImg.cols / BLOCK_SIZE_Col) - 1, x + (maskImg.rows / BLOCK_SIZE_Row) - 1), CV_RGB(255, 0, 0), 1); // visualization

					//show the current block
					imshow ( "small Images", cv::Mat ( XYZ, rect ));// visualization
					imshow("Image", maskImg); // visualization


					if(even_block == true){
						even_block =  false;

						// HERE we can change the even blocks






					}else{
						even_block = true;
					}
					//waitKey(0); // visualization
				}
			}
	   }else if(XYZ.cols % BLOCK_SIZE_Col != 0){
		cout << "Error: Please use another divisor for the column split." << endl;
		exit(1);
	   }else if(XYZ.rows % BLOCK_SIZE_Row != 0){
		cout << "Error: Please use another divisor for the row split." << endl;
		exit(1);
	   }

	//Mat XYZ2BGR;


	//convert fusion pair to BGR
	cvtColor(fusion_pair_1,fusion_pair_1,COLOR_XYZ2BGR);
	cvtColor(fusion_pair_2,fusion_pair_2,COLOR_XYZ2BGR);


	namedWindow( "FusionPair + Delta", WINDOW_AUTOSIZE );// Create a window for display.
	imshow( "FusionPair + Delta", fusion_pair_1);
	cv::imwrite("images/fusion_pair_1_BGR.tiff",fusion_pair_1);


	namedWindow( "FusionPair - Delta", WINDOW_AUTOSIZE );// Create a window for display.
	imshow( "FusionPair - Delta", fusion_pair_2);
	cv::imwrite("images/fusion_pair_2_BGR.tiff",fusion_pair_2);

	}
}





