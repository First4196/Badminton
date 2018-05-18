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
    {"bg", "png"},
    {"court", "txt"}
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

Point3f intersection(const Vec4f &l1, const Vec4f &l2){
    
    Point2f p1(l1[0], l1[1]), q1(l1[2], l1[3]);
    Point2f p2(l2[0], l2[1]), q2(l2[2], l2[3]);

    double a1 = q1.y-p1.y;
    double b1 = p1.x-q1.x;
    double c1 = a1*p1.x + b1*p1.y;
    
    double a2 = q2.y-p2.y;
    double b2 = p2.x-q2.x;
    double c2 = a2*p2.x + b2*p2.y;

    double det = a1*b2 - a2*b1;
    if(det != 0){
        double x = (b2*c1 - b1*c2)/det;
        double y = (a1*c2 - a2*c1)/det;
        return Point3f(x,y,1);
    }
    else{
        return Point3f(0,0,-1);
    }
    
}

void drawWhiteLine(Mat &mat, Vec4f l){
    Point2f p(l[0], l[1]), q(l[2], l[3]);
    line(mat, p, q, color.white, 1, CV_AA);
}

bool isVertical(const Vec4i &l){
    Point2f p(l[0], l[1]), q(l[2], l[3]);
    return (p.x-q.x)*(p.x-q.x) < (p.y-q.y)*(p.y-q.y);
}

tuple<vector<Vec4f>,Vec4f> getTransformedCourtLines(Point2f NW, Point2f NE, Point2f SW, Point2f SE){
    
    vector<Point2f> intersectionCorners({NW,NE,SW,SE});
    Mat homoMat = getPerspectiveTransform(courtCorners, intersectionCorners);
    
    vector<Point2f> courtPoints1, courtPoints2;
    courtLines.push_back(middleCourtLine); 
    for(auto &l : courtLines){
        courtPoints1.push_back({l[0],l[1]});
        courtPoints2.push_back({l[2],l[3]});
    }
    courtLines.pop_back();

    vector<Point2f> transformedCourtPoints1, transformedCourtPoints2;
    perspectiveTransform(courtPoints1, transformedCourtPoints1, homoMat);
    perspectiveTransform(courtPoints2, transformedCourtPoints2, homoMat);
    
    vector<Vec4f> transformedCourtlines;
    for(size_t i=0; i<transformedCourtPoints1.size(); i++){
        transformedCourtlines.push_back({transformedCourtPoints1[i].x,transformedCourtPoints1[i].y,transformedCourtPoints2[i].x,transformedCourtPoints2[i].y});
    }

    Vec4f transformedMiddleCourtLine = transformedCourtlines.back();
    transformedCourtlines.pop_back();

    return tuple<vector<Vec4f>,Vec4f>(transformedCourtlines, transformedMiddleCourtLine);
    
}

void process(string input, map<string,int> options){

    Mat bgImage = imread(getPath("bg", input));
    assert(!bgImage.empty());
    cout << "BG image loaded" << endl;

    Mat bgImageYCrCb;
    cvtColor(bgImage, bgImageYCrCb, CV_BGR2YCrCb);
    
    Mat whiteMask = Mat::zeros( Size(WIDTH, HEIGHT), CV_8U );
    int t1=6, t2=10, sl=128, sd=20;
    for(int x=t2; x<WIDTH-t2; x++){
        for(int y=t2; y<HEIGHT-t2; y++){
            bool isWhite = false;
            uchar here = bgImageYCrCb.at<Vec3b>(y,x)[0];
            if(here>sl){
                for(int t=t1; t<=t2; t++){
                    isWhite |= here-bgImageYCrCb.at<Vec3b>(y,x-t)[0]>sd && here-bgImageYCrCb.at<Vec3b>(y,x+t)[0]>sd;
                    isWhite |= here-bgImageYCrCb.at<Vec3b>(y-t,x)[0]>sd && here-bgImageYCrCb.at<Vec3b>(y+t,x)[0]>sd;
                }
            }
            if(isWhite){
                whiteMask.at<uchar>(y,x) = 255;
            }
        }
    }
    dilate(whiteMask, whiteMask, kernel);
    if(options["debug"]){
        showImage("whiteMask",whiteMask,0);
    }

    Mat lineImg = Mat::zeros( Size(WIDTH, HEIGHT), CV_8U);
    vector<Vec4i> lines, Wlines, Elines, Nlines, Slines;
    HoughLinesP(whiteMask, lines, 1, CV_PI/180, HEIGHT/6, HEIGHT/6, 10 );
    for(auto &l : lines){
        if(isVertical(l)){
            if(l[0]+l[2]<=WIDTH){
                Wlines.push_back(l);
            }
            else{
                Elines.push_back(l);
            }
        }
        else{
            if(l[1]+l[3]<=HEIGHT){
                Nlines.push_back(l);
            }
            else{
                Slines.push_back(l);
            }
        }
        drawWhiteLine(lineImg,l);
    }
    if(options["debug"]){
        showImage("line",lineImg,0);
    }

    double bestScore = -INFINITY;
    Point2f bestNW = {150,100};
    Point2f bestNE = {350,100};
    Point2f bestSW = {100,400};
    Point2f bestSE = {400,400};
    
    int x = 1;
    for(auto &W : Wlines){
        cout << "Court : " << x << "/" << Wlines.size() << " " << bestScore << endl; x++;
        for(auto &E : Elines){
            int vGap = (E[0]+E[2])/2 - (W[0]+W[2])/2;
            if(vGap < 128){
                continue;
            }
            for(auto &N : Nlines){
                for(auto &S : Slines){
                    int hGap = (S[1]+S[3])/2 - (N[1]+N[3])/2;
                    if(hGap < 128){
                        continue;
                    }
                    Point3f tNW, tNE, tSW, tSE;
                    tNW = intersection(N,W);
                    tNE = intersection(N,E);
                    tSW = intersection(S,W);
                    tSE = intersection(S,E);
                    if(tNW.z<0 || tNE.z<0 || tSW.z<0 || tSE.z<0){
                        continue;
                    }
                    Point2f NW(tNW.x,tNW.y), NE(tNE.x,tNE.y), SW(tSW.x,tSW.y), SE(tSE.x,tSE.y);
                    vector<Vec4f> transformedCourtlines;
                    Vec4f transformedMiddleCourtLine;
                    tie(transformedCourtlines, transformedMiddleCourtLine) = getTransformedCourtLines(NW,NE,SW,SE);
                    double score = 0;
                    for(auto &l : transformedCourtlines){
                        for(double r=0; r<=1.0; r+=0.01){
                            int x = l[0]*r + l[2]*(1.0-r);
                            int y = l[1]*r + l[3]*(1.0-r);
                            if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT){
                                score += whiteMask.at<uchar>(y, x);
                            }
                        }
                    }
                    if(score>bestScore){
                        bestScore = score;
                        bestNW = NW;
                        bestNE = NE;
                        bestSW = SW;
                        bestSE = SE;
                    } 
                }
            }
        }
    }

    if(options["save"]){
        ofstream courtfile;
        courtfile.open(getPath("court",input));
        courtfile << bestNW.x << " " << bestNW.y << endl;
        courtfile << bestNE.x << " " << bestNE.y << endl;
        courtfile << bestSW.x << " " << bestSW.y << endl;
        courtfile << bestSE.x << " " << bestSE.y << endl;
    }

    if(options["show"]){
        vector<Vec4f> transformedCourtlines;
        Vec4f transformedMiddleCourtLine;
        tie(transformedCourtlines, transformedMiddleCourtLine) = getTransformedCourtLines(bestNW,bestNE,bestSW,bestSE);
        Mat courtImg = Mat::zeros( Size(WIDTH,HEIGHT), CV_8U);
        for(auto &l : transformedCourtlines){
            drawWhiteLine(courtImg,l);
        }
        drawWhiteLine(courtImg,transformedMiddleCourtLine);

        Mat result = bgImage.clone();
        Mat yellowImage = Mat::zeros( Size(WIDTH, HEIGHT), CV_8UC3);
        yellowImage = color.yellow;
        yellowImage.copyTo(result, courtImg);
        showImage("court",result,0);
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