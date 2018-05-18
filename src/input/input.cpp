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
    {"raw", "mp4"},
    {"input", "avi"}
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

void process(string input, map<string,int> options){
   
    VideoCapture inputVideo(getPath("raw", input));
    assert(inputVideo.isOpened());

    int inputVideoWidth = inputVideo.get(CAP_PROP_FRAME_WIDTH);
    int inputVideoHeight = inputVideo.get(CAP_PROP_FRAME_HEIGHT);
    int inputFPS = inputVideo.get(CAP_PROP_FPS);
    auto ex = VideoWriter::fourcc('D', 'I', 'V', 'X');

    int fps = 10;
    int ratio = round((double)inputFPS/(double)fps);
    
    VideoWriter outputVideo;
    if(options["save"]){
        outputVideo.open(getPath("input",input), ex, 1, Size(WIDTH, HEIGHT), true);
        assert(outputVideo.isOpened());
    }

    Mat outputFrame;
    for(int frameNumber=0;; frameNumber++){
        inputVideo >> outputFrame;
        if(outputFrame.empty()){
            break;
        }
        if(frameNumber%ratio!=0){
            continue;    
        }
        if(options["save"]){
            resize(outputFrame, outputFrame, Size(WIDTH, HEIGHT));
            outputVideo << outputFrame;
        }
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
        {"save", false}
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