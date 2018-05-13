#include <iostream>
#include <fstream>
#include <random>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

struct Color{
    Scalar blue = Scalar(255,0,0);
    Scalar green = Scalar(0,255,0);
    Scalar red = Scalar(0,0,255);
    Scalar yellow = Scalar(0,255,255);
    Scalar white = Scalar(255,255,255);
}color;

map<string, string> fileExt{
    {"lq", "avi"}
};

string getPath(string what, string input){
    assert(fileExt.find(what)!=fileExt.end());
    return "data/" + what +"s/" + what + input + "." + fileExt[what];
}

int WIDTH = 512;
int HEIGHT = 512;

void showImage(string windowName, const Mat &image, int wait = 0){
    namedWindow(windowName, WINDOW_AUTOSIZE );
    imshow(windowName, image);
    waitKey(wait);
}

VideoCapture inputVideo;
int frameNumber = 0;
Mat frame;

void on_trackbar(int, void*){
    inputVideo.set(CV_CAP_PROP_POS_FRAMES, frameNumber);
    inputVideo >> frame;
    imshow("explore", frame);
}

void process(string input, map<string,int> options){
    
    inputVideo = VideoCapture(getPath("lq", input));
    assert(inputVideo.isOpened());

    namedWindow("explore", CV_WINDOW_AUTOSIZE);

    char TrackbarName[50];
    sprintf( TrackbarName, "Frame (%d)", (int)inputVideo.get(CV_CAP_PROP_FRAME_COUNT));

    createTrackbar( TrackbarName, "explore", &frameNumber, inputVideo.get(CV_CAP_PROP_FRAME_COUNT)-1, on_trackbar );

    inputVideo >> frame;
    imshow("explore", frame);

    waitKey(0);
}

int main(int argc, char** argv ){

    srand(time(NULL));

    vector<string> optionArgs;
    vector<string> inputArgs;
    for(int i=1; i<argc; i++){
        string arg(argv[i]);
        if(arg[0] == '-'){
            optionArgs.push_back(arg.substr(1));
        }
        else{
            inputArgs.push_back(arg);
        }
    }

    map<string,int> options{
        
    };

    for(string option : optionArgs){
        if(options.find(option)!=options.end()){
            options[option] = true;
        }
    }

    for(string input : inputArgs){
        process(input, options);
    }

    return 0;
}