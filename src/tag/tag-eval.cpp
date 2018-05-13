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
    {"tag", "txt"},
    {"tag-gt", "txt"}
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

    int frameNumberTag, tag;
    ifstream tagfile;
    tagfile.open(getPath("tag",input));

    int frameNumberTaggt, taggt;
    ifstream taggtfile;
    taggtfile.open(getPath("tag-gt",input));
        
    int totalFrame = 0;
    int truePositive = 0;
    int falsePositive = 0;
    int trueNegative = 0;
    int falseNegative = 0;

    for(int frameNumber=0; frameNumber<1434; frameNumber++){
        tagfile >> frameNumberTag >> tag;
        taggtfile >> frameNumberTaggt >> taggt;
        totalFrame ++;
        if(tag==1){
            if(taggt==1){
                truePositive++;
            }
            else{
                falsePositive++;
            }
        }
        else{
            if(taggt==0){
                trueNegative++;
            }
            else{
                falseNegative++;
            }
        }
    }

    cout << totalFrame << endl;
    cout << truePositive << endl;
    cout << falsePositive << endl;
    cout << trueNegative << endl;
    cout << falseNegative << endl;

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