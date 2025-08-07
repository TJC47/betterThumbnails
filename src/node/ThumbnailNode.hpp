#pragma once
#include <cocos2d.h>
#include <cocos-ext.h>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ThumbnailNode : public cocos2d::CCNode
{
public:
    EventListener<web::WebTask> m_listener;
    int m_thumbId = 0;
    static ThumbnailNode *create(const cocos2d::CCSize &size, int id, int user_id, const std::string &username, int level_id, bool accepted, const std::string &upload_time, bool replacement);
    bool init(const cocos2d::CCSize &size, int id, int user_id, const std::string &username, int level_id, bool accepted, const std::string &upload_time, bool replacement);
    void onViewButton(CCObject*);
};
