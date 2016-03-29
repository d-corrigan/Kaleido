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
#include <sstream>


using namespace std;
using namespace cv;




int value = 1;


//important variables
int delta = 30;

int BLOCK_SIZE_Col = 8;
int BLOCK_SIZE_Row = 8;

int frame_rate= 120;

int main(){




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
	cout << "Original Video :: Frame per seconds : " << fps << endl;

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


			cv::VideoCapture in_capture("images/%04d.png");
			Mat img;

			cout<< "video written @"<<frame_rate<<" frames / sec"<<endl;
			VideoWriter out_capture("images/video.avi", CV_FOURCC('M','J','P','G'), frame_rate, Size(512,512));

		  while (true)
		  {
			in_capture >> img;
			if(img.empty())
				break;

			out_capture.write(img);
		  }

			break;
		}

		//get width and height of the frames
		cout<<"Frame: "<<frame_number<<" "<<BGR.size()<<endl;

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
		//imshow( "Display window XYZ", XYZ );


		//Show three channels "grayscale"
		namedWindow( "X Channel", WINDOW_AUTOSIZE );// Create a window for display.
		//imshow( "X Channel", XYZ_channels[0] );

		namedWindow( "Y Channel", WINDOW_AUTOSIZE );// Create a window for display.
		//imshow( "Y Channel", XYZ_channels[1] );

		namedWindow( "Z Channel", WINDOW_AUTOSIZE );// Create a window for display.
		//imshow( "Z Channel", XYZ_channels[2] );  // Show our image inside it.
		//waitKey(0);



		//create the fusion pair matrices 8bit 3 channel with the same size as the original
		Mat fusion_pair_1 = Mat::zeros( XYZ.size(), XYZ.type() );
		Mat fusion_pair_2 = Mat::zeros( XYZ.size(), XYZ.type() );


		 /* Cloning the image to another for visualization later, if you do not want to visualize the result just comment every line related to visualization */
		 cv::Mat maskImg = XYZ.clone();

		 /* Checking if the clone image was cloned correctly */

		 if(!maskImg.data || maskImg.empty())
		cout<< "Problem Loading Image" << endl;


		bool even_block = false;
		int previous_y = 0;
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

					// this creates the checker board pattern

					if(y > previous_y){
						if (even_block){
							even_block = false;
						}else{
							even_block = true;
						}
					}

					previous_y = y;


					if(even_block == true){

						even_block =  false;

						for (int i = 0; i < XYZ(rect).rows; i++){
							for( int j = 0; j < XYZ(rect).cols; j++){
								for( int c = 0; c < 3; c++ ){

									 int new_delta;

									 if ((int)XYZ(rect).at<Vec3b>(j,i)[c] + delta >255 || (int)XYZ(rect).at<Vec3b>(j,i)[c] - delta < 0){

										 if((int)XYZ(rect).at<Vec3b>(j,i)[c] + delta >255){
											 new_delta = 255 - (int)XYZ(rect).at<Vec3b>(j,i)[c];
										 }else{
											 new_delta = (int)XYZ(rect).at<Vec3b>(j,i)[c]- 0;
										 }

									 }else{

									 new_delta = delta;
									 }

									fusion_pair_1.at<Vec3b>(x+j,y+i)[c] =  (int)XYZ(rect).at<Vec3b>(j,i)[c] + new_delta;
									fusion_pair_2.at<Vec3b>(x+j,y+i)[c] =  (int)XYZ(rect).at<Vec3b>(j,i)[c] - new_delta;

								 }
							}
						}


						// HERE we can change the even blocks

					}else{

						even_block = true;

						for (int i = 0; i < XYZ(rect).rows; i++){
							for( int j = 0; j < XYZ(rect).cols; j++){
								for( int c = 0; c < 3; c++ ){

									 int new_delta;

									 if ((int)XYZ(rect).at<Vec3b>(j,i)[c] + delta >255 || (int)XYZ(rect).at<Vec3b>(j,i)[c] - delta < 0){

										 if((int)XYZ(rect).at<Vec3b>(j,i)[c] + delta >255){
											 new_delta = 255 - (int)XYZ(rect).at<Vec3b>(j,i)[c];
										 }else{
											 new_delta = (int)XYZ(rect).at<Vec3b>(j,i)[c]- 0;
										 }

									 }else{

									 new_delta = delta;
									 }

									 //Here we can alternate the _+ delta, right now its left as is
									fusion_pair_1.at<Vec3b>(x+j,y+i)[c] =  (int)XYZ(rect).at<Vec3b>(j,i)[c] ;
									fusion_pair_2.at<Vec3b>(x+j,y+i)[c] =  (int)XYZ(rect).at<Vec3b>(j,i)[c] ;
								}
							}
						}
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


	//convert fusion pair to BGR
	cvtColor(fusion_pair_1,fusion_pair_1,COLOR_XYZ2BGR);
	cvtColor(fusion_pair_2,fusion_pair_2,COLOR_XYZ2BGR);


	namedWindow( "FusionPair + Delta", WINDOW_AUTOSIZE );// Create a window for display.
	//imshow( "FusionPair + Delta", fusion_pair_1);

	namedWindow( "FusionPair - Delta", WINDOW_AUTOSIZE );// Create a window for display.
	//imshow( "FusionPair - Delta", fusion_pair_2);



	//Write images to file to be converted to video

	std::string result = "";
	std::string result2 = "";
	std::string result3 = "";
	std::string result4 = "";
	std::string format = ".png";
	std::string prefix = "images/000";

	std::stringstream sstm;
	std::stringstream sstm2;
	std::stringstream sstm3;
	std::stringstream sstm4;

	if (value > 9){
		prefix = "images/00";
	}else if(value > 99){
		prefix = "images/0";
	}else if(value > 999){
		prefix = "images/";
	}else{
		prefix = "images/000";
	}


	sstm << prefix << value <<format;
	result = sstm.str();

	if (value+1 > 9){
		prefix = "images/00";
	}else if(value+1 > 99){
		prefix = "images/0";
	}else if(value+1 > 999){
		prefix = "images/";
	}else{
		prefix = "images/000";
	}

	sstm2 << prefix << value+1 <<format;
	result2 = sstm2.str();

	if (value+2 > 9){
		prefix = "images/00";
	}else if(value +2> 99){
		prefix = "images/0";
	}else if(value +2> 999){
		prefix = "images/";
	}else{
		prefix = "images/000";
	}

	sstm3 << prefix << value+2 <<format;
	result3 = sstm3.str();

	if (value+3 > 9){
		prefix = "images/00";
	}else if(value +3 > 99){
		prefix = "images/0";
	}else if(value +3 > 999){
		prefix = "images/";
	}else{
		prefix = "images/000";
	}

	sstm4 << prefix << value+3 <<format;
	result4 = sstm4.str();

	value += 4;

		cv::imwrite(result,BGR);
		cv::imwrite(result2,fusion_pair_1);
		cv::imwrite(result3,fusion_pair_2);
		cv::imwrite(result4,BGR);

	}

}
