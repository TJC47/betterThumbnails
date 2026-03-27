#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <cue/ListNode.hpp>
#include <vector>

using namespace geode::prelude;

class NotificationMenuPopup : public Popup {
public:
    static NotificationMenuPopup* create();
    bool init() override;

    void setNotifications(const std::vector<std::pair<std::string, std::string>>& notifications, int userId);

private:
    void populateList();
    void onClearAll(CCObject*);

    cue::ListNode* m_listNode = nullptr;
    std::vector<std::pair<std::string, std::string>> m_notifications;
    int m_userId = 0;
    async::TaskHolder<web::WebResponse> m_listener;
};