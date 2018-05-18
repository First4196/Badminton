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
    {"tag-truth", "txt"},
    {"court", "txt"},
    {"player-tag-truth", "txt"},
    {"csv", "csv"}
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

Mat kernel = getStructuringElement(MORPH_ELLIPSE,Size(3,3),Point(1,1));

Point2f cNW(0,0), cNE(200,0), cSW(0,440), cSE(200,440);
vector<Point2f> courtCorners({cNW,cNE,cSW,cSE});
vector<Vec4f> courtLines({
    // hlines
    {0,0,200,0},
    {0,25,200,25},
    {0,155,200,155},
    {0,285,200,285},
    {0,415,200,415},
    {0,440,200,440},
    // vlines
    {0,0,0,440},
    {15,0,15,440},
    {100,0,100,155},
    {100,285,100,440},
    {185,0,185,440},
    {200,0,200,440}
});
Vec4f middleCourtLine({0,220,200,220});

double distance(Point2f p1, Point2f p2){
    return 10*(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y);
}

Point2f getMean(vector<Point2f> &vec, Point2f old){
    if(vec.size()==0){
        return old;
    }
    assert(vec.size()>0);
    double sumX = 0, sumY = 0;
    for(Point2f u : vec){
        sumX += u.x;
        sumY += u.y;
    }
    return Point2f(sumX/vec.size(), sumY/vec.size());
}

tuple<Point2f,Point2f> twoMeans(Mat &fgImg){
    Point2f centerN(WIDTH/2, HEIGHT/5), centerS(WIDTH/2, HEIGHT*4/5); 
    vector<Point2f> points;
    for(int x=0; x<WIDTH; x++){
        for(int y=0; y<HEIGHT; y++){
            if(fgImg.at<uchar>(y,x) == 255){
                points.push_back(Point2f(x,y));
            }
        }
    }
    for(int i=0; i<100; i++){
        vector<Point2f> pointsCenterN, pointsCenterS;
        for(auto point : points){
            if(distance(point, centerN)<=distance(point, centerS)){
                pointsCenterN.push_back(point);
            }
            else{
                pointsCenterS.push_back(point);
            }
        }
        Point2f newCenterN = getMean(pointsCenterN, centerN);
        Point2f newCenterS = getMean(pointsCenterS, centerS);
        double changeN = distance(centerN, newCenterN);
        double changeS = distance(centerS, newCenterS);
        centerN = newCenterN;
        centerS = newCenterS;
        if(changeN<1 && changeS<1){
            break;
        }
    }
    return tuple<Point2f,Point2f>(centerN, centerS);
}

Rect getBoundingBox(Point2f center, vector<Point2f> &points){
    
    if(points.size()==0){
        return Rect( Point2f(0, 0), Point2f(0, 0) );
    }
    
    if(points.size()==1){
        return Rect( points[0], points[0] );
    }

    double meanX = 0, meanY = 0;
    for(Point2f point : points){
        meanX += point.x;
        meanY += point.y;
    }
    meanX /= points.size();
    meanY /= points.size();

    double sdX = 0, sdY = 0;
    for(Point2f point : points){
        sdX += (point.x-meanX)*(point.x-meanX);
        sdY += (point.y-meanY)*(point.y-meanY);
    }
    sdX = min(sqrt(sdX/(points.size()-1)),20.0);
    sdY = min(sqrt(sdY/(points.size()-1)),40.0);

    bool anyGood = false;
    float loX=WIDTH, hiX=0, loY=HEIGHT, hiY=0;
    for(Point2f point : points){
        bool goodX = (meanX-3*sdX < point.x && point.x < meanX+3*sdX);
        bool goodY = (meanY-3*sdY < point.y && point.y < meanY+3*sdY);
        if(goodX && goodY){
            anyGood = true;
            loX = min(loX, point.x);
            hiX = max(hiX, point.x);
            loY = min(loY, point.y);
            hiY = max(hiY, point.y);
        }    
    }
    if(anyGood){
        return Rect( Point2f(loX, loY), Point2f(hiX, hiY) );
    }
    else{
        return Rect( Point2f(meanX, meanY), Point2f(meanX, meanY) );
    }
}

void process(string input, map<string,int> options){
   
    VideoCapture inputVideo(getPath("input", input));
    assert(inputVideo.isOpened());

    Mat bgImage = imread(getPath("bg", input));
    assert(!bgImage.empty());
    cout << "BG image loaded" << endl;

    ifstream courtfile;
    courtfile.open(getPath("court",input));
    
    Point2f NW, NE, SW, SE;
    courtfile >> NW.x >> NW.y;
    courtfile >> NE.x >> NE.y;
    courtfile >> SW.x >> SW.y;
    courtfile >> SE.x >> SE.y;
    vector<Point2f> corners({NW,NE,SW,SE});
    Mat homoMat = getPerspectiveTransform(corners, courtCorners);

    Rect courtROI;
    courtROI.x = min(NW.x,SW.x);
    courtROI.y = min(NW.y,NE.y);
    courtROI.width = max(NE.x,SE.x) - courtROI.x;
    courtROI.height = max(SW.y,SE.y) - courtROI.y;

    int extend = courtROI.y - max(0,courtROI.y-100);
    Rect ROI = courtROI;
    ROI.y = courtROI.y-extend;
    ROI.height = courtROI.height+extend;

    ifstream tagfile;
    tagfile.open(getPath("tag-truth",input));
    Mat frame;
    int tagFrameNumber, tag;

    ofstream playerfile;
    if(options["save"]){
        playerfile.open(getPath("player-tag-truth",input));
    }
    
    ofstream csvfile;
    if(options["savecsv"]){
        csvfile.open(getPath("csv",input));
        csvfile << "frame,tag,xN,yN,wN,hN,xS,yS,wS,hS" << endl;
    }

    int splitY = -1;
    for(int frameNumber=0; frameNumber<2500; frameNumber++){
        inputVideo >> frame;
        if(frame.empty()){
            break;
        }
        tagfile >> tagFrameNumber >> tag;
        assert(frameNumber==tagFrameNumber);
        if(tag){
            Mat fgImage = Mat::zeros( Size(WIDTH,HEIGHT), CV_8U);
            Mat frameYCrCb, bgImageYCrCb;
            cvtColor(frame, frameYCrCb, CV_BGR2YCrCb);
            cvtColor(bgImage, bgImageYCrCb, CV_BGR2YCrCb);

            for(int i=0; i<ROI.width; i++){
                for(int j=0; j<ROI.height; j++){
                    int x = i+ROI.x;
                    int y = j+ROI.y;
                    Vec3b framePixel = frameYCrCb.at<Vec3b>(y,x);
                    Vec3b bgImgPixel = bgImageYCrCb.at<Vec3b>(y,x);
                    double dist = 0;
                    dist += (framePixel[1]-bgImgPixel[1])*(framePixel[1]-bgImgPixel[1]);
                    dist += (framePixel[2]-bgImgPixel[2])*(framePixel[2]-bgImgPixel[2]);
                    if(dist>128){
                        fgImage.at<uchar>(y,x) = 255;
                    }
                }
            }

            erode(fgImage,fgImage,kernel);     

            if(options["debug"]){
                showImage("fgImage",fgImage,1);
            }
            
            Point2f centerN, centerS;
            tie(centerN,centerS) = twoMeans(fgImage);

            vector<Point2f> pointCenterN, pointCenterS;
            for(int x=0; x<WIDTH; x++){
                for(int y=0; y<HEIGHT; y++){
                    if(fgImage.at<uchar>(y,x) == 255){
                        if(distance(Point2f(x,y), centerN)<=distance(Point2f(x,y), centerS)){
                            pointCenterN.push_back(Point2f(x,y));
                        }
                        else{
                            pointCenterS.push_back(Point2f(x,y));
                        }
                    }
                }
            }

            Rect boundingBoxN = getBoundingBox(centerN,pointCenterN);
            Rect boundingBoxS = getBoundingBox(centerS,pointCenterS);
            
            Point2f feetN = Point2f(boundingBoxN.x + boundingBoxN.width*0.5, boundingBoxN.y + boundingBoxN.height*0.9);
            Point2f feetS = Point2f(boundingBoxS.x + boundingBoxS.width*0.5, boundingBoxS.y + boundingBoxS.height*0.9);    

            if(options["save"]){
                playerfile << frameNumber << " " << tag << " ";

                playerfile << boundingBoxN.x << " " << boundingBoxN.y << " ";
                playerfile << boundingBoxN.width << " " << boundingBoxN.height << " ";
                playerfile << feetN.x << " " << feetN.y << " ";
                
                playerfile << boundingBoxS.x << " " << boundingBoxS.y << " ";
                playerfile << boundingBoxS.width << " " << boundingBoxS.height << " ";
                playerfile << feetS.x << " " << feetS.y << endl;
            }

            if(options["savecsv"]){
                vector<Point2f> points({feetN, feetS}), transformedPoints;
                perspectiveTransform(points, transformedPoints, homoMat);
                Point2f transformedFeetN = transformedPoints[0];
                Point2f transformedFeetS = transformedPoints[1];

                csvfile << frameNumber << "," << tag << ",";

                csvfile << transformedFeetN.x << "," << transformedFeetN.y << ",";
                csvfile << boundingBoxN.width << "," << boundingBoxN.height << ",";

                csvfile << transformedFeetS.x << "," << transformedFeetS.y << ",";
                csvfile << boundingBoxS.width << "," << boundingBoxS.height << endl;
            }
            
            if(options["show"]){
                if(options["debug"]){
                    rectangle(frame, ROI.tl(), ROI.br(), color.red, 2);
                }
                rectangle(frame, boundingBoxN, color.blue, 2);
                circle(frame, feetN, 2, color.blue, 2);
                rectangle(frame, boundingBoxS, color.blue, 2);
                circle(frame, feetS, 2, color.blue, 2);
                showImage(input,frame,1);
            }
        }
        else{
            if(options["save"]){
                playerfile << frameNumber << " " << tag << " ";
                playerfile << "-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1\n";
            }
            
            if(options["savecsv"]){
                csvfile << frameNumber << "," << tag << ",";
                csvfile << "-1,-1,-1,-1,-1,-1,-1,-1" << endl;                
            }

            if(options["show"]){
                showImage(input,frame,1);
            }
        }
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
        {"savecsv", false},
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