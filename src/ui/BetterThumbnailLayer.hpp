#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class BetterThumbnailLayer : public CCLayer
{

public:
    EventListener<web::WebTask> m_listener;
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

    CCSprite *myThumbSprite = nullptr;
    CCSprite *recentSprite = nullptr;
    CCSprite *pendingSprite = nullptr;
    CCSprite *manageSprite = nullptr;

};
