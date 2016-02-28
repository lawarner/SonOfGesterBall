//
//  physics.cpp
//  sonOfGesterBall
//

#include "physics.h"

const float Physics::defaultGravity = 26000;		// gravity in pixels / second^2
float Physics::gravity = 26000;		// gravity in pixels / second^2


Physics::GravityType Physics::gravityType = Physics::gravityGround;
