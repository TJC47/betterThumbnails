#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <cue/DropdownNode.hpp>
#include <cue/LoadingCircle.hpp>
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
    bool banned = false;
    std::string banReason;
    std::string bannedBy;
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
    cue::LoadingCircle* m_loadingCircle = nullptr;
    cue::DropdownNode* m_roleDropdown = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    CCLabelBMFont* m_infoLabel = nullptr;
    CCMenu* m_navMenu = nullptr;
    CCMenu* m_filterMenu = nullptr;
    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;
    CCMenuItemToggler* m_bannedToggle = nullptr;
    CCLabelBMFont* m_emptyLabel = nullptr;

    bool m_showBannedOnly = false;
    std::string m_selectedRole = "Any";

    std::vector<ManageUserEntry> m_users;

    void fetchPage(int page);
    void populateList();
    void banUser(int id);
    void unbanUser(int id);
    void onToggleBanned(CCObject*);
    void onPrevPage(CCObject*);
    void onNextPage(CCObject*);
    CCSprite* spriteForRole(const std::string& role);
};