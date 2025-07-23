#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include "../webrequest/ThumbRequest.hpp"
#include <gdutilsdevs.gdutils/include/RateEvent.hpp>

using namespace geode::prelude;

class BetterThumbnailLayer : public CCLayer {
    ThumbRequest thumbRequest;
public:
    static BetterThumbnailLayer *create();
    static CCScene *scene();
    bool init();
    void onBackButton(CCObject *);
    void keyBackClicked() override;
private:
    void onMyThumbnail(CCObject*);
    void onRecent(CCObject*);
    void onPending(CCObject*);
    void onManage(CCObject*);
};
