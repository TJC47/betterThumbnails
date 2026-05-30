#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <vector>
#include <unordered_set>
#include "../popup/NotificationMenuPopup.hpp"

using namespace geode::prelude;

class NotificationOverlay : public CCLayer {
public:
    static NotificationOverlay* create();
    static NotificationOverlay* get();

    bool init() override;
    void onEnter() override;
    void onExit() override;

    void pollNotifications(float dt);
    void showNotificationList();
    void showNotificationList(std::vector<NotificationMenuPopup::NotificationEntry> notifications);

private:
    void fetchNotifications();
    void processNotificationResponse(web::WebResponse res);
    void loadReadNotificationIds();
    void saveReadNotificationIds();

    long long m_lastNotificationTimestamp = 0;
    std::unordered_set<int> m_readNotificationIds;
    bool m_isNotificationFetchInProgress = false;
    async::TaskHolder<web::WebResponse> m_notificationListener;
    std::vector<NotificationMenuPopup::NotificationEntry> m_notifications;
};
