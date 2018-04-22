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
    {"input", "mp4"},
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

void process(string input, map<string,int> options){
        
    VideoCapture inputVideo(getPath("input", input));
    assert(inputVideo.isOpened());

    Mat frame;
    Scalar stausColor;
    int tagFrameNumber, tag;

    ifstream tagfile;
    tagfile.open(getPath("tag",input));
        
    for(int frameNumber=0;; frameNumber++){
        inputVideo >> frame;
        if(frame.empty()) break;
        if(frameNumber%1000 == 0){
            cout << "Processing frame number : " << frameNumber+1 << endl;
        }
        tagfile >> tagFrameNumber >> tag;
        assert(frameNumber==tagFrameNumber);
        if(tag){
            stausColor = color.green;
        }
        else{
            stausColor = color.red;
        }
        circle(frame, Point(20,20), 20, stausColor, -1);
        showImage(input, frame, options["fast"] ? 1 : 20);
    }
    destroyWindow(input);

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
        {"fast", false}
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