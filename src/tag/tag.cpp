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
    {"input", "avi"},
    {"bg", "png"},
    {"tag", "txt"}
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

bool isSimilar(Mat image1, Mat image2, double threshold = 1){
    Mat grayImage1, grayImage2;
    cvtColor(image1, grayImage1, CV_BGR2GRAY);
    cvtColor(image2, grayImage2, CV_BGR2GRAY);
    assert(grayImage1.size() == grayImage2.size());
    double dist = 0;
    for(int x=0; x<grayImage1.size().width; x++){
        for(int y=0; y<grayImage1.size().height; y++){
            uchar p1 = grayImage1.at<uchar>(y,x);
            uchar p2 = grayImage2.at<uchar>(y,x);
            dist += abs(double(p1)-double(p2));
        }   
    }
    return dist < WIDTH*HEIGHT*threshold;
}

void process(string input, map<string,int> options){
    
    Mat bgImage = imread(getPath("bg", input));
    assert(!bgImage.empty());
    cout << "BG image loaded" << endl;
    
    VideoCapture inputVideo(getPath("input", input));
    assert(inputVideo.isOpened());

    Mat frame, lastFrame, taggedFrame;
    Scalar stausColor;
    bool lastFrameGood = false;

    ofstream tagfile;
    if(options["save"]){
        tagfile.open(getPath("tag",input));
    }
        
    for(int frameNumber=0;; frameNumber++){
        inputVideo >> frame;
        if(frame.empty()) break;
        if(frameNumber%1000 == 0){
            cout << "Processing frame number : " << frameNumber+1 << endl;
        }
        if(options["show"]){
            taggedFrame = frame.clone();
        }
        if(isSimilar(frame, bgImage, 10) || lastFrameGood && isSimilar(frame, lastFrame, 3)){
            stausColor = color.green;
            lastFrameGood = true;
            if(options["save"]){
                tagfile << frameNumber << " " << 1 << endl;
            }
        }
        else{
            stausColor = color.red;
            lastFrameGood = false;
            if(options["save"]){
                tagfile << frameNumber << " " << 0 << endl;
            }
        }
        if(options["show"]){
            circle(taggedFrame, Point(20,20), 20, stausColor, -1);
            showImage(input, taggedFrame, 1);
        }
        
        lastFrame = frame.clone();
    }
    if(options["save"]){
        cout << "Tag saved" << endl;
    }
    if(options["show"]){
        destroyWindow(input);
    }

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
        {"save", false},
        {"show", false}
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