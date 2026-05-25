#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <cue/ListNode.hpp>

#include <string>
#include <vector>

using namespace geode::prelude;

struct ManageUserEntry {
    int id = 0;
    int accountId = -1;
    std::string username;
    std::string role;
    int accepted = 0;
    int activeThumbnails = 0;
    int pending = 0;
    int rejected = 0;
    int totalUploads = 0;
};

class ManageUserLayer : public CCLayer {
public:
    async::TaskHolder<web::WebResponse> m_listener;

    static ManageUserLayer* create();

    bool init() override;
    void keyBackClicked() override;

private:
    static constexpr int ITEMS_PER_PAGE = 10;

    int m_currentPage = 1;
    int m_totalPages = 1;
    int m_totalUsers = 0;

    cue::ListNode* m_listNode = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    CCLabelBMFont* m_infoLabel = nullptr;
    CCMenu* m_navMenu = nullptr;
    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;

    std::vector<ManageUserEntry> m_users;

    void fetchPage(int page);
    void populateList();
    void onPrevPage(CCObject*);
    void onNextPage(CCObject*);
    void onOpenProfile(CCObject*);
    CCSprite* spriteForRole(const std::string& role);
};