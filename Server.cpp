/*
 *   C++ UDP socket server for live image upstreaming
 *   Modified from http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoServer.cpp
 *   Copyright (C) 2015
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   Edited by Austin Greisman
 */

#include "PracticalSocket.h" // For UDPSocket and SocketException
#include <iostream>          // For cout and cerr
#include <cstdlib>           // For atoi()
#include <string.h>
#include <stdio.h>


#define BUF_LEN 65540 // Larger than maximum UDP packet size

#include "opencv2/opencv.hpp"
using namespace cv;
#include "config.h"

Mat Find_Edges(Mat frame, int thresh);
Mat presentMat(Mat frame, int width, int height);

int main(int argc, char * argv[]) {
    //Custom code
    Mat croppedImage = cv::Mat::zeros(167, 154, CV_8UC3);
    Mat CannyFrame;
    cv::Rect myROI(0, 20, 154, 167);
    char cannyornaw[1];

    unsigned short servPort = atoi(argv[1]); // First arg:  local port

    //Out
    string servAddressout = "192.168.0.105"; // First arg: server address
    unsigned short servPortout = Socket::resolveService("1997", "udp");

    try {
        UDPSocket sock(servPort);

        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
        unsigned short sourcePort; // Port of datagram source

        clock_t last_cycle = clock();

        // Out
        UDPSocket sockout; 
        Mat frame_out, send;
        vector < uchar > encoded;
        int jpegqual =  ENCODE_QUALITY; // Compression Parameter
        vector < int > compression_params;
        compression_params.push_back(IMWRITE_JPEG_QUALITY);//CV_IMWRITE_JPEG_QUALITY);
        compression_params.push_back(jpegqual);

        //namedWindow("recv", CV_WINDOW_AUTOSIZE);
        while (1) {
            // Block until receive message from a client
            do {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
            } while (recvMsgSize > sizeof(int));
            int total_pack = ((int * ) buffer)[0];
            //printf("%d\n",total_pack );
            cout << "expecting length of packs:" << total_pack << endl;
            char * longbuf = new char[PACK_SIZE * total_pack];
            for (int i = 0; i < total_pack; i++) {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
                if (recvMsgSize != PACK_SIZE) {
                    cerr << "Received unexpected size pack:" << recvMsgSize << endl;
                    continue;
                }
                memcpy( & longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
            }

            cout << "Received packet from " << sourceAddress << ":" << sourcePort << endl;
 
            Mat rawData = Mat(1, PACK_SIZE * total_pack, CV_8UC1, longbuf);
            Mat frame = imdecode(rawData, IMREAD_COLOR);//CV_LOAD_IMAGE_COLOR);
            if (frame.size().width == 0) {
                cerr << "decode failure!" << endl;
                continue;
            }
            // Crop the full image to that image contained by the rectangle myROI
            // Note that this doesn't copy the data
            croppedImage = frame(myROI);
            //cv:resize(croppedImage, croppedImage, Size(), 4, 4, INTER_LINEAR);
            //Place Text
            //rotate(croppedImage, croppedImage, ROTATE_90_COUNTERCLOCKWISE);
            CannyFrame = Find_Edges(croppedImage, 100);
            //rotate(CannyFrame, CannyFrame, ROTATE_90_CLOCKWISE);
            //CannyFrame = presentMat(CannyFrame, 2560, 1440);
            imshow("SplitView", CannyFrame);

            if(CannyFrame.size().width==0)continue;//simple integrity check; skip erroneous data...
            //resize(outframe, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);

            imencode(".jpg", CannyFrame, encoded, compression_params);
            //imshow("send", send);
            total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;

            int ibuf[1];
            ibuf[0] = total_pack;
            sockout.sendTo(ibuf, sizeof(int), servAddressout, servPortout);

            for (int i = 0; i < total_pack; i++){
                sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, servAddressout, servPortout);
                //printf("i this round is %d\n", i);
            }


            free(longbuf);

            if(!cv::waitKey(30)) break;
            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
            cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

            cout << next_cycle - last_cycle;
            last_cycle = next_cycle;


        }
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}

Mat Find_Edges(Mat frame, int thresh)
{
    Mat Thermal_Copy = frame.clone();
    //Grey scale
    cvtColor(frame, frame, COLOR_BGR2GRAY);
    blur( frame, frame, Size(3,3) );
    
    Mat canny_output;
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    
    /// Detect edges using canny
    Canny( frame, canny_output, thresh, thresh*1.5, 3 );
    
    /// Find contours
    findContours( canny_output, contours, hierarchy, 
    RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    //CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( 0, 255, 0 );
        drawContours( drawing, contours, i, color, 0.5, 8, hierarchy, 0, Point() );
    }
    
    return drawing;
}

Mat presentMat(Mat frame, int width, int height){
    //Rotate and Resize Frame
    Mat frame_final = frame.clone();
    rotate(frame, frame_final, ROTATE_90_CLOCKWISE);
    resize(frame_final, frame_final, cv::Size(width - 1/ 2, height));
    
    //Changing image size and filling with black
    //copyMakeBorder(frame_final, frame_final, 100, 100, 100, 100, BORDER_CONSTANT);
    
    //Place Text
    //putText(frame_final, "Warning: CO High", cvPoint(frame_final.cols * 0.30 ,frame_final.rows * 0.30), FONT_HERSHEY_DUPLEX, 1, cvScalar(0,0,250), 2, CV_AA);
    
    //Make the video Google Carboard compatible (Split image based on ROI)
    Mat Eye_Block(frame_final, Rect (frame_final.cols / 4, 0, frame_final.cols / 2, frame_final.rows));
    
    //Recombined image to make Stereovision video
    Mat Stereo_frame;
    //Left_Final = frame_final(Left_Frame);
    hconcat(Eye_Block, Eye_Block, Stereo_frame);
    
    // Display the resulting frame
    //namedWindow("Frame", WINDOW_NORMAL);
    //resizeWindow("Frame", 1980, 1080);
    //imshow( "Frame", Stereo_frame );
    return Stereo_frame;
}

//2560x1440
/*

Mat presentMat(Mat frame, int width, int height){
    //Rotate and Resize Frame
    Mat frame_final = frame.clone();
    rotate(frame, frame_final, ROTATE_90_CLOCKWISE);
    resize(frame_final, frame_final, cv::Size(), 0.70, 0.70);
    
    //Changing image size and filling with black
    copyMakeBorder(frame_final, frame_final, 100, 100, 100, 100, BORDER_CONSTANT);
    
    //Place Text
    //putText(frame_final, "Warning: CO High", cvPoint(frame_final.cols * 0.30 ,frame_final.rows * 0.30), FONT_HERSHEY_DUPLEX, 1, cvScalar(0,0,250), 2, CV_AA);
    
    //Make the video Google Carboard compatible (Split image based on ROI)
    Mat Eye_Block(frame_final, Rect (frame_final.cols / 4, 0, frame_final.cols / 2, frame_final.rows));
    
    //Recombined image to make Stereovision video
    Mat Stereo_frame;
    //Left_Final = frame_final(Left_Frame);
    hconcat(Eye_Block, Eye_Block, Stereo_frame);
    
    // Display the resulting frame
    //namedWindow("Frame", WINDOW_NORMAL);
    //resizeWindow("Frame", 1980, 1080);
    //imshow( "Frame", Stereo_frame );
    return Stereo_frame;
}*/

        
