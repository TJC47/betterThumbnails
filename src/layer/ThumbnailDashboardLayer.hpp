#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/ui/ProgressBar.hpp>

using namespace geode::prelude;

struct UserStats {
    int uploadCount = 0;
    int acceptedUploadCount = 0;
    int levelCount = 0;
    int activeThumbnailCount = 0;
    int pendingUploadCount = 0;
    std::string username = "unknown";
};

struct GlobalStats {
    int accepted_uploads_total = 0;
    int current_pending_uploads = 0;
    int pending_uploads_total = 0;
    long long storage = 0;
    int thumbnails = 0;
    int total_levels = 0;
    int uploads_total = 0;
    int users_per_month = 0;
    int users_total = 0;
};

class ThumbnailDashboardLayer : public CCLayer {
public:
    async::TaskHolder<web::WebResponse> m_listener;
    async::TaskHolder<web::WebResponse> m_statsListener;
    
    static ThumbnailDashboardLayer* create();
    static CCScene* scene();
    bool init() override;
    void keyBackClicked() override;

private:
    CCLabelBMFont* m_title = nullptr;
    CCMenuItemToggler* m_statsToggler = nullptr;
    bool m_showingGlobalStats = false;
    UserStats m_userStats;
    GlobalStats m_globalStats;

    // acceptance stats
    CCNode* m_acceptanceStatsNode = nullptr;
    CCCounterLabel* m_acceptanceLabel = nullptr;
    CCLabelBMFont* m_acceptanceTitle = nullptr;

    // active thumbnail stats
    CCNode* m_activeThumbnailsNode = nullptr;
    CCCounterLabel* m_activeThumbnailsLabel = nullptr;
    CCLabelBMFont* m_activeThumbnailsTitle = nullptr;

    // acceptance progress
    geode::ProgressBar* m_progressBar = nullptr;

    // upload stats
    CCNode* m_uploadStatsNode = nullptr;
    CCCounterLabel* m_uploadLabel = nullptr;
    CCLabelBMFont* m_uploadTitle = nullptr;

    // accepted upload stats
    CCNode* m_acceptanceUploadsNode = nullptr;
    CCCounterLabel* m_acceptanceUploadsLabel = nullptr;
    CCLabelBMFont* m_acceptanceUploadsTitle = nullptr;

    // unique levels
    CCNode* m_uniqueLevelsNode = nullptr;
    CCCounterLabel* m_uniqueLevelsLabel = nullptr;
    CCLabelBMFont* m_uniqueLevelsTitle = nullptr;

    // rejected upload stats
    CCNode* m_rejectedUploadsNode = nullptr;
    CCCounterLabel* m_rejectedUploadsLabel = nullptr;
    CCLabelBMFont* m_rejectedUploadsTitle = nullptr;

    // pending upload stats
    CCNode* m_pendingUploadsNode = nullptr;
    CCCounterLabel* m_pendingUploadsLabel = nullptr;
    CCLabelBMFont* m_pendingUploadsTitle = nullptr;

    // replaced thumbnail stats
    CCNode* m_replacedThumbnailsNode = nullptr;
    CCCounterLabel* m_replacedThumbnailsLabel = nullptr;
    CCLabelBMFont* m_replacedThumbnailsTitle = nullptr;

    // users total
    CCNode* m_usersTotalNode = nullptr;
    CCCounterLabel* m_usersTotalLabel = nullptr;
    CCLabelBMFont* m_usersTotalTitle = nullptr;

    // users per month
    CCNode* m_usersPerMonthNode = nullptr;
    CCCounterLabel* m_usersPerMonthLabel = nullptr;
    CCLabelBMFont* m_usersPerMonthTitle = nullptr;

    // storage
    CCNode* m_storageNode = nullptr;
    CCLabelBMFont* m_storageLabel = nullptr;
    CCLabelBMFont* m_storageTitle = nullptr;

    void fetchDashboard();
    void fetchGlobalStats();
    void onToggleStats(CCObject* sender);
    void updateUI();
};
