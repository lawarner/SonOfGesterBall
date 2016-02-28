#pragma once

#include <ofMain.h>

#include "automapper.h"
#include "Ball.h"
#include "Leap.h"

#define MAX_FINGERS 10

class ofApp : public ofBaseApp, Leap::Listener
{
public:
    ofApp();
    virtual ~ofApp();

public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
public:
    void tearDown();
    
public: // Leap motion listener implementation
    virtual void onInit(const Leap::Controller& ctrl);
    virtual void onConnect(const Leap::Controller& ctrl);
    virtual void onDisconnect(const Leap::Controller& ctrl);
    virtual void onExit(const Leap::Controller& ctrl);
    virtual void onFrame(const Leap::Controller& ctrl);
    virtual void onFocusGained(const Leap::Controller& ctrl);
    virtual void onFocusLost(const Leap::Controller& ctrl);
    virtual void onDeviceChange(const Leap::Controller& ctrl);
    virtual void onServiceConnect(const Leap::Controller& ctrl);
    virtual void onServiceDisconnect(const Leap::Controller& ctrl);

private:
    int  ballColor(void);
    void clear(void);
    void drawHelpFbo(void);
    void drawMenuFbo(int menu);
    void drawScene(const ofRectangle& viewp);
    void drawWalls(const ofRectangle& viewp);
    void setStereoView(void);
    void setupViewports(void);
    void updateFinger(ofVec3f& finger);
    
    typedef vector<Ball *> BallVector;
    typedef BallVector::iterator BallIterator;
    BallVector balls;
    int ballSize;

private:
    float centerX;
    float centerY;
    float rotation;

    bool isMouse_;    // use mouse until leap comes online
    bool isLaunching_;
    int launchTime_;
    float gravityFactor_;
    
    ofPoint mousePoint;  // not using right now
    
    bool isRecording;
    bool isPlaying;
    bool isStereo;
    
    bool leftView;
    int parallax;
    ofRectangle viewPort[3];
    
    ofTrueTypeFont font;
    
    ofFbo helpFbo;
    ofFbo menuFbo;
    
    ofEasyCam cam;		// camera
    float camDistance;
    ofLight light;
    ofFloatColor lightColor;
    ofMaterial material;
    ofColor materialColor;
    int hue;

    AutoMapper mapper_;
    Leap::Controller leap_;
    // For multi-level menus
    int firstMenuKey_;
};
