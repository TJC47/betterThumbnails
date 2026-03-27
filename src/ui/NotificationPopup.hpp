#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class NotificationPopup : public Popup {
public:
    static NotificationPopup *create(
        const std::vector<std::pair<std::string, std::string>> &notifications,
        geode::Function<void()> onClear);

protected:
    bool init(const std::vector<std::pair<std::string, std::string>> &notifications,
              geode::Function<void()> onClear);

private:
    std::vector<std::pair<std::string, std::string>> m_notifications;
    geode::Function<void()> m_onClear;

    void onClear(CCObject *);
};