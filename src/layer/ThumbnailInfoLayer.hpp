#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <cue/LoadingCircle.hpp>
#include <set>
#include <Geode/ui/Button.hpp>

using namespace geode::prelude;

class ThumbnailInfoLayer : public CCLayer {
public:
    static CCScene* scene(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id);
    static ThumbnailInfoLayer* create(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id);

    bool init(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id);

    void keyBackClicked() override;

    // Actions
    void onAccept(CCObject*);
    void onReject(CCObject*);
    void onPlayLevelButton(CCObject*);
    void onShowOriginal(CCObject*);
    void onInfoToggle(CCObject*);
    bool ccTouchBegan(CCTouch* pTouch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* pTouch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* pTouch, CCEvent* event) override;
    void scrollWheel(float y, float x) override;
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
    LazySprite* m_thumbReplacement = nullptr;
    LazySprite* m_thumbOriginal = nullptr;
    cue::LoadingCircle* m_thumbSpinner = nullptr;
    CCMenuItemToggler* m_toggleButton = nullptr;
    CCMenuItemToggler* m_infoToggle = nullptr;
    MDTextArea* m_infoTextArea = nullptr;
    NineSlice* m_thumbBg = nullptr;
    CCNode* m_creatorNode = nullptr;
    CCNode* m_replacementNode = nullptr;
    bool m_showingOriginal = false;
    bool m_originalLoaded = false;
    bool m_infoVisible = false;
    std::set<CCTouch*> m_touches;
    CCPoint m_touchMidPoint = {0, 0};
    float m_initialScale = 1.f;
    float m_initialDistance = 0.f;
    bool m_wasZooming = false;
    int m_levelFetchRetries = 0;
    int m_accountId = 0;
    GJGameLevel* m_level = nullptr;
};
