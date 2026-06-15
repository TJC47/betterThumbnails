#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <cue/LoadingCircle.hpp>
#include <cue/ListNode.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

struct MyThumbnailEntry {
    int id = 0;
    int level_id = 0;
    std::string accepted_time;
    std::string note_data;
    std::string upload_time;
};

class MyThumbnailsLayer : public CCLayer {
public:
    async::TaskHolder<web::WebResponse> m_listener;
    static MyThumbnailsLayer* create();
    bool init() override;
    void keyBackClicked() override;

public:
    enum class UploadMode {
        Active,
        Pending,
        Rejected,
    };

private:
    static std::string endpointForMode(UploadMode mode);
    static constexpr int ITEMS_PER_PAGE = 12;
    int m_currentPage = 1;
    int m_apiPerPage = 0;
    int m_apiTotal = 0;
    bool m_serverPaging = true;
    UploadMode m_mode = UploadMode::Active;

    std::vector<MyThumbnailEntry> m_uploads;

    cue::ListNode* m_listNode = nullptr;
    cue::LoadingCircle* m_loadingCircle = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;
    CCLabelBMFont* m_infoLabel = nullptr;
    CCMenu* m_navMenu = nullptr;
    CCMenu* m_tabMenu = nullptr;
    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;
    CCMenuItemSpriteExtra* m_reloadBtn = nullptr;
    TabButton* m_activeTab = nullptr;
    TabButton* m_pendingTab = nullptr;
    TabButton* m_rejectedTab = nullptr;

    void fetchPage(int page);
    void updateUI();
    void onPrevPage(CCObject*);
    void onNextPage(CCObject*);
    void onReload(CCObject*);
    void onSelectActive(CCObject*);
    void onSelectPending(CCObject*);
    void onSelectRejected(CCObject*);
};
