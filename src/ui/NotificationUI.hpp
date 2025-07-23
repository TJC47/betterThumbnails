#pragma once
#include <Geode/Geode.hpp>
#include <string>

class NotificationUI : public cocos2d::CCLayer
{
public:
    static NotificationUI *create(const std::string &title, const std::string &message);
    bool init(const std::string &title, const std::string &message);
    void removeFromParent() { this->removeFromParentAndCleanup(true); }
};