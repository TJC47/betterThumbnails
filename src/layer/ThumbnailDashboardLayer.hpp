#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/ui/ProgressBar.hpp>

using namespace geode::prelude;

class ThumbnailDashboardLayer : public CCLayer {
public:
    async::TaskHolder<web::WebResponse> m_listener;
    static ThumbnailDashboardLayer* create();
    static CCScene* scene();
    bool init() override;
    void keyBackClicked() override;

private:
    CCLabelBMFont* m_title = nullptr;

    // acceptance stats
    CCNode* m_acceptanceStatsNode = nullptr;
    CCCounterLabel* m_acceptanceLabel = nullptr;

    // active thumbnail stats
    CCNode* m_activeThumbnailsNode = nullptr;
    CCCounterLabel* m_activeThumbnailsLabel = nullptr;

    // acceptance progress
    geode::ProgressBar* m_progressBar = nullptr;

    // upload stats    CCNode* m_uploadStatsNode = nullptr;
    CCCounterLabel* m_uploadLabel = nullptr;
    CCNode* m_uploadStatsNode = nullptr;

    // accepted upload stats
    CCNode* m_acceptanceUploadsNode = nullptr;
    CCCounterLabel* m_acceptanceUploadsLabel = nullptr;

    // unique levels
    CCNode* m_uniqueLevelsNode = nullptr;
    CCCounterLabel* m_uniqueLevelsLabel = nullptr;

    // rejected upload stats
    CCNode* m_rejectedUploadsNode = nullptr;
    CCCounterLabel* m_rejectedUploadsLabel = nullptr;

    // pending upload stats
    CCNode* m_pendingUploadsNode = nullptr;
    CCCounterLabel* m_pendingUploadsLabel = nullptr;

    // replaced thumbnail stats
    CCNode* m_replacedThumbnailsNode = nullptr;
    CCCounterLabel* m_replacedThumbnailsLabel = nullptr;

    void fetchDashboard();
};
