#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <string>

class NotificationNode : public cocos2d::CCLayer
{
public:
    using Callback = std::function<void()>;

    static NotificationNode *create(const std::string &title, const std::string &message, Callback viewCallback = nullptr);
    bool init(const std::string &title, const std::string &message, Callback viewCallback = nullptr);

    void setViewCallback(Callback viewCallback) { m_viewCallback = viewCallback; }
    void removeFromParent() { this->removeFromParentAndCleanup(true); }

private:
    void onViewButton(CCObject *);
    Callback m_viewCallback = nullptr;
};