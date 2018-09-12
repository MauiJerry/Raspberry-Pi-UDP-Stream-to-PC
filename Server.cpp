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

int main(int argc, char * argv[]) {
    //Custom code
    Mat croppedImage = cv::Mat::zeros(167, 154, CV_8UC3);
    Mat CannyFrame;
    cv::Rect myROI(0, 20, 154, 167);
    char cannyornaw[1];


    if (argc != 3) { // Test for correct number of parameters
        std::cout << "Remember that you need 1 for Canny 0 for Thermal input!" << std::endl;
        cerr << "Usage: " << argv[0] << " <Server Port>" << endl;

        exit(1);
    }
    // Copies the answer to which view should be displayed
    strcpy(cannyornaw, argv[2]);

    unsigned short servPort = atoi(argv[1]); // First arg:  local port

    namedWindow("recv", CV_WINDOW_AUTOSIZE);
    try {
        UDPSocket sock(servPort);

        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
        unsigned short sourcePort; // Port of datagram source

        clock_t last_cycle = clock();

        while (1) {
            // Block until receive message from a client
            do {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
            } while (recvMsgSize > sizeof(int));
            int total_pack = ((int * ) buffer)[0];

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
            Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);
            if (frame.size().width == 0) {
                cerr << "decode failure!" << endl;
                continue;
            }
            // Crop the full image to that image contained by the rectangle myROI
            // Note that this doesn't copy the data
            croppedImage = frame(myROI);
            cv:resize(croppedImage, croppedImage, Size(), 4, 4, INTER_LINEAR);
            //Place Text
            CannyFrame = Find_Edges(croppedImage, 25);
            if (!strcmp(cannyornaw, "1"))
            {
                imshow("Canny Thermal", CannyFrame);
            }
            else{
                putText(croppedImage, "Drone Connected", cvPoint(croppedImage.cols * 0.05 ,croppedImage.rows * 0.05), FONT_HERSHEY_DUPLEX, 1, cvScalar(0,250,0), 2, CV_AA);
                imshow("Thermal Live Stream", croppedImage);
            }
            free(longbuf);

            if(cv::waitKey(30) >= 0) break;
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
    Canny( frame, canny_output, thresh, thresh*2, 3 );
    
    /// Find contours
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( 0, 255, 0 );
        drawContours( drawing, contours, i, color, 3, 8, hierarchy, 0, Point() );
    }
    
    return drawing;
}



        