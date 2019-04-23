#pragma once
//
//  Ball.h
//  sonOfGesterBall
//

#include <ofMain.h>

#include "physics.h"


// create params:
// initial position, initial speed, color, radius (and thus weight)

class Ball
{
public:
    friend ostream& operator<<(ostream& os, Physics::GravityType gt);
    
    Ball(const ofPoint& _pos, const ofPoint& _speed, int _color, int _radius);
    
    int ballColor(void) const { return color; }
    int ballRadius(void) const { return radius; }
    bool collide(Ball* other);
    bool collide(const ofPoint& other, float otherRadius);
    void maintainMinimumDistance(ofPoint& other, float otherRadius, float dist);
    void stickToFinger(int _fingerId) { stuckToFinger = _fingerId; }
    void setViewport(const ofRectangle& viewp) { viewPort = viewp; }
    void draw(void);
    void update(void);
    
    float kineticEnergy(void) const;
    
private:
    void calcGravity(int mx, int my, int mz, float interval, float g);
    
    ofPoint position;
    ofPoint speed;
    ofPoint direction;
    
    int color;
    int radius;
    int stuckToFinger;
    
    float mass;
    int  lastUpdateTime;
    ofRectangle viewPort;
    
    ofMaterial material;
};


inline ostream& operator<<(ostream& os, Physics::GravityType gt)
{
    switch (gt)
    {
        case Physics::gravityNone:
            os << "None";
            break;
        case Physics::gravityGround:
            os << "Ground";
            break;
        case Physics::gravityCenter:
            os << "Center";
            break;
        case Physics::gravityFingers:
            os << "Finger";
            break;
        default:
            os << "unknown=" << gt;
            break;
    }
    return os;
}
