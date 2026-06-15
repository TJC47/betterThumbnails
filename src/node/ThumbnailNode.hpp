#pragma once
#include <cocos-ext.h>
#include <cocos2d.h>

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <cue/LoadingCircle.hpp>
#include <string>

using namespace geode::prelude;

class PendingThumbnailLayer;

class ThumbnailNode : public cocos2d::CCLayer {
public:
    async::TaskHolder<web::WebResponse> m_listener;
    LazySprite* m_backdrop = nullptr;
    // Stored data from API for opening the info layer
    int m_thumbId = 0;
    int m_userId = 0;
    std::string m_username;
    int m_levelId = 0;
    bool m_accepted = false;
    std::string m_uploadTime;
    bool m_replacement = false;
    std::string m_noteData;
    int m_accountId = 0;
    std::string m_acceptedTime;
    bool m_showViewButton = true;
    std::string m_thumbnailUrl;
    PendingThumbnailLayer* m_pendingLayer = nullptr;
    enum class Mode {
        MyThumbnail,
        PendingThumbnail,
    } m_mode = Mode::MyThumbnail;
    std::string m_levelName;
    GJGameLevel* m_level = nullptr;
    CCLabelBMFont* m_levelInfoLabel = nullptr;
    CCNode* m_creatorNode = nullptr;
    CCNode* m_replacementNode = nullptr;
    CCNode* m_playBtn = nullptr;
    bool m_openOnLevelLoaded = false;
    cue::LoadingCircle* m_levelLoadingCircle = nullptr;
    int m_levelFetchRetries = 0;
    static ThumbnailNode* create(const cocos2d::CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& note_data, int account_id, const std::string& accepted_time = "", std::string thumbnailUrl = "", Mode mode = Mode::MyThumbnail);
    bool init(const cocos2d::CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& note_data, int account_id, const std::string& accepted_time = "", std::string thumbnailUrl = "", Mode mode = Mode::MyThumbnail);
    void fetchLevel();
    void updateBadges();
    void setPendingLayer(PendingThumbnailLayer* pendingLayer);
};
