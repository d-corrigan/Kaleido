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


	// open the video file for reading
	VideoCapture cap("output.mp4");

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
		int width = BGR.rows;
		int height = BGR.cols;
		cout<<frame_number<<" Frame Width: "<<width<<" Height: "<<height<<endl;

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

		//create the fusion pair matrices 8bit 3 channel with the same size as the original
		Mat fusion_pair_1 = Mat::zeros( XYZ.size(), XYZ.type() );
		Mat fusion_pair_2 = Mat::zeros( XYZ.size(), XYZ.type() );


		 /// Do the operation new_image(i,j) = alpha*image(i,j) + beta
		 for( int y = 0; y < XYZ.rows; y++ ){
			 for( int x = 0; x < XYZ.cols; x++ ){
				 for( int c = 0; c < 3; c++ ){

					 //cout<<"["<<(int)XYZ.at<Vec3b>(y,x)[c]<<"]"<<endl;

					 int new_delta;



					 if ((int)XYZ.at<Vec3b>(y,x)[c] + delta >255 || (int)XYZ.at<Vec3b>(y,x)[c] - delta < 0){

						 if((int)XYZ.at<Vec3b>(y,x)[c] + delta >255){
							 new_delta = 255 - (int)XYZ.at<Vec3b>(y,x)[c];
						 }else{
							 new_delta = (int)XYZ.at<Vec3b>(y,x)[c]- 0;
						 }

					 }else{

					 new_delta = delta;
					 }




					fusion_pair_1.at<Vec3b>(y,x)[c] =  (int)XYZ.at<Vec3b>(y,x)[c] + new_delta;
					fusion_pair_2.at<Vec3b>(y,x)[c] =  (int)XYZ.at<Vec3b>(y,x)[c] - new_delta;

				 }
		    }
		 }

	Mat XYZ2BGR;

	//Dont need to convert back to XYZ from BGR
	//cvtColor(XYZ,XYZ2BGR,COLOR_XYZ2BGR);

	//namedWindow( "BGR back to XYZ", WINDOW_AUTOSIZE );// Create a window for display.
	//imshow( "Display window XYZ->BGR", XYZ2BGR);// Show our image inside it.
	//waitKey(0);


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





