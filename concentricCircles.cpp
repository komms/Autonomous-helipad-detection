/*
 * concentricCircles.cpp
 *
 *  Created on: Apr 9, 2018
 *      Author: Nitin
 */


#include "opencv2/imgproc.hpp" //filters,edges...
#include "opencv2/features2d.hpp" // class featureDetector
#include "opencv2/highgui.hpp" //imshow,imwrite,imread,namedwindow...
#include "opencv2/core/utility.hpp" //Mat
#include "opencv2/imgcodecs.hpp" //imread is moved to imgcodecs from highgui
#include "opencv2/videoio.hpp"// for video

#include <stdio.h>
#include<iostream>
#include<vector>
#include<math.h>

using namespace cv;
using namespace std;

//vector<double>pixel_radius_vector; //global

struct state_estimates{

	double x_heli=0.0,y_heli=0.0;
};

//function returning a structure
struct state_estimates func(int centroidavgX, int centroidavgY){//instead of void it should take centroids.x and .y and height from the sensor
	cout<<endl<<"im in the struct function";
	state_estimates uav;//creating an object for the structure
//in MM
	double height=300,HFOV=70.42,VFOV=43.30;
	int resX=480,resY=640;

//in MM
	uav.x_heli=((centroidavgX-240)*height*tan(HFOV/2))/resX;//centroidavgX-opticalcenter.x should be written ??
	uav.y_heli=((centroidavgY-320)*height*tan(VFOV/2))/resY;
	cout<<endl<<"in struct----the realtive position in x(CM)--"<<uav.x_heli/10<<"the realtive position in y(CM)--"<<uav.y_heli/10;
	return uav;

}

int main()
{
	struct state_estimates uav; //creating a object UAV
	RNG rng(12345);
	const int width=640;
	const int height=480;

	//real metric values of the circles in mm
     double real_radius[7]={27,30,35,40,51.5,59,97.5};//105-first parent  and 15-last child  19,
     cout<<endl<<"real radius check--"<<real_radius[0];
   //PIXEL RADIUS
     double pixel_radius=0; //local in main

    /* //calculate height function declaration
     void calculateheight(double r_outerPx,double r_outermm);//,double r_innerPx,double r_innermm
    */

	Mat helipad = Mat::zeros(height,width, CV_8UC3 );
	    //helipad=imread("./data/helipad_s.jpg");
	char key = 'a';
	 VideoCapture cap(1);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	 //cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);

  while (key != 27)
	    {
		  bool objectfound=false;
	     Mat frame;
	     cap>>frame;
	     //cout<<frame.cols<<"**"<<frame.rows;
	     helipad=frame.clone();
	     if( helipad.empty())
	     	    {
	     	        cout<< "Error reading object " << std::endl;
	     	        return -1;
	     	    }
	     //single channeled image
	     Mat helipadbin=Mat::zeros( height,width, CV_8UC1 );
	     cvtColor(helipad,helipadbin,COLOR_BGR2GRAY);
	     GaussianBlur(helipadbin,helipadbin,Size(7,7),2);


	//conversion to binary
	   threshold(helipadbin, helipadbin,100,255,THRESH_BINARY_INV);
	   imshow("binary",helipadbin);



   //contours
	 vector<vector<Point>> contours;
	 vector<Vec4i> hierarchy;

	 findContours(helipadbin,contours,hierarchy,RETR_TREE,CHAIN_APPROX_NONE);
	 cout<<endl<<"contours found--"<<contours.size()<<endl<<"hierarchy---"<<hierarchy.size();

	 //draw contours
	 char text[10] ;
	 	 for(int i=0;i<(int)contours.size();i++){
	 		sprintf(text,"%d",i);
	 		 //contourArea
	 		 double a=contourArea( contours[i],false);//greens theorem
	 		  if(a>200){ //&& a<900
	 			//Scalar color(0,0,255);
	 		 Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	 		 drawContours( helipad, contours, i, color, 2, 8, hierarchy, 0, Point() );
	 		putText(helipad,text,contours[i][2],FONT_HERSHEY_SIMPLEX,.5,Scalar(0,0,255),1,1);
	 		  }
	 	 }
	 	 //cout<<endl<<"a point in the contour"<<contours[2][2];
	 	 imshow("source",helipad);

	  //Hierarchy
	 	   //for(int j=0;j<(int)hierarchy.size();j++){
	        // cout<<endl<<"heirarchy at-"<<j<<hierarchy[j];
	 	 // }

	 	 //keeping only rings i.e contours which have a child -->[next,previous,first_child,parent]
     	 //vector<Vec4i> hierarchy_updated;
	 	 vector<int> contourNo; //cannot be more than 12 whn hierarchy is considered
	 	 for(int k=0;k<(int)hierarchy.size();k++){
              if( hierarchy[k][2]==k+1 && hierarchy[k][3]==k-1){//detecting contours with a parent and a child
                       contourNo.push_back(k);
	              // hierarchy_updated[k][j]=hierarchy[k][j];

	 			 //contour with no child
	 			  //cout<<endl<<"contour with child and a parent--"<<k;

	 			//erasing that element
	 			  //contours.erase(contours.begin()+k);
	 			  //hierarchy.erase(hierarchy.begin()+k); //enabling it,gives false rings
	 		}//else{
	 			//cout<<endl<<"-----NESTED CONTOURS NOT FOUND for that hierarchy-----"<<k<<endl;
	 		//}
         }
	 	 cout<<endl<<"no of contours with hierarchy --"<<contourNo.size();

	 	//storing the respective contours of contourNo's
	 	         vector<vector<Point>> new_contours;
	 	Mat helipad1=Mat::zeros( height,width, CV_8UC3 );//-------------helipad1
	 	//putText(helipad1,"komal",Point(200,300),FONT_HERSHEY_SIMPLEX,.5,Scalar(0,0,255),2,1);


	 	 if((int)contourNo.size()>1){ // heirarchised contours size

         int c=0;
         for(int k=0;k<(int)contourNo.size();k++){
        	 c=contourNo[k];
              cout<<endl<<"--C--VALUE--"<<c;
        	 new_contours.push_back(contours[c]);
        	 //cout<<endl<<"new_contours-------"<<k<<new_contours[k];

         }

         //cout<<endl<<"---newContours----"<<new_contours[1];


	 	 //cout<<endl<<"updated heirarchy size--"<<hierarchy.size()<<endl<<"updated contours size--"<<new_contours.size();
	 	 //cout<<endl<<"contour-23--"<<contours[23];
	 	// for(int i=0;i<(int)hierarchy.size();i++){
	 		 //cout<<endl<<"updated hierarchy--"<<i<<hierarchy[i];
	 	//  }



             //Roundness check and contours passed the test
         	  //Area of the contour
                 vector<double>area; //can be int, max.area-307200
                 //double ar=0.0;
                 vector<double>perimeter; //can be int
                 double per=0.0;
                 vector<double>pixel_radius_vector;
                 float roundness;
                 const double pi = 3.1415926535897;
                 Mat helipad1=Mat::zeros( height,width, CV_8UC3 );

              //Moments variables
                 vector<Moments> mu(new_contours.size());
                 vector<Point2i> mc(new_contours.size());//can be int
                 const char text1[2]="*";

                  //vector of centroids
                 vector<Point2i>centroids;

                 //vector<float> hutemp;
                 vector<vector<float>> hu_vecofvec;

                 // for each contour---- area,perimeter and radius is calculated
                 for(int i=0;i<(int)new_contours.size();i++){

                	 //area of the contour
                	 double ar=0.0;
                 	 ar=contourArea( new_contours[i],false);//false for absolute value and using green theorem
                     area.push_back(ar);
                     cout<<endl<<"area of contour--"<<i<<"--"<<ar;

                    //perimeter of the contour
                     per=arcLength(new_contours[i],1);
                     perimeter.push_back(per);
                     cout<<endl<<endl<<"perimeter of contour--"<<i<<"--"<<per;
                     roundness=(4*pi*ar)/(per*per);
                     cout<<endl<<"Roundness check for contour--"<<i<<"--"<<roundness;


                    cout<<endl<<" curve with roundness greater than 0.82 is further processed";
                     //when the camera is oblique, the roundness of the circle decreases,results in ellipse
                     if(roundness>0.82){ //need one more condition because of single circle detection

                    	 //radius of the contour in pixels
                    	    // double pixel_radius=0; //local
                    	          pixel_radius=pow(ar/pi,0.5);
                    	          pixel_radius_vector.push_back(pixel_radius);
                    	          cout<<endl<<"pixel radius of  contour--"<<i<<"----"<<pixel_radius;


                    	 //calculate those moments
                    	 mu[i]=(moments(new_contours[i],false));
                         // cout<<endl<<"checking moment of contour--"<<i<<"nu03--"<<mu[i].nu03;//NORMALISED
                         // cout<<endl<<"checking moment of contour--"<<i<<"MU11--"<<mu[i].mu11;//CENTRAL
                         // cout<<endl<<"checking moment of contour--"<<i<<"M00--"<<mu[i].m00;//SPATIAL

                    	 //calculating the centroid using the spatial moments
                    	         mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
                    	        // cout<<endl<<"centroid of the contour-- "<<i<<"--"<<mc[i];
                    	         centroids.emplace_back(mc[i]);
                    	         cout<<endl<<"in centroids assignment"<<centroids[i];

                    	    //calculating HU moments
                    	         double hu[7];
                    	         vector<float> hutemp;
                    	        // vector<vector<float>> hu_vecofvec;
                    	         //vector<Vec3f> hu_moments(new_contours.size()); global declaration
                    	         HuMoments(mu[i],hu);
                    	         cout<<endl<<endl<<"--HU MOMENTS for contour--"<<i;

                    	         //cout << "size of hutemp" << hutemp.size() << endl;

                    	        for(int k=0;k<3;k++){
                    	        	double temp=log(hu[k]);
                    	        	 hutemp.push_back(temp);
                    	        	 //cout<<endl<<"hu Moment-"<<k<<"--"<<hutemp[k];
                    	         }
                    	         //cout<<endl<<"hutemp vector--"<<hutemp[i];
                    	         //pushing the vector
                    	         hu_vecofvec.push_back(hutemp);
                    	         cout<<endl<<"hu vecof vec size--"<<hu_vecofvec.size();

                    	       for(int i = 0 ; i<1 /*(int)hu_vecofvec.size()*/ ; i++){
                    	        	// cout<<endl<<"for -----"<<i;*/
                    	        	 for(int k=0;k<(int)hutemp.size();k++){
                    	        		 cout<<endl<<"printing the first 3 hu moments-----"<<k<<"---"<<hu_vecofvec[i][k]<<endl;
                    	        		 // cout<<endl<<"printing the vector-----"<<hu_vecofvec --->array of arrays;
                    	         }
                    	      }






                    	          //hutemp.push_back(log(hu));
                    	           //cout<<endl<<"Hu moment--1"<<hutemp[0]<<endl;
                     }//if-roundness>0.82
                   }//for loop for each curve
                //cout<<endl<<"-------------imhere";


                 //object recognition
                 //hu moments descriptors -- hu1=1.83;hu2=12.22;hu3=17.6087
                 int sum=0;
                //global bool objectfound=false;
                 for(int h=0;h<(int)hu_vecofvec.size() -1;h++){
                	 cout<<"im in hu compare------"<<h<<endl<<endl;
                	 int hu10=hu_vecofvec[h][0];
                	 int hu20=hu_vecofvec[h+1][0];
                	 int hu11=hu_vecofvec[h][1];
                	 int hu21=hu_vecofvec[h+1][1];
                	 int hu12=hu_vecofvec[h][2];
                	 int hu22=hu_vecofvec[h+1][2];
                	 cout<<endl<<"hu10"<<hu10<<"hu20"<<hu20<<endl<<"hu11"<<hu11<<"hu21"<<hu21<<endl<<"hu12"<<hu12<<"hu22"<<hu22;
                		 //if((hu1>=1.7 ||hu1<=1.9) && (hu2>=1.7 ||hu2<=1.9)){
                	 if((hu10==hu20) && (hu11/hu21<=1.5||hu11/hu21>=0.69) &&(hu12/hu22<=1.5||hu12/hu22>=0.69)){ //from hu11 they are not equal
                			 cout<<endl<<"-----------------helipad found";
                			 sum=sum+1;
                			 //if(h==(int)hu_vecofvec.size())
                				// break;
                		 }
                 }
                		 cout<<endl<<"sum--------------"<<sum<<endl;

                		if(sum>1){
                			objectfound=true;
                		}


                	// }


               if(objectfound==true){
                     cout<<endl<<"-------------imhere in object found";

                    // cout<<endl<<"checking for centroids--"<<centroids;
                     // cout<<endl<<"checking for centroids--"<<centroids.size();
                     //avg of centroids of all circles
                                     Point2i center;
                                      int sumX=0, sumY=0;
                                      Point2i avg_center;
                                      for(int z=0;z<(int)centroids.size();z++){
                                     	 //cout<<endl<<"centroid of contour--"<<z<<"--"<<centroids[z];

                                     	 sumX=sumX+centroids[z].x;

                                     	 sumY=sumY+centroids[z].y;
                                      }

                                      //cout<<endl<<"sum of x coordinates--"<<sumX;
                                      //cout<<endl<<"sum of y coordinates--"<<sumY;
                                      avg_center.x=sumX/(int)centroids.size();
                                      avg_center.y=sumY/(int)centroids.size();
                                      cout<<endl<<"Centroid_Averaged--"<<avg_center;
                                      putText(helipad1,text1,avg_center,FONT_HERSHEY_SIMPLEX,.5,Scalar(0,0,255),2,1);//helipad1 imshow
                                      cout<<endl<<"-------------im after centroid";

                                      //calling struct state-estimates func
                                      uav=func(avg_center.x,avg_center.y);

                                      //frame center
                                                       Point2i av,bv,ch,dh,center_frame;
                                                       	av.x=width/2;av.y=0;
                                                       	bv.x=width/2;bv.y=height;
                                                       	ch.x=0;ch.y=height/2;
                                                       	dh.x=width;dh.y=height/2;
                                                       	center_frame.x=width/2;center_frame.y=height/2;
                                                       	cout<<endl<<"center of the frame---"<<center_frame;
                                                       line(helipad1,av,bv,Scalar(0,0,255),2,8,0);
                                                       line(helipad1,ch,dh,Scalar(0,0,255),2,8,0);
                                                       cout<<endl<<"-------------im after frame center";
                         cout<<endl<<"pixel radius vector check--"<<pixel_radius_vector.size();



                                 /* //calculateHeight function call
                                                //assuming that the smallest circle will be detected in each frame
                                                 for(int a=0; a<(int)pixel_radius_vector.size()-2;a++){
                                               //pixel radius should be arranged in ascneding order
                                                	 //cout<<endl<<"pixel radius ---"<<pixel_radius_vector[a];
                                                	 int b=(int)pixel_radius_vector.size()-a;
                                                	 cout<<"b value--"<<b;
                                                	 cout<<endl<<"-----parameters used-----";
                                                	 cout<<endl<<"rinnter in px--"<<pixel_radius_vector[b]<<endl<<"r inner in mm--"<<real_radius[a];
                                                	 cout<<endl<<"rOuter in px--"<<pixel_radius_vector[b-1]<<endl<<"r outer in mm--"<<real_radius[a+1];

                                               // calculateheight(double r_outerPx,double r_outermm,double r_innerPx,double r_innermm)
                                               calculateheight(pixel_radius_vector[b],real_radius[a]);//pixel_radius_vector[b-1],real_radius[a+1],

                                                }*/
                                                /*giving wrong values*/

                 }
                 //putText(helipad1,"sai",Point(200,500),FONT_HERSHEY_SIMPLEX,.5,Scalar(0,0,255),2,1);
                 //cout<<endl<<"-------------im after object found";




               }//if contours detected with heirarchy>=1
               else{
                  cout<<endl<<"-----------no contours with heirarchy found----------";
                  objectfound=false;
                   }





	 	cout<<endl<<"im before newcontour";
	 	// these are contours with hierarchy test passed and not roundness
	 	 for(int k=0;k<(int)new_contours.size();k++){
                    	         sprintf(text,"%d",k);
                    	          Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
                    	          drawContours( helipad1, new_contours, k, color, 2, -1 );
                    	          putText(helipad1,text,new_contours[k][2],FONT_HERSHEY_SIMPLEX,.5,Scalar(0,0,255),1,1);
	 	 }
                /* //checking for moments
                 for(const Moments& v : mu){ //avoids copying
                	 cout<<v<<endl;
                 }*/



                 //centroids are the same, what if the image is not parallel to camera


      //imshow("source",helipad);
	 	//}//if contourNo ending
	 imshow("updated",helipad1);//centroid and frame center are not dispalyed in this??

	//waitKey(0);*/
	 	 cout<<endl<<"im at the end";
	//imshow("updated",helipad1);
	key = waitKey(1); //increasing delay??
	     }


	return 0;
}

/*//function definition for returning the height of the each circle detected
// the innermost circle should always be detected
 void calculateheight(double r_outerPx,double r_outermm){//,double r_innerPx,double r_innermm

	 //camera intrinsic parameter
   double alpha_x=454; //in pixels-from experiments-454 and camera calibration-629.19
	double height;
	//general equation of lens, but with avg
	height= alpha_x *((r_outermm/r_outerPx));//+(r_innermm/r_innerPx)
	//alpha_x= height*((r_outerPx/r_outermm));//+(r_innerPx/r_innermm)
	cout<<endl<<"the real height of the ring"<<height<<"mm";
	//cout<<endl<<"the intrinsic camera parameter---"<<alpha_x<<"mm";
}*/


