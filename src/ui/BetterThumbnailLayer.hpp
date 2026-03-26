#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>

using namespace geode::prelude;

class BetterThumbnailLayer : public CCLayer
{

public:
    async::TaskHolder<web::WebResponse> m_listener;
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
    void fetchNotifications();

    CCSprite *myThumbSprite = nullptr;
    CCSprite *recentSprite = nullptr;
    CCSprite *pendingSprite = nullptr;
    CCSprite *manageSprite = nullptr;

    int m_lastNotificationId = -1;

};
