
#include "ofMain.h"
#include "ofApp.h"

int main(int argc, const char* argv[])
{
    // parameters: width and height, OF_WINDOW or OF_FULLSCREEN
	ofSetupOpenGL(1024,768,OF_FULLSCREEN);

    ofApp* app = new ofApp;
	ofRunApp(app);    // this kicks off the running of my app

    return 0;
}
