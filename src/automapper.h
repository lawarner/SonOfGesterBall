#pragma once
//
//  automapper.h
//  sonOfGesterBall
//

#include <ofMain.h>
#include "Leap.h"

/**
 *  Map coordinates between model space and world space.
 *
 *  Note the model's bounds are set on construction and are immutable.
 *  The matching world bounds are auto-expanded to fit within the model as
 *  new data points are encountered, but only if the map is not in a frozen
 *  state (see freeze() below).
 */
class AutoMapper
{
public:
    // This is the most obvious constructor, but others are possible for convenience (TODO)
    // When fromCenter is true to use from calibration where user hovers at desired centerpoint
    AutoMapper(const ofPoint& modelDimensions, const ofPoint& modelReferencePt, bool fromCenter = false);
#if 0
    /** modelOffset is diagonal distance from point(0,0,0) */
    AutoMapper(const ofPoint& modelDimensions, float modelOffset);
#endif
    
    void freeze(bool doFreeze = true);
    void printMap();
    void reset();
    
    // When set the map is also frozen
    void set(const ofPoint& worldNearLL, const ofPoint& worldFarUR);
    void set(const Leap::Vector& worldNearLL, const Leap::Vector& worldFarUR);

    void setModel(const ofPoint& modelDimensions, const ofPoint& modelReferencePt, bool fromCenter = false);
    
    void worldToModel(const Leap::Vector& finger, ofPoint& point);

private:
    bool frozen_;
    /** Model near lower left */
    ofPoint modelNearLL_;
    /** Model far upper right */
    ofPoint modelFarUR_;
    
    ofPoint worldNearLL_;
    ofPoint worldFarUR_;

    Leap::Vector lastPoint_;
    float maxOffset_;
};
