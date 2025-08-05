#pragma once
#include <cocos2d.h>
#include <cocos-ext.h>
#include <Geode/Geode.hpp>

class ThumbnailNode : public cocos2d::CCNode
{
public:
    static ThumbnailNode *create(const cocos2d::CCSize &size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);
    bool init(const cocos2d::CCSize &size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);
};
