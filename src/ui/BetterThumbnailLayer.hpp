#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include "../ThumbRequest.hpp"
#include <gdutilsdevs.gdutils/include/RateEvent.hpp>

using namespace geode::prelude;

class BetterThumbnailLayer : public CCLayer {
    ThumbRequest thumbRequest;
public:
    static BetterThumbnailLayer *create();
    static CCScene *scene();
    bool init();
    void onBackButton(CCObject *);
};
