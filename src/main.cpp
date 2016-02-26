#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main(int argc, const char* argv[])
{
    // this kicks off the running of my app
    // parameters: width and height, OF_WINDOW or OF_FULLSCREEN
	ofSetupOpenGL(1024,768,OF_WINDOW);

    ofApp* app = new ofApp;
	ofRunApp(app);

    return 0;
}
