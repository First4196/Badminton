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
    {"tag", "txt"},
    {"test-tag-vid", "avi"}
};

string getPath(string what, string input){
    assert(fileExt.find(what)!=fileExt.end());
    return "data/" + what +"/" + what + input + "." + fileExt[what];
}

int WIDTH = 512;
int HEIGHT = 512;

void showImage(string windowName, const Mat &image, int wait = 0){
    namedWindow(windowName, WINDOW_AUTOSIZE );
    imshow(windowName, image);
    waitKey(wait);
}

void putWhiteText(Mat& img, String text, Point p){
    putText(img, text, p, FONT_HERSHEY_SCRIPT_COMPLEX, 1, color.white, 2);
}

void process(string input, map<string,int> options){
        
    VideoCapture inputVideo(getPath("input", input));
    assert(inputVideo.isOpened());
    int numberOfFrames = inputVideo.get(CV_CAP_PROP_FRAME_COUNT);

    Mat frame;
    Scalar stausColor;
    int tagFrameNumber, tag;
    int numberOfTaggedFrames = 0;

    ifstream tagfile;
    tagfile.open(getPath("tag",input));
        
    VideoWriter outputVideo;
    if(options["save"]){
        auto ex = VideoWriter::fourcc('D', 'I', 'V', 'X');
        outputVideo.open(getPath("test-tag-vid",input), ex, 1, Size(WIDTH, HEIGHT), true);
        assert(outputVideo.isOpened());
    }

    for(int frameNumber=0;; frameNumber++){
        inputVideo >> frame;
        if(frame.empty()) break;
        if(frameNumber%1000 == 0){
            cout << "Processing frame number : " << frameNumber+1 << endl;
        }
        tagfile >> tagFrameNumber >> tag;
        assert(frameNumber==tagFrameNumber);
        if(tag){
            numberOfTaggedFrames ++;
            stausColor = color.green;
        }
        else{
            stausColor = color.red;
        }
        circle(frame, Point(20,20), 20, stausColor, -1);
        
        string text = to_string(frameNumber+1) + '/' + to_string(numberOfFrames);
        putWhiteText(frame, text, Point(2,HEIGHT-4));

        if(options["show"]){
            showImage(input, frame, options["fast"] ? 1 : 20);
        }

        if(options["save"]){
            outputVideo << frame;
        }

    }

    frame = Mat::zeros( Size(WIDTH, HEIGHT), CV_8UC3);

    string text1 = "Number of tagged frames : ";
    string text2 = to_string(numberOfTaggedFrames);
    string text3 = "Number of non-tagged frames : ";
    string text4 = to_string(tagFrameNumber-numberOfTaggedFrames);

    putWhiteText(frame, text1, Point(2,HEIGHT*1/6+10));
    putWhiteText(frame, text2, Point(2,HEIGHT*2/6+10));
    putWhiteText(frame, text3, Point(2,HEIGHT*4/6+10));
    putWhiteText(frame, text4, Point(2,HEIGHT*5/6+10));

    if(options["show"]){
        for(int i=0; i<100; i++){
            showImage(input, frame, options["fast"] ? 1 : 20);
        }
        destroyWindow(input);
    }

    if(options["save"]){
        for(int i=0; i<100; i++){
            outputVideo << frame;
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
        {"show", false},
        {"fast", false},
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