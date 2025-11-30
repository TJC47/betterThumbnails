#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>

using namespace geode::prelude;

struct PendingThumbEntry {
    int id = 0;
    int user_id = 0;
    std::string username;
    int level_id = 0;
    bool accepted = false;
    std::string upload_time;
    bool replacement = false;
};

class PendingThumbnailLayer : public CCLayer {
public:
    EventListener<web::WebTask> m_listener;
    static PendingThumbnailLayer* create();
    static CCScene *scene();
    bool init() override;
    void onBackButton(CCObject *);
    void keyBackClicked() override;

private:
    static constexpr int ITEMS_PER_PAGE = 20;
    int m_currentPage = 1;
    std::vector<PendingThumbEntry> m_pendingItems;

    ScrollLayer* m_scrollLayer = nullptr;
    cocos2d::CCLayer* m_contentLayer = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    CCLabelBMFont* m_infoLabel = nullptr;
    CCMenu* m_navMenu = nullptr;
    CCMenu* m_filterMenu = nullptr;
    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;

    CCMenuItemToggler* m_allFilterBtn = nullptr;
    CCMenuItemToggler* m_replacementFilterBtn = nullptr;
    bool m_showReplacementOnly = false;

    void refreshPage();
    void onPrevPage(CCObject*);
    void onNextPage(CCObject*);
    void onFilterAll(CCObject*);
    void onFilterReplacement(CCObject*);
};

