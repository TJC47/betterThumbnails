#include "NotificationPopup.hpp"

using namespace geode::prelude;

NotificationPopup *NotificationPopup::create(
    const std::vector<std::pair<std::string, std::string>> &notifications,
    geode::Function<void()> onClear) {
    auto ret = new NotificationPopup;
    if (ret && ret->init(notifications, std::move(onClear))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NotificationPopup::init(
    const std::vector<std::pair<std::string, std::string>> &notifications,
    geode::Function<void()> onClear) {
    if (!Popup::init(340.f, 280.f))
        return false;

    m_notifications = notifications;
    m_onClear = std::move(onClear);

    setTitle("Notifications");

    std::string body;
    for (auto &entry : m_notifications) {
        body += fmt::format("{}: {}\n", entry.first, entry.second);
    }
    if (body.empty()) {
        body = "No new notifications.";
    }

    auto bodyLabel = CCLabelBMFont::create(body.c_str(), "bigFont.fnt");
    bodyLabel->setAnchorPoint({0.f, 1.f});
    bodyLabel->setScale(0.45f);
    bodyLabel->setPosition({20.f, m_mainLayer->getContentSize().height - 30.f});
    m_mainLayer->addChild(bodyLabel);


    auto clearSpr = ButtonSprite::create("Clear All", "goldFont.fnt", "GJ_button_03.png", 1.f);
    auto clearBtn = CCMenuItemSpriteExtra::create(clearSpr, this, menu_selector(NotificationPopup::onClear));
    clearBtn->setPosition({m_mainLayer->getContentSize().width / 2.f - 70.f, 30.f});
    m_buttonMenu->addChild(clearBtn);

    return true;
}

void NotificationPopup::onClear(CCObject *) {
    if (m_onClear) {
        m_onClear();
    }
    this->removeFromParentAndCleanup(true);
}
