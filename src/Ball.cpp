//
//  Ball.cpp
//  sonOfGesterBall
//

#include "Ball.h"

extern ofPoint fingerAt;
extern int fingerCount;
extern ofVec3f fingerPoints[];
extern ofVec3f fingerSpeeds[];



Ball::Ball(const ofPoint& _pos, const ofPoint& _speed, int _color, int _radius)
: position(_pos)
, speed(_speed)
, direction(_speed)
, color(_color)
, radius(_radius)
, stuckToFinger(0)
{
    direction.normalize();
    mass = radius * radius;	// circle
    //	mass = 4.0 / 3.0 * PI * pow((float) radius, 3);		// sphere
    lastUpdateTime = 0;
    
    material.setShininess(0.2);
    //		materialColor.setHue((*it)->ballRadius() * 3);
    ofColor materialColor;
    materialColor.setHex(color);
    ofColor specularColor;
    specularColor.setHex(color);
    specularColor *= 0.75;  // only for diffuse lighting, otherwise would be > 1.0
    material.setSpecularColor(specularColor);	// the light highlight of the material
    material.setDiffuseColor(materialColor);
    material.setAmbientColor(materialColor);
    //	material.setEmissiveColor(materialColor);
}


bool Ball::collide(Ball* other)
{
    float length = position.distance(other->position);
    if (length < radius + other->radius)
    {
        ofPoint fts = position - other->position;
        fts.normalize();
        
        float a1 = speed.dot(fts);
        float a2 = other->speed.dot(fts);
        
        float massTotal = mass + other->mass;
        float optimizedP = (2.0f * (a1 - a2)) / massTotal;
        
        ofPoint dir = speed - optimizedP * other->mass * fts;
        ofPoint otherdir = other->speed + optimizedP * mass * fts;
        
        speed = dir;	// * elasticity
        other->speed = otherdir;
        
        maintainMinimumDistance(other->position, other->radius, length);
        
        return true;
    }
    
    return false;
}


bool Ball::collide(const ofPoint& other, float otherRadius)
{
    float length = position.distance(other);
    if (length < radius + otherRadius)
    {
        ofPoint otherPoint(other);
        maintainMinimumDistance(otherPoint, otherRadius, length);
        
        return true;
    }
    
    return false;
}


void Ball::maintainMinimumDistance(ofPoint& other, float otherRadius, float dist)
{
    float radiiSum = radius + otherRadius;
    if (dist < radiiSum)
    {
        ofPoint diffVect = position - other;
        diffVect.normalize();
        ofPoint newPos = diffVect * ((radiiSum - dist) / 2);
        position += newPos;
        other -= newPos;
    }
}


void Ball::draw(void)
{
    ofPoint pos2z = position * ofPoint(1,1,2);	// because of scale 0.5
    material.begin();
    ofSetHexColor(color);
    ofDrawSphere(pos2z, radius);
    material.end();
    
    // drop shadow
    ofPushMatrix();
    ofTranslate(pos2z.x, ofGetWindowHeight() - 0.01, pos2z.z);
    ofRotateX(90);
    ofFill();
    ofSetColor(0, 0, 0, 60);
    ofDrawCircle(0, 0, 0, radius - 3);
    
    ofPopMatrix();
}

void Ball::update(void)
{
    if (lastUpdateTime == 0)
    {
        lastUpdateTime = ofGetElapsedTimeMillis();
        return;
    }
    
    float interval = (ofGetElapsedTimeMillis() - lastUpdateTime) / 1000.0;
    if (interval < 0.008)
        return;
    
    ofPoint tickSpeed = speed * interval;
    position += tickSpeed;
    
    // bounce off walls
    int winW = viewPort.width;	// ofGetWindowWidth();
    int winH = viewPort.height;
    int winZ = winH;
    
    if (position.x + radius >= winW || position.x - radius <= 0)
    {
        position.x = MAX(radius, MIN(winW - radius, position.x));
        speed.x *= -1;
    }
    if (position.y + radius >= winH || position.y - radius <= 0)
    {
        position.y = MAX(radius, MIN(winH - radius, position.y));
        speed.y *= -1;
    }
    if (position.z + radius >= winZ || position.z - radius <= 0)
    {
        position.z = MAX(radius, MIN(winZ - radius, position.z));
        speed.z *= -1;
    }
    
    // Gravity :
    // g' = (r / re ) g     inside
    // g' = (re^2 / r^2) g  outside
    float dist;
    float gravPull; // = mass * G * pow(interval, 2);
    const float G = Physics::gravity;

    switch (Physics::gravityType)
    {
        case Physics::gravityGround:
            dist = ofGetWindowHeight() - position.y;
            if (dist < radius)
            {
                cout << "Below ground " << dist << " of R=" << radius << endl;
                gravPull = interval * (4.0 / 3.0) * G * dist;	// normally multiply by PI
            }
            else
                gravPull = interval * G * mass * 0.8 / pow(dist * 0.6, 2);
            if (gravPull > 33)	// dampen a bit
            {
                //            cout << "Big gravPull = " << gravPull << endl;
                gravPull = 33;
            }
            speed.y += gravPull;
            break;
        case Physics::gravityCenter:
        {
            int centerX = ofGetViewportWidth() / 2;		// was ofGetWindowWidth()
            int centerY = ofGetWindowHeight() / 2;
            calcGravity(centerX, centerY, centerX, interval, G);
            break;
        }
        case Physics::gravityFingers:
            for (int idx = 0; idx < fingerCount; ++idx)
            {
                calcGravity(fingerPoints[idx].x, fingerPoints[idx].y, fingerPoints[idx].z, interval, G * 8);
            }
            break;
        default:
            break;
    }
    
    if (stuckToFinger)
    {
        position = fingerAt;
    }
    
    lastUpdateTime = ofGetElapsedTimeMillis();
}


void Ball::calcGravity(int mx, int my, int mz, float interval, float g)
{
    float gravPull;
    float dist = position.distance(ofPoint(mx, my, mz)) * 0.5;
#if 1
    // Note the 0.5 below is to increase the range (distance) of gravitational pull
    if (dist < radius)
    {
        //gravPull = interval * g * mass * (radius - dist) * 0.8 / (radius * 10);
        gravPull = 0.0001;
        speed *= 0.91;
    }
    else
        gravPull = interval * g * mass * 0.5 / pow(dist, 2);
    gravPull = CLAMP(gravPull, 0, 48);
#else
    if (dist < radius)
        g = radius / dist * G;
    else
        g = pow((float) radius * 0.5, 2) / pow(dist * 0.5, 2) * G;
    gravPull *= g;
#endif
    if (position.x < mx)
        speed.x += gravPull;
    else if (position.x > mx)
        speed.x -= gravPull;
    if (position.y < my)
        speed.y += gravPull;
    else if (position.y > my)
        speed.y -= gravPull;
    if (position.z < mz)
        speed.z += gravPull;
    else if (position.z > my)
        speed.z -= gravPull;
}


float Ball::kineticEnergy(void) const
{
    return mass * speed.length();
}
