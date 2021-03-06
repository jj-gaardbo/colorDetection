#include <iostream>

#include <termios.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

#define KEY_RED 49
#define KEY_GREEN 50
#define KEY_BLUE 51
#define KEY_MAGENTA 52
#define KEY_CYAN 53
#define KEY_YELLOW 54

int iLastX = -1;
int iLastY = -1;

bool USING_CAM = false;



/*===============================================
 * ========= Update the trackbar values =========
 * ============================================*/
void changeTrackbarValues(int v1, int v2, int v3, int v4, int v5, int v6) {
    cvSetTrackbarPos("LowH", "Control", v1);
    cvSetTrackbarPos("HighH", "Control", v2);
    cvSetTrackbarPos("LowS", "Control", v3);
    cvSetTrackbarPos("HighS", "Control", v4);
    cvSetTrackbarPos("LowV", "Control", v5);
    cvSetTrackbarPos("HighV", "Control", v6);
}



/*===============================================
 * ========= Used for detecting key press =======
 * ============================================*/
static struct termios old, new1;
/* Initialize new1 terminal i/o settings */
void initTermios(int echo) {
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    new1 = old; /* make new1 settings same as old settings */
    new1.c_lflag &= ~ICANON; /* disable buffered i/o */
    new1.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &new1); /* use these new1 terminal i/o settings now */
}
/* Restore old terminal i/o settings */
void resetTermios(void) {
    tcsetattr(0, TCSANOW, &old);
}
/* Read 1 character without echo */
char getch(void) {
    char ch;
    initTermios(0);
    ch = getchar();
    resetTermios();
    return ch;
}
void checkKeyPress() {
    switch (getch()) {
        case KEY_RED:
            changeTrackbarValues(0, 94, 0, 36, 150, 255);
            cout << endl << "Red" << endl;
            break;
        case KEY_GREEN:
            changeTrackbarValues(0, 22, 232, 255, 0, 45);
            cout << endl << "Green" << endl;
            break;
        case KEY_BLUE:
            changeTrackbarValues(164, 255, 0, 31, 0, 45);
            cout << endl << "Blue" << endl;
            break;
        case KEY_MAGENTA:
            changeTrackbarValues(173, 255, 0, 255, 227, 255);
            cout << endl << "Magenta" << endl;
            break;
        case KEY_CYAN:
            changeTrackbarValues(177, 255, 58, 255, 0, 255);
            cout << endl << "Cyan" << endl;
            break;
        case KEY_YELLOW:
            changeTrackbarValues(0, 255, 204, 255, 90, 255);
            cout << endl << "Yellow" << endl;
            break;
        default:
            break;
    }
}


int main(int argc, char **argv) {
    Mat image;
    String imageName("test.jpg");
    image = imread(imageName, CV_LOAD_IMAGE_COLOR);
    VideoCapture cap(0); //capture the video from web cam
    if(!USING_CAM){
        if (image.empty()) {
            cout << "Could not open or find the image" << endl;
            return -1;
        }
    } else {
        if ( !cap.isOpened() )  // if not success, exit program
        {
            cout << "Cannot open the web cam" << endl;
            return -1;
        }
    }

    //Create a black image with the size as the camera output
    Mat imgLines = Mat::zeros(image.size(), CV_8UC3);


    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

    int iLowH = 0;
    int iHighH = 255;

    int iLowS = 0;
    int iHighS = 255;

    int iLowV = 0;
    int iHighV = 255;

    cvCreateTrackbar("LowH", "Control", &iLowH, 255); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &iHighH, 255);

    cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &iHighS, 255);

    cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &iHighV, 255);

    while (1) {

        if(USING_CAM){
            bool bSuccess = cap.read(image); // read a new frame from video
            if (!bSuccess) //if not success, break loop
            {
                cout << "Cannot read a frame from video stream" << endl;
                break;
            }
            Mat imgHSV;
            cvtColor(image, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
        }

        Mat imageThresholded;
        inRange(image, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imageThresholded);

        checkKeyPress();

        //Calculate the moments of the thresholded image
        Moments oMoments = moments(imageThresholded);
        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;
        // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
        if (dArea > 10000) {
            //calculate the position of the ball
            int posX = dM10 / dArea;
            int posY = dM01 / dArea;

            if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
                //Draw a red line from the previous point to the current point
                line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0, 0, 255), 2);
            }

            cout << "X: " << posX << "\n" << "Y: " << posY << endl;

            iLastX = posX;
            iLastY = posY;
        }

        //morphological opening (remove small objects from the foreground)
        erode(imageThresholded, imageThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(imageThresholded, imageThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        //morphological closing (fill small holes in the foreground)
        dilate(imageThresholded, imageThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        erode(imageThresholded, imageThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        imshow("Thresholded", imageThresholded);

        imshow("Original", image);
        char a = waitKey(33);
        if (a == 27) break;
    }


    waitKey(0);
    return 0;

}
