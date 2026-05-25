#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>

using namespace geode::prelude;

class BetterThumbnailLayer : public CCLayer {
public:
    async::TaskHolder<web::WebResponse> m_listener;
    static BetterThumbnailLayer* create();
    static CCScene* scene();
    bool init() override;
    void onBackButton(CCObject*);
    void keyBackClicked() override;

private:
    void onMyThumbnail(CCObject*);
    void onDashboard(CCObject*);
    void onPending(CCObject*);
    void onManage(CCObject*);
    void onInfoButton(CCObject*);
    void fetchNotifications();

    CCSprite* myThumbSprite = nullptr;
    CCSprite* dashboardSprite = nullptr;
    CCSprite* pendingSprite = nullptr;
    CCSprite* manageSprite = nullptr;

    CCCounterLabel* coinLabel = nullptr;
    CCMenu* m_bottomLeftMenu = nullptr;
    CCMenu* m_menuButtons = nullptr;

    int m_activeThumbnailCount = 0;
    int m_uploadThumbnailCount = 0;
    int m_acceptedUploadThumbnailCount = 0;

    int m_lastNotificationId = -1;
};
