#pragma once
//
//  Ball.h
//  sonOfGesterBall
//

#include <ofMain.h>


// create params:
// initial position, initial speed, color, radius (and thus weight)

class Ball
{
public:
    enum GravityType
    {
        gravityNone = 0,
        gravityGround,
        gravityCenter,
        gravityFingers,
        gravityTypeCount
    };
    friend ostream& operator<<(ostream& os, GravityType gt);
    
    Ball(const ofPoint& _pos, const ofPoint& _speed, int _color, int _radius,
         GravityType _gravityType = gravityGround);
    
    int ballColor(void) const { return color; }
    int ballRadius(void) const { return radius; }
    bool collide(Ball* other);
    bool collide(const ofPoint& other, float otherRadius);
    void maintainMinimumDistance(ofPoint& other, float otherRadius, float dist);
    void stickToFinger(int _fingerId) { stuckToFinger = _fingerId; }
    void setGravityType(GravityType _gravityType);
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
    
    GravityType gravityType;
    
    float mass;
    int  lastUpdateTime;
    ofRectangle viewPort;
    
    ofMaterial material;
};


inline ostream& operator<<(ostream& os, Ball::GravityType gt)
{
    string gtstr;
    
    switch (gt)
    {
        case Ball::gravityNone:
            gtstr = "None";
            break;
        case Ball::gravityGround:
            gtstr = "Ground";
            break;
        case Ball::gravityCenter:
            gtstr = "Center";
            break;
        case Ball::gravityFingers:
            gtstr = "Finger";
            break;
        default:
            gtstr = "unknown="; // + gt;
            break;
    }
    
    os << gtstr;
    return os;
}
