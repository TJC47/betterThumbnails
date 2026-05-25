#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <cue/LoadingCircle.hpp>

using namespace geode::prelude;

class ThumbnailInfoLayer : public CCLayer {
public:
    static CCScene* scene(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note);
    static ThumbnailInfoLayer* create(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note);

    bool init(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note);

    void keyBackClicked() override;

    // Actions
    void onAccept(CCObject*);
    void onReject(CCObject*);
    void onPlayLevelButton(CCObject*);
    void onShowOriginal(CCObject*);
    void fetchLevel();

private:
    // Stored for API calls
    int m_id = 0;
    int m_userId = 0;
    std::string m_username;
    int m_levelId = 0;
    bool m_acceptedFlag = false;
    std::string m_uploadTime;
    bool m_replacementFlag = false;
    std::string m_submissionNote;

    async::TaskHolder<web::WebResponse> m_listener;
    CCMenu* m_bottomMenu = nullptr;
    LazySprite* m_thumbReplacement = nullptr;
    LazySprite* m_thumbOriginal = nullptr;
    cue::LoadingCircle* m_thumbSpinner = nullptr;
    CCMenuItemSpriteExtra* m_showOriginalBtn = nullptr;
    bool m_showingOriginal = false;
    bool m_originalLoaded = false;
    CCLabelBMFont* m_thumbLabel = nullptr;
    LevelCell* m_levelCell;
    int m_levelFetchRetries = 0;
    GJGameLevel* m_level = nullptr;
};
