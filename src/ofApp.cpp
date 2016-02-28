
#include "ofApp.h"
#include "ofxTweenzor.h"

#include "physics.h"


// Some globals for now
ofVec3f fingerAt;
int fingerCount;
ofVec3f fingerPoints[MAX_FINGERS];
ofVec3f fingerSpeeds[MAX_FINGERS];

float helpAlpha;
float menuAngle;

static const char* helpString =
"<sp> Clear all balls     l  Launch ball\n"
" d   Deserialize         p  Playback recorded points\n"
" f   Toggle fullscreen   m  Mapper commands\n"
" g   Change gravity      r  Stop/start recording\n"
"                       -,+  Change z depth of mouse\n"
" h,? Help              0-9  Set ball size\n";

static const char* gravityMenuString =
" GRAVITY OPTIONS:\n"
"  c   Center\n"
"  f   Fingers\n"
"  g   Ground\n"
"  n   None\n"
" +/-  Change gravity 10%\n"
" 0-9  Set gravity magnitude";

static const char* mapMenuString =
" MAPPER COMMANDS:\n"
"  f  Freeze map\n"
"  p  Print map\n"
"  r  Reset (and thaw) map\n"
"  t  Thaw map (unfreeze)";


ofApp::ofApp()
: isMouse_(true)
, launchTime_(0)
, gravityFactor_(4)
, mapper_(ofPoint(ofGetWindowWidth(), ofGetWindowHeight(), ofGetWindowHeight()), ofPoint(0,0,0))
, firstMenuKey_(0)
{
    fingerCount = 0;
    //mapper_.set(ofPoint(-200, 10, -128), ofPoint(68, 250, 190));
    // World: -244.206, 0, -128.671 to 67.7722, 258.394, 152.245
    // fullscreen, after
    //World: -185.543, 0, -140.934 to 165.885, 447.236, 176.862
    mapper_.set(ofPoint(-180, 0, -140), ofPoint(165, 430, 170));
}

ofApp::~ofApp()
{
    tearDown();
}

int ofApp::ballColor()
{
    int r = ofMap(ballSize, 7, 70, 0, 255);
    int g = ofMap(ballSize, 7, 70, 128, 1413);
    int b = ofMap(ballSize, 7, 70, 567, 0);
    g %= 256;
    b %= 256;
    int color = (r << 16) | (g << 8) | b;
    //	cout << "size=" << ballSize << ", color=0x" << hex << color << dec << endl;
    return color;
}

void ofApp::clear(void)
{
    for (BallIterator it = balls.begin(); it != balls.end(); it++)
        delete *it;
    
    balls.clear();
}

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofBackground(30, 30, 34);
    //	ofEnableAlphaBlending();
    ofSetVerticalSync(true);
    ofSetFrameRate(60); // if vertical sync is off, we can go a bit fast
    
    glEnable(GL_DEPTH_TEST);
    
    ofSetSmoothLighting(true);
    
    Tweenzor::init();
    
    ofEnableSmoothing();
    
    ///	ofHideCursor();
    isLaunching_ = false;
    isRecording = false;
    isPlaying = false;
    isStereo = false;
    leftView = false;
    
    parallax = 0;
    ballSize = 50;
    
    font.load("courbd.ttf", 22);
    
    setupViewports();
    
    mousePoint.z = centerY;
    
    camDistance = ofGetWindowHeight() / 2.3;	// 360;
    cam.setDistance(camDistance);
    //	cam.setTarget(ofPoint(centerX, centerY, ofGetWindowHeight()));
    ///	cam.cacheMatrices();
    
    ofSetSphereResolution(32);
    
#if 1
    material.setShininess(0.75);
    //		materialColor.setHue((*it)->ballRadius() * 3);
    ofColor materialColor;
    materialColor.setHex(0x797996);
    // the light highlight of the material //
    material.setSpecularColor(ofColor(222,200,200));
    material.setDiffuseColor(ofColor::darkSlateBlue);
    material.setAmbientColor(materialColor);
#else
    // the light highlight of the material //
    material.setSpecularColor(ofColor(200,60,80));
    // shininess is a value between 0 - 128, 128 being the most shiny //
    material.setShininess( 77 );
    materialColor.set(220, 220, 220);
    materialColor.setBrightness(250.f);
    materialColor.setSaturation(200);
    material.setDiffuseColor(materialColor);
#endif
    //	light.setPointLight();
    //	light.setDirectional();
    //    lightColor.setBrightness( 90.f );  // ( 180.f );
    //    lightColor.setSaturation( 150.f );
    //	lightColor.setHue(100);
    lightColor.set(0.9, 0.9, 0.94, 1.0);
    light.setDiffuseColor(lightColor);
    light.setAmbientColor(lightColor);
    
    // Point lights emit light in all directions //
    // set the diffuse color, color reflected from the light source //
    //    light.setDiffuseColor( ofColor(155.f, 155.f, 175.f));
    
    // specular color, the highlight/shininess color
    light.setSpecularColor( ofFloatColor(0.7f, 0.7f, 0.7f));
    
    helpAlpha = 0;
    menuAngle = 0;
    helpFbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA);
    menuFbo.allocate(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2, GL_RGBA);

    leap_.addListener(*this);
}

void ofApp::tearDown()
{
    std::cout << "Tear down the App" << std::endl;
    leap_.removeListener(*this);
}

void ofApp::setupViewports(void)
{
    ofRectangle screenRect(0, 0, ofGetScreenWidth(), ofGetScreenHeight());
    ofRectangle windowRect(ofGetWindowPositionX(), ofGetWindowPositionY(), ofGetWindowWidth(), ofGetWindowHeight());
    
    centerY = ofGetWindowHeight() / 2;
    if (isStereo)		// split screen stereo
        centerX = ofGetWindowWidth() / 4;
    else
        centerX = ofGetWindowWidth() / 2;
    
    int centerWind = ofGetWindowWidth() / 2;
    
    viewPort[0].set(0, 0, windowRect.width, windowRect.height);
    viewPort[1].set(0, 0, centerWind, windowRect.height);
    viewPort[2].set(centerWind, 0, centerWind, windowRect.height);
}

//--------------------------------------------------------------
void ofApp::drawHelpFbo()
{
    //int y = 16 + font.getLineHeight();
    ofPushMatrix();
    helpFbo.begin();

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + 0);
    ofClear(0, 0, 0, 0);
    ofRotateX(180);
    ofTranslate(0, -ofGetWindowHeight() + font.getLineHeight() + 16, 0);
    ofSetColor(140, 212, 212, helpAlpha);
    font.drawString(helpString, 16, 0);
    
    helpFbo.end();
    ofPopMatrix();
}

void ofApp::drawMenuFbo(int menu)
{
    ofPushMatrix();
    menuFbo.begin();

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + 0);
    ofClear(255, 255, 255, 0);
    ofRotateX(180);
    ofTranslate(0, -centerY, 0);
//    ofRotateY(menuAngle);
    ofSetColor(3, 3, 190, 120);
    ofDrawRectangle(0, 0, centerX, centerY);
    ofTranslate(0, 0, -1);
    ofSetColor(29, 255, 29, 255);

    float ypos = font.getLineHeight() * 2 + 8;
    const char* strMenu = mapMenuString;
    if (menu == 'g')
    {
        char currG[64];
        snprintf(currG, sizeof(currG), "  G = %d", (int) Physics::gravity);
        font.drawString(currG, 20, ypos - 8);
        ypos += font.getLineHeight() + 8;
        strMenu = gravityMenuString;
    }
    font.drawString(strMenu, 20, ypos);
    
    menuFbo.end();
    ofPopMatrix();
}

void ofApp::updateFinger(ofVec3f& finger)
{
    finger.set(ofBaseApp::mouseX, ofBaseApp::mouseY, mousePoint.z);
}

//--------------------------------------------------------------
void ofApp::update()
{
    int currMillis = ofGetElapsedTimeMillis();
    
    //updateFinger(fingerPoint);
    if (isMouse_)
    {
        fingerCount = 1;
        updateFinger(fingerPoints[0]);
    }

    Tweenzor::update(currMillis);
    
    for (int idx = 0; idx < fingerCount; ++idx)
    {
        fingerAt = fingerPoints[idx];
        
        if (isLaunching_)
        {
            if (currMillis - launchTime_ > 112)
            {
                ofPoint speed = fingerSpeeds[idx] * 2.2;
                float speedLen = speed.length();
                //cout << " - speed is (" << speedLen << ") mm / second" << endl;

                float thresholdSpeed = isMouse_ ? 100 : 200;
                if (speedLen > thresholdSpeed)
                {
                    //isLaunching_ = false;
                    launchTime_ = currMillis;
                    fingerSpeeds[idx].set(0, 0, 0);
                    Ball* ball = new Ball(fingerAt, speed, ballColor(), ballSize);
                    ball->setViewport(ofGetCurrentViewport());
                    balls.push_back(ball);
                }
            }
        }
    }
    
    if (helpAlpha > 0)
    {
        drawHelpFbo();
    }
    if (firstMenuKey_)
    {
        drawMenuFbo(firstMenuKey_);
    }
    
    for (BallIterator it = balls.begin(); it != balls.end(); it++)
    {
        (*it)->setViewport(isStereo ? viewPort[1] : viewPort[0]);
        (*it)->update();
        
        BallIterator jt = it;
        jt++;
        for ( ; jt != balls.end(); jt++)
            (*it)->collide(*jt);
#if 0
        if (isLaunching_)
        {
            (*it)->stickToFinger(0);
        }
        else
        if ((*it)->collide(fingerPoints_[0], 16))
        {
            (*it)->stickToFinger(1);
        }
#endif
    }
    
}

//--------------------------------------------------------------
void ofApp::draw()
{
    //	ofSetRectMode(OF_RECTMODE_CENTER);
    ofBackground(34, 34, 34);

    if (isStereo)
    {
        drawScene(viewPort[1]);
        drawScene(viewPort[2]);
    }
    else
        drawScene(viewPort[0]);
}


void ofApp::drawScene(const ofRectangle& viewp)
{
    cam.begin(viewp);
#if 0
    // Set the aspect ratio
    float ar = (float) ofGetViewportWidth() / ofGetViewportHeight();
    float scaleX, scaleY, scaleZ;
    if (ar < 1.0f)
    {
        scaleX = 1.0 / ar;
        scaleY = 1.0;
    } else {
        scaleX = 1.0;
        scaleY = ar;
    }
    scaleZ = 1.0;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-scaleX, scaleX, -scaleY, scaleY, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    // #else
    glMatrixMode(GL_PROJECTION);
    cam.setScale(scaleX, scaleY, scaleZ);
    glMatrixMode(GL_MODELVIEW);
#endif
    //		ofRotateX(ofRadToDeg(.5));
    //		ofRotateY(ofRadToDeg(-.5));
    
    if (isStereo)
    {
        parallax = -parallax;
        cam.setScale(1, 1, 1);
    }
    
    ofPushMatrix();

    if (helpAlpha > 0 || firstMenuKey_)
    {
        ofEnableAlphaBlending();
    }
    
    int centerViewX = viewp.width / 2;
    ofTranslate(-centerViewX, -centerY, -camDistance);
    ofRotateZ(180);
    ofRotateY(180);
    ofTranslate(parallax, -ofGetWindowHeight(), 0);
    
    light.setDiffuseColor(lightColor);
    light.setSpecularColor(ofFloatColor(0.7f, 0.7f, 0.7f));
    //	light.setSpecularColor(lightColor);
    //    ofSetColor(light.getDiffuseColor());	// draw the light
    //    ofSphere(light.getPosition(), 18.f);
    //	ofSphere(ofPoint(centerX, centerY, viewp.height), 18.f);
    //light.setPosition(centerX, viewp.height * 1.2, centerX);
    //light.lookAt(ofPoint(viewp.width, viewp.height, viewp.height));
    
#if 0
    light.setGlobalPosition(centerViewX, -40, centerY + 100);
    light.setDirectional();
    light.setAmbientColor(0xffffff);
    light.setDiffuseColor(0xffffff);
    light.setSpecularColor(0xee88cc);
#endif

    for (int idx = 0; idx < fingerCount; ++idx)
    {
        ofVec3f currentFinger = fingerPoints[idx];
        if (isLaunching_)
        {
            ofSetHexColor(ballColor());	// draw ball outline
            ofNoFill();
            ofSetLineWidth(3);
            ofDrawCircle(currentFinger, ballSize);
            ofFill();
        }
        else
        {
            ofSetLineWidth(3);
            ofNoFill();
            ofSetHexColor(0x50ff50);	// draw green fingertip
            ofDrawCircle(currentFinger, 16);
        }
    }
    
    drawWalls(viewp);
    
    if (helpAlpha > 0)
    {
        ofPushMatrix();
        ofTranslate(0, 0, ofGetWindowHeight());
        ofSetHexColor(0xffffff);
        helpFbo.draw(0, 0);
        ofPopMatrix();
    }
    if (firstMenuKey_)
    {
        ofPushMatrix();
        ofTranslate(0, 0, centerY * 2 / 3);
        ofRotate(menuAngle, 1, 1, 1);
//        ofRotateY(menuAngle);
        //ofSetHexColor(0xefefff);
        menuFbo.draw(centerX / 2, centerY / 2);
        ofPopMatrix();
    }
    
    ofEnableLighting();
    // the position of the light must be updated every frame,
    // call enable() so that it can update itself //
    light.enable();
    light.setPosition(viewp.height * 1.2, centerX, centerX);
    light.setGlobalPosition(centerX, viewp.height * 1.2, centerX);
    
    ofFill();
    ofPushMatrix();
    ofRotateX(90);
    ofTranslate(0, -viewp.height, -viewp.height);
    material.begin();	// draw a floor
    ofSetHexColor(0xf0f0ff);
    ofDrawRectangle(0, viewp.height, 0, viewp.width, viewp.height);
    material.end();
    ofPopMatrix();
    
    ofPushMatrix();
    ofScale(1.0, 1.0, 0.5);
    for (BallIterator it = balls.begin(); it != balls.end(); it++)
    {
        (*it)->draw();
    }
    ofPopMatrix();
    
    light.disable();
    ofDisableLighting();
    
    ofPopMatrix();
    
    cam.end();
}


void ofApp::drawWalls(const ofRectangle& viewp)
{
    ofSetColor(255, 100, 100);
    ofSetLineWidth(2);
    
    int ww = viewp.width;
    int wh = viewp.height;
    int wz = wh;
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(0, 0, wz);
    glVertex3f(ww, 0, wz);
    glVertex3f(ww, wh, wz);
    glVertex3f(0, wh, wz);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(0, 0, 0);
    glVertex3f(ww, 0, 0);
    glVertex3f(ww, wh, 0);
    glVertex3f(0, wh, 0);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, wz);
    glVertex3f(ww, 0, 0);
    glVertex3f(ww, 0, wz);
    glVertex3f(0, wh, 0);
    glVertex3f(0, wh, wz);
    glVertex3f(ww, wh, 0);
    glVertex3f(ww, wh, wz);
    glEnd();
    
}


void ofApp::setStereoView()
{
#if 0
    int length = videoW*videoH*3;
    for(int i = 0; i < length; i+=3)
    {
        resultPixels[i] = rightPixels[i];
        resultPixels[i+1] = (leftPixels[i+1]*9 + rightPixels[i+1]) / 10;
        resultPixels[i+2] = (leftPixels[i+2]*9 + rightPixels[i+2]) / 10;
    }
#endif
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key0)
{
    int key = tolower(key0);

    switch (firstMenuKey_ ? firstMenuKey_ : key)
    {
        case ' ':
            clear();
            break;
        case 'd':
            //tracker.deserialize("gestures1");
            break;
        case 'f':
            ofToggleFullscreen();
            mapper_.setModel(ofPoint(ofGetWindowWidth(), ofGetWindowHeight(), ofGetWindowHeight()), ofPoint(0,0,0));
            break;
        case 'g':
            if (firstMenuKey_)
            {
                firstMenuKey_ = 0;
                Physics::GravityType lastGravityType = Physics::gravityType;
                switch (key)
                {
                    case 'c': // Center
                        Physics::gravityType = Physics::gravityCenter;
                        break;
                    case 'f': // Fingers
                        Physics::gravityType = Physics::gravityFingers;
                        break;
                    case 'g': // Ground
                        Physics::gravityType = Physics::gravityGround;
                        break;
                    case 'n': // None
                        Physics::gravityType = Physics::gravityNone;
                        break;
                    case '+':
                    case '-':
                    {
                        Physics::gravity += (key == '+' ? 1 : -1) * Physics::gravity * 0.1;
                    }
                        break;
                    default:
                        if (key >= '0' && key <= '9')
                        {
                            gravityFactor_ = key - '0';
                            float gf = pow(10, gravityFactor_);
                            Physics::gravity = 2.6 * gf;
                        }
                        break;
                }
            }
            else
            {
                firstMenuKey_ = key;
                Tweenzor::removeTween(&menuAngle);
                Tweenzor::add(&menuAngle, 180, 0, 0, 1, EASE_OUT_CUBIC);
            }
            break;
        case 'h':
        case '?':
            if (helpAlpha == 0)
            {
                Tweenzor::removeTween(&helpAlpha);
                Tweenzor::add(&helpAlpha, 0, 255, 0, 2, EASE_OUT_QUAD);
            }
            else
            {
                Tweenzor::removeTween(&helpAlpha);
                Tweenzor::add(&helpAlpha, 255, 0, 0, 2, EASE_OUT_QUAD);
            }
            break;
        case 'i':
            if (key0 == 'I')
                ;
            else
                ;
            break;
        case 'l':
            if (isLaunching_ && isMouse_) {
                Ball* ball = new Ball(fingerAt, ofPoint(), ballColor(), ballSize);
                ball->setViewport(ofGetCurrentViewport());
                balls.push_back(ball);
            }
            isLaunching_ = !isLaunching_;
            break;
        case 'm':  // automapper commands
            if (firstMenuKey_)
            {
                switch (key)
                {
                    case 'f':
                        mapper_.freeze();
                        break;
                    case 'p':
                        mapper_.printMap();
                        ofLogVerbose() << "Window: " << ofGetWindowSize() << std::endl;
                        break;
                    case 'r':
                        mapper_.reset();
                        break;
                    case 't':
                        mapper_.freeze(false);
                        break;
                }
                firstMenuKey_ = 0;
            }
            else
            {
                firstMenuKey_ = key;
                Tweenzor::removeTween(&menuAngle);
                Tweenzor::add(&menuAngle, 180, 0, 0, 2, EASE_OUT_CUBIC);
            }
            break;
        case 'p':
            //isPlaying = !tracker.isPlayingback();
            //tracker.playback(isPlaying);
            break;
        case 'r':
            isRecording = !isRecording;
            //tracker.record(isRecording);
            //if (!isRecording)
            //    tracker.serialize("gestures1");
            break;
        case 's':
            isStereo = !isStereo;
            ofSetVerticalSync(isStereo);
            parallax = isStereo ? -10 : 0;
            break;
        case '-':
        case '_':
            mousePoint.z -= 10;
            ofLogVerbose() << "Mouse depth " << mousePoint.z << endl;
            break;
        case '+':
        case '=':
            mousePoint.z += 10;
            ofLogVerbose() << "Mouse depth " << mousePoint.z << endl;
            break;
        default:
            if (key >= '0' && key <= '9')
                ballSize = (key - '0' + 1) * 7;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
    mousePoint.x = x;
    mousePoint.y = y;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
    std::cout << "Mouse dragged " << x << "," << y << std::endl;
    //mousePoint.set(x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    //mousePoint.z = 99;
    fingerSpeeds[0].set(2, 97, 6);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    //mousePoint.z = -50;
    fingerSpeeds[0].set(0, 0, 0);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
    setupViewports();
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
    std::cout << "ofApp got message: " << msg.message << std::endl;
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
    
}

//--------------------------------------------------------------
//   LISTENER FOR LEAP MOTION
//--------------------------------------------------------------

void ofApp::onInit(const Leap::Controller& ctrl)
{
    std::cout << "Leap Motion toolkit initialized" << std::endl;
}

void ofApp::onConnect(const Leap::Controller& ctrl)
{
    ofLogVerbose() << "Switching to non-mouse" << std::endl;
    isMouse_ = false;
}

void ofApp::onDisconnect(const Leap::Controller& ctrl)
{
    
}

void ofApp::onExit(const Leap::Controller& ctrl)
{
    
}

void ofApp::onFrame(const Leap::Controller& ctrl)
{
    const Leap::Frame frame = ctrl.frame();
    int numFingers = frame.fingers().extended().count();
    // need 2 frames in a row that agree
    if (numFingers != fingerCount)
    {
        fingerCount = numFingers;
        return;
    }
    for (int idx = 0; idx < fingerCount; ++idx)
    {
        const float FACTOR = 0.09;  // speed control
        const Leap::Finger finger = frame.fingers().extended()[idx];
        const Leap::Vector fingerSpeed = finger.tipVelocity() * FACTOR;

        //TODO kick outliers
        mapper_.worldToModel(finger.tipPosition(), fingerPoints[idx]);
        fingerSpeeds[idx].set(fingerSpeed.x, fingerSpeed.y, fingerSpeed.z);
    }
}

void ofApp::onFocusGained(const Leap::Controller& ctrl)
{
    
}

void ofApp::onFocusLost(const Leap::Controller& ctrl)
{
    
}

void ofApp::onDeviceChange(const Leap::Controller& ctrl)
{
    
}

void ofApp::onServiceConnect(const Leap::Controller& ctrl)
{
    
}

void ofApp::onServiceDisconnect(const Leap::Controller& ctrl)
{
    
}
