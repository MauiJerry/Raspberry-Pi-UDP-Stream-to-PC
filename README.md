# UDP Live Image Streaming

This project is a fork of a fork of a fork. I forked it to see if it would work for rPi to PC streaming. As yet it is untested/modified.

This project is inspired by https://www.cs.utexas.edu/~teammco/misc/udp_video/ , where images are grabbed from camera on one machine and transfered to another machine via UDP, resulting in negligible latency.

Parameters such as stream size or quality can be adjusted in `config.h` before re-compile the program.

## Updates
forked from a fork from a fork, updated to build on rPi Buster with OpenCV 4.5.5.  Required changing some CV_ defines to drop the CV_ (opencv 3.x apparently). Also mod the cmake to remove deprecated warnings about dynamic throw 

## Grabbing

The code grabs video stream from a Raspberry Pi that is pushing UDP packets to port 8080 for the machine that you are running this code on.

## Encoding

To avoid latency altogether there is no video codec involved during this process. Every frame is individually encoded to `jpeg` format by OpenCV to drastically reduce the bandwidth consumption.

If passing raw image is preferred, consider changing `jpeg` to `bmp`.

## Demo

Run the following command to see stream your camera through localhost: (`CMake` and `OpenCV` required)
```
git clone https://github.com/pokeastuff/Raspberry-Pi-UDP-Stream-to-PC.git
cd Raspberry-Pi-UDP-Stream-to-PC/
cmake . && make
./server 10000 &
./client 127.0.0.1 10000

server alternation:
./server PORT TYPE-OF-IMAGE-PROCESSING
./server 10000 0
./server 10000 1
```
## Note
You will need to change the IP of the destination for the UDP stream. At the moment, you will need to re-compile every time the IP changes. So I reccommend making your local IP static on the recieving end.

## Acknowledgement and Copyright
This project is built upon various open-sourced libraries, like [Practical C++ Sockets](http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/) and [OpenCV 3](http://opencv.org/) ; please refer to their original license accordingly (GPL/BSD). Code of this project is puslished under MIT License.
