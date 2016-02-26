//
//  automapper.cpp
//  sonOfGesterBall
//

#include "automapper.h"


AutoMapper::AutoMapper(const ofPoint& modelDimensions, const ofPoint& modelReferencePt, bool fromCenter)
: frozen_(false)
, modelNearLL_(modelReferencePt)
, modelFarUR_(modelReferencePt + modelDimensions)
, maxOffset_(20)
{
    if (fromCenter)
    {
        ofPoint halfDim = modelDimensions / 2;
        modelNearLL_ -= halfDim;
        modelFarUR_  -= halfDim;
    }
}

void AutoMapper::freeze(bool doFreeze)
{
    frozen_ = doFreeze;
}

void AutoMapper::printMap()
{
    std::cout << "World: " << worldNearLL_ << " to " << worldFarUR_ << std::endl;
}

void AutoMapper::reset()
{
    frozen_ = false;
    worldNearLL_.set(0, 0, 0);
    worldFarUR_.set(0, 0, 0);
    lastPoint_ = Leap::Vector::zero();
}

// World: -244.206, 0, -128.671 to 67.7722, 258.394, 152.245
void AutoMapper::set(const ofPoint& worldNearLL, const ofPoint& worldFarUR)
{
    frozen_ = true;
    worldNearLL_ = worldNearLL;
    worldFarUR_ = worldFarUR;
}

void AutoMapper::set(const Leap::Vector& worldNearLL, const Leap::Vector& worldFarUR)
{
    set(ofPoint(worldNearLL.x, worldNearLL.y, worldNearLL.z), ofPoint(worldFarUR.x, worldFarUR.y, worldFarUR.z));
}

void AutoMapper::setModel(const ofPoint& modelDimensions, const ofPoint& modelReferencePt, bool fromCenter)
{
    modelNearLL_ = modelReferencePt;
    modelFarUR_ = modelReferencePt + modelDimensions;
    if (fromCenter)
    {
        ofPoint halfDim = modelDimensions / 2;
        modelNearLL_ -= halfDim;
        modelFarUR_  -= halfDim;
    }
}

void AutoMapper::worldToModel(const Leap::Vector& finger, ofPoint& point)
{
    //TODO some sanity checks on finger, resistance
    //TODO return bool when model is expanded
    ofPoint ofinger(finger.x, finger.y, finger.z);
    ofPoint lowleft = ofinger - worldNearLL_;
    ofPoint upright = ofinger - worldFarUR_;
    if (!frozen_)
    {
        if (lowleft.x < 0 || lowleft.y < 0 || lowleft.z < 0)
        {
            //std::cout << "Outside of world lower-left bounds." << std::endl;
            lowleft.x = CLAMP(lowleft.x, -maxOffset_, 0);
            lowleft.y = CLAMP(lowleft.y, -maxOffset_, 0);
            lowleft.z = CLAMP(lowleft.z, -maxOffset_, 0);
            //if (lowleft.x > 0) lowleft.x = 0;
            worldNearLL_ += lowleft;
        }
        else if (upright.x > 0 || upright.y > 0 || upright.z > 0)
        {
            //std::cout << "Outside of world upper-right bounds." << std::endl;
            upright.x = CLAMP(upright.x, 0, maxOffset_);
            upright.y = CLAMP(upright.y, 0, maxOffset_);
            upright.z = CLAMP(upright.z, 0, maxOffset_);
            worldFarUR_ += upright;
        }
    }
    point.x = ofMap(finger.x, worldNearLL_.x, worldFarUR_.x, modelNearLL_.x, modelFarUR_.x, frozen_);
    point.y = ofMap(finger.y, worldNearLL_.y, worldFarUR_.y, modelFarUR_.y, modelNearLL_.y, frozen_);
    point.z = ofMap(finger.z, worldNearLL_.z, worldFarUR_.z, modelFarUR_.z, modelNearLL_.z, frozen_);

    lastPoint_ = finger;
}
