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
    {"player", "txt"},
    {"openpose-result", "avi"}
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

    VideoCapture poseVideo(getPath("openpose-result", input));
    assert(poseVideo.isOpened());

    int inputVideoWidth = inputVideo.get(CAP_PROP_FRAME_WIDTH);
    int inputVideoHeight = inputVideo.get(CAP_PROP_FRAME_HEIGHT);

    int maxCropWidth = 120 * inputVideoWidth / WIDTH;
    int maxCropHeight = 240 * inputVideoHeight / HEIGHT;

    ifstream playerfile;
    playerfile.open(getPath("player",input));

    Mat frame, poseFrame;
    
    for(int frameNumber=0;; frameNumber++){
        inputVideo >> frame;
        poseVideo >> poseFrame;
        if(frame.empty()){
            break;
        }
        
        int tagFrameNumber, tag;
        Rect boundingBoxN, boundingBoxS;
        Point2f feetN, feetS;
        playerfile >> tagFrameNumber >> tag;
        playerfile >> boundingBoxN.x >> boundingBoxN.y;
        playerfile >> boundingBoxN.width >> boundingBoxN.height;
        playerfile >> feetN.x >> feetN.y;
        playerfile >> boundingBoxS.x >> boundingBoxS.y;
        playerfile >> boundingBoxS.width >> boundingBoxS.height;
        playerfile >> feetS.x >> feetS.y;

        boundingBoxN.x = boundingBoxN.x * inputVideoWidth / WIDTH;
        boundingBoxN.y = boundingBoxN.y * inputVideoHeight / HEIGHT;
        boundingBoxN.width = boundingBoxN.width * inputVideoWidth / WIDTH;
        boundingBoxN.height = boundingBoxN.height * inputVideoHeight / HEIGHT;
        feetN.x = feetN.x * inputVideoWidth / WIDTH;
        feetN.y = feetN.y * inputVideoHeight / HEIGHT;

        boundingBoxS.x = boundingBoxS.x * inputVideoWidth / WIDTH;
        boundingBoxS.y = boundingBoxS.y * inputVideoHeight / HEIGHT;
        boundingBoxS.width = boundingBoxS.width * inputVideoWidth / WIDTH;
        boundingBoxS.height = boundingBoxS.height * inputVideoHeight / HEIGHT;
        feetS.x = feetS.x * inputVideoWidth / WIDTH;
        feetS.y = feetS.y * inputVideoHeight / HEIGHT;

        assert(frameNumber==tagFrameNumber);
        
        if(tag){
            poseFrame(boundingBoxN).copyTo(frame(boundingBoxN));
            poseFrame(boundingBoxS).copyTo(frame(boundingBoxS));
            rectangle(frame, boundingBoxN, color.blue, 2);
            circle(frame, feetN, 2, color.blue, 2);
            rectangle(frame, boundingBoxS, color.blue, 2);
            circle(frame, feetS, 2, color.blue, 2);
        }
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