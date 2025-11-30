#pragma once
#include <cocos2d.h>
#include <cocos-ext.h>
#include <Geode/Geode.hpp>
#include <string>

using namespace geode::prelude;

class ThumbnailNode : public cocos2d::CCLayer
{
public:
    EventListener<web::WebTask> m_listener;
    // Stored data from API for opening the info layer
    int m_thumbId = 0;
    int m_userId = 0;
    std::string m_username;
    int m_levelId = 0;
    bool m_accepted = false;
    std::string m_uploadTime;
    bool m_replacement = false;
    static ThumbnailNode *create(const cocos2d::CCSize &size, int id, int user_id, const std::string &username, int level_id, bool accepted, const std::string &upload_time, bool replacement);
    bool init(const cocos2d::CCSize &size, int id, int user_id, const std::string &username, int level_id, bool accepted, const std::string &upload_time, bool replacement);
    void onViewButton(CCObject *);
};
