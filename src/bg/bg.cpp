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
    assert(image1.size() == image2.size());
    double dist = 0;
    for(int x=0; x<image1.size().width; x++){
        for(int y=0; y<image1.size().height; y++){
            uchar p1 = image1.at<uchar>(y,x);
            uchar p2 = image2.at<uchar>(y,x);
            dist += abs(double(p1)-double(p2));
        }   
    }
    return dist < WIDTH*HEIGHT*threshold;
}

Mat getBgImage(string input, map<string,int> options){
    
    VideoCapture inputVideo(getPath("input", input));
    assert(inputVideo.isOpened());
    
    Mat frame, lastFrame, showFrame;
    Mat bgImage = Mat::zeros(Size(WIDTH,HEIGHT), CV_8UC3);
    Scalar stausColor;
    int similarity = 0;
    int goodFrameCount = 0;
    vector<Mat> frames;
    vector<vector<vector<int>>> bucket(64,vector<vector<int>>(64,vector<int>(64,0)));
    
    for(int frameNumber=0; goodFrameCount<2500; frameNumber++){
        inputVideo >> frame;
        if(frame.empty()) break;
        if(frameNumber%1000 == 0){
            cout << "Processing frame number : " << frameNumber+1 << endl;
        }
        showFrame = frame.clone();
        resize(frame, frame, Size(WIDTH,HEIGHT));
        if(frameNumber>0){
            if(isSimilar(frame, lastFrame, 3)){
                if(similarity<200){
                    similarity += 1;
                }
            }
            else{
                similarity = 0;
            }
            if(similarity>=200){
                stausColor = color.green;
                goodFrameCount++;
                if(frames.size()<500){
                    frames.push_back(frame);
                }
                else{
                    int index = rand()%goodFrameCount;
                    if(index<500){
                        frames[index] = frame;
                    }
                }
            }
            else{
                stausColor = color.red;
            }
            circle(showFrame, Point(20,20), 20, stausColor, -1);
        }
        if(options["debug"]){
            showImage(input, showFrame, 1);
        }
        lastFrame = frame;
    }
    if(options["debug"]){
        destroyWindow(input);
    }
    cout << "Number of good frames : " << frames.size() << endl;

    for(int x=0; x<WIDTH; x++){
        if(x%10 == 0){
            cout << "Calculate background : " << (x/10)+1 << "/" << (WIDTH-1)/10+1 << endl;
        }
        for(int y=0; y<HEIGHT; y++){
            int maxFreq = 0;
            Vec3b maxFreqColor({0,0,0});
            for(int i=0; i<frames.size(); i++){
                Vec3b p = frames[i].at<Vec3b>(y,x);
                int B = p[0]/4;
                int G = p[1]/4;
                int R = p[2]/4;
                for(int dB=-1; dB<=1; dB++){
                    for(int dG=-1; dG<=1; dG++){
                        for(int dR=-1; dR<=1; dR++){
                            int nB = B+dB;
                            int nG = G+dG;
                            int nR = R+dR;
                            if(nB>=0 && nB<64 && nG>=0 && nG<64 && nR>=0 && nR<64){
                                bucket[nB][nG][nR]++;               
                                if(bucket[nB][nG][nR]>maxFreq){
                                    maxFreq = bucket[nB][nG][nR];
                                    maxFreqColor = Vec3b(nB*4+2,nG*4+2,nR*4+2);
                                }
                            }
                        }
                    }    
                }
            }
            for(int i=0; i<frames.size(); i++){
                Vec3b p = frames[i].at<Vec3b>(y,x);
                int B = p[0]/4;
                int G = p[1]/4;
                int R = p[2]/4;
                for(int dB=-1; dB<=1; dB++){
                    for(int dG=-1; dG<=1; dG++){
                        for(int dR=-1; dR<=1; dR++){
                            int nB = B+dB;
                            int nG = G+dG;
                            int nR = R+dR;
                            if(nB>=0 && nB<64 && nG>=0 && nG<64 && nR>=0 && nR<64){
                                bucket[nB][nG][nR]--;               
                            }
                        }
                    }    
                }
            }
            bgImage.at<Vec3b>(y,x) = maxFreqColor;
        }
    }
    return bgImage;

}

void process(string input, map<string,int> options){
    Mat bgImage = getBgImage(input, options);
    if(options["save"]){
        cout << "BG image saved" << endl;
        imwrite(getPath("bg", input), bgImage);
    }
    if(options["show"]){
        showImage("bgImage", bgImage, 0);
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
        {"show", false},
        {"debug", false}
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