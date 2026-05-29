#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>

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
    CCNode* m_statsNode = nullptr;
    CCLabelBMFont* m_acceptanceLabel = nullptr;
    CCLabelBMFont* m_activeThumbnailsLabel = nullptr;
    CCNode* m_uploadStatsNode = nullptr;
    CCLabelBMFont* m_acceptedUploadsLabel = nullptr;
    CCLabelBMFont* m_acceptedLevelsLabel = nullptr;
    CCLabelBMFont* m_pendingUploadsLabel = nullptr;
    void fetchDashboard();
};
