#pragma once
//
//  physics.h
//  sonOfGesterBall
//

class Physics
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

    static const float defaultGravity;
    static float gravity;
    static GravityType gravityType;
};
