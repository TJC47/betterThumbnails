#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class BetterThumbnailLayer : public CCLayer
{

public:
    static BetterThumbnailLayer *create();
    static CCScene *scene();
    bool init() override;
    void onBackButton(CCObject *);
    void keyBackClicked() override;

private:
    void onMyThumbnail(CCObject *);
    void onRecent(CCObject *);
    void onPending(CCObject *);
    void onManage(CCObject *);
    void onInfoButton(CCObject *);
};
