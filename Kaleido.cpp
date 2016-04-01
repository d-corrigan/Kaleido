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
#include <iomanip>


using namespace std;
using namespace cv;

int value = 1;

//Green Point
float greenX = 0.28;
float greenY = 0.595;

//Blue Point
float blueX = 0.155;
float blueY = 0.07;

//Red Point
float redX = 0.625;
float redY = 0.34;


//important variables
float delta = 30;

int BLOCK_SIZE_Col = 32;
int BLOCK_SIZE_Row = 32;

int frame_rate= 120;



////methods

String getNextName();

float getDistanceBetweenPoints(float x1, float y1,float x2,float y2 );

float getAngleInRadians (float x1, float y1,float x2,float y2 );

bool isInRGBRange(int x, int y);

float getRandomNumberInRange();

float calculateNewXPoint(float dist, float angle, float x);

float calculateNewYPoint(float dist, float angle, float y);

float convertBackX(float Y, float x, float y );
float convertBackY(float Y );
float convertBackZ(float Y, float x, float y );



int main(){

	//for random color pollution set to true
	bool random = false;

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

			// Write to Video when finished reassembling four sub-frames
			cv::VideoCapture in_capture("images/%04d.png");
			Mat img;


			cout<< "video written @"<<frame_rate<<" frames/sec"<<endl;
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


		//create the fusion pair matrices 8bit 3 channel with the same size as the original
		Mat fusion_pair_1 = Mat::zeros( XYZ.size(), XYZ.type() );
		Mat fusion_pair_2 = Mat::zeros( XYZ.size(), XYZ.type() );

		//create fusion pair for random color pollution
		Mat color_fusion_pair_1 = Mat::zeros( XYZ.size(), XYZ.type() );
		Mat color_fusion_pair_2 = Mat::zeros( XYZ.size(), XYZ.type() );


		 /* Cloning the image to another for visualization later, if you do not want to visualize the result just comment every line related to visualization */
		 cv::Mat maskImg = XYZ.clone();

		 /* Checking if the clone image was cloned correctly */

		 if(!maskImg.data || maskImg.empty())
			 cout<< "Problem Loading Image" << endl;


		bool even_block = false;
		int previous_y = 0;
		vector <Mat> blocks;


		if (random){// random color pixel pollution section

			for(int i = 0; i < XYZ.cols; i++)
			{
				for(int j = 0; j < XYZ.rows; j++)
				{

					bool in_RGB_range = false;
					Vec3b pixel = XYZ.at<Vec3b>(i,j);
					delta = .02;

					//convert to Yxy format
					float Y = (float)pixel[1] /255;
					float x = (float)pixel[0] / ( (float)pixel[0] + (float)pixel[1] + (float)pixel[2] );
					float y = (float)pixel[1] / ( (float)pixel[0] + (float)pixel[1] + (float)pixel[2] );


					// get a random color
					while(in_RGB_range == false ){


						//random number between 1 and 0
						float random_pointX = getRandomNumberInRange();
						float random_pointY = getRandomNumberInRange();

						// find out if random numbers are within the RGB range
						if (isInRGBRange(random_pointX, random_pointY)){in_RGB_range = true;}

						//get the distance between points
						float distance = getDistanceBetweenPoints(random_pointX, random_pointY, x, y);

						//get the angle between points
						float radians = getAngleInRadians(random_pointX, random_pointY, x, y);

						//calculate new point based on distance and angle
						float calculateNewXPoint(float dist, float angle, float x);

						float newX = calculateNewXPoint(distance, radians, x);
						float newY = calculateNewYPoint(distance, radians, y);


						if (isInRGBRange(newX, newY)){

							// return two new coordinates to xyz format and add to Mat
							color_fusion_pair_1.at<Vec3b>(i,j)[0] = convertBackX(Y, random_pointX, random_pointY );
							color_fusion_pair_1.at<Vec3b>(i,j)[1] = convertBackY(Y );
							color_fusion_pair_1.at<Vec3b>(i,j)[2] =  convertBackZ(Y, random_pointX, random_pointY );

							color_fusion_pair_2.at<Vec3b>(i,j)[0] = convertBackX(Y, newX, newY );
							color_fusion_pair_2.at<Vec3b>(i,j)[1] = convertBackY(Y );
							color_fusion_pair_2.at<Vec3b>(i,j)[2] = convertBackZ(Y, newX, newY );

							in_RGB_range = true;

						}else {
							in_RGB_range = false;

						}
					}
				}
			}
		}//END of random color

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
					//rectangle(maskImg, Point(y, x), Point(y + (maskImg.cols / BLOCK_SIZE_Col) - 1, x + (maskImg.rows / BLOCK_SIZE_Row) - 1), CV_RGB(255, 0, 0), 1); // visualization


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


						// Change ever other block for the two fusion pair
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




					}else{

						even_block = true;

						// Leave every other block as is (with ability to use a delta)
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
				}
			}
		//Verify block size will work
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

	//conversion color pollution pair to BGR
	cvtColor(color_fusion_pair_1,color_fusion_pair_1,COLOR_XYZ2BGR);
	cvtColor(color_fusion_pair_2,color_fusion_pair_2,COLOR_XYZ2BGR);


	//Write images to file to be converted to video

		if(random){
			cv::imwrite(getNextName(),color_fusion_pair_2);
			cv::imwrite(getNextName(),color_fusion_pair_1);
			cv::imwrite(getNextName(),color_fusion_pair_2);
			cv::imwrite(getNextName(),color_fusion_pair_1);
		}else{
			cv::imwrite(getNextName(),fusion_pair_2);
			cv::imwrite(getNextName(),fusion_pair_1);
			cv::imwrite(getNextName(),fusion_pair_2);
			cv::imwrite(getNextName(),fusion_pair_1);
		}

	}

	return (1);

}

String getNextName(){

	std::stringstream ss;
	ss << "images/"<< std::setw(4) << std::setfill('0') << value <<".png";
	std::string str = ss.str();
	std::cout <<"Writing: "<< str<<endl;

	value++;


	return str;
}

float getDistanceBetweenPoints(float x1, float y1,float x2,float y2 ){

	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

float getAngleInRadians (float x1, float y1,float x2,float y2 ){

	return atan2(y1 - y2, x1 - y2);
}

float getRandomNumberInRange(){
	return ((float) rand()) / (float) RAND_MAX;
}

float calculateNewXPoint(float dist, float angle, float x){

	return x + dist * cos(angle);

}
float calculateNewYPoint(float dist, float angle, float y){

	return  y + dist * sin(angle);

}
float convertBackX(float Y, float x, float y ){

	return x * ( Y*255 / y);

}
float convertBackY(float Y ){

	return Y*255;
}
float convertBackZ(float Y, float x, float y ){

	return ( 1 - x - y ) * ( Y*255 / y );
}


bool isInRGBRange(int x, int y){

	bool range = false;

	float alpha = ((blueY - redY)*(x - redX) + (redX - blueX)*(y - redY)) /((blueY - redY)*(greenX - redX) + (redX - blueX)*(greenY - redY));
	float beta = ((redY - greenY)*(x - redX) + (greenX - redX)*(y - redY)) /((blueY - redY)*(blueX - redX) + (redX - blueX)*(greenY - redY));
	float gamma = 1.0f - alpha - beta;


	if (alpha > 0 && beta > 0 && gamma > 0){

		range = true;
	}

	return range;
}
