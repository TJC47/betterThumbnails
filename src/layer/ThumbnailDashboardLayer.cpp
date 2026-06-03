#include "ThumbnailDashboardLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/ProgressBar.hpp>
#include <cue/LoadingCircle.hpp>

using namespace geode::prelude;

CCScene* ThumbnailDashboardLayer::scene() {
    auto scene = CCScene::create();
    scene->addChild(ThumbnailDashboardLayer::create());
    return scene;
}

ThumbnailDashboardLayer* ThumbnailDashboardLayer::create() {
    auto ret = new ThumbnailDashboardLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ThumbnailDashboardLayer::init() {
    if (!CCLayer::init()) {
        return false;
    }

    addSideArt(this, SideArt::All, false);

    auto bg = createLayerBG();
    if (bg != nullptr) {
        this->addChild(bg, -1);
    }

    addBackButton(this, BackButtonStyle::Green);

    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    m_title = CCLabelBMFont::create("-", "goldFont.fnt");
    m_title->setAnchorPoint({0.5f, 1.f});
    m_title->setPosition({screenSize.width / 2.f, screenSize.height - 15.f});
    m_title->setScale(0.8f);
    this->addChildAtPosition(m_title, Anchor::Top, {0.f, -20.f}, false);

    // acceptance stats
    m_acceptanceStatsNode = CCNode::create();
    m_acceptanceStatsNode->setAnchorPoint({0.5f, 0.5f});
    m_acceptanceStatsNode->setContentSize({180.f, 60.f});
    m_acceptanceStatsNode->setPosition({screenSize.width / 2.f - 100.f, screenSize.height - 80.f});
    this->addChild(m_acceptanceStatsNode);

    auto acceptanceStatsBg = NineSlice::create("square02_001.png");
    acceptanceStatsBg->setContentSize(m_acceptanceStatsNode->getContentSize());
    acceptanceStatsBg->setOpacity(150);
    acceptanceStatsBg->setPosition(m_acceptanceStatsNode->getContentSize() / 2.f);
    m_acceptanceStatsNode->addChild(acceptanceStatsBg);

    auto acceptanceStatsTitle = CCLabelBMFont::create("Acceptance Rate", "goldFont.fnt");
    acceptanceStatsTitle->limitLabelWidth(m_acceptanceStatsNode->getContentWidth(), 0.5f, 0.2f);
    acceptanceStatsTitle->setPosition({m_acceptanceStatsNode->getContentSize().width / 2.f, m_acceptanceStatsNode->getContentSize().height - 10.f});
    m_acceptanceStatsNode->addChild(acceptanceStatsTitle);

    m_acceptanceLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_acceptanceLabel->limitLabelWidth(m_acceptanceStatsNode->getContentWidth(), 0.6f, 0.2f);
    m_acceptanceLabel->setPosition({m_acceptanceStatsNode->getContentSize().width / 2.f, m_acceptanceStatsNode->getContentSize().height / 2.f - 5.f});
    m_acceptanceStatsNode->addChild(m_acceptanceLabel);

    // active thumbnails
    m_activeThumbnailsNode = CCNode::create();
    m_activeThumbnailsNode->setContentSize({180.f, 60.f});
    m_activeThumbnailsNode->setAnchorPoint({0.5f, 0.5f});
    m_activeThumbnailsNode->setPosition({screenSize.width / 2.f + 100.f, screenSize.height - 80.f});
    this->addChild(m_activeThumbnailsNode);

    auto activeThumbnailsBg = NineSlice::create("square02_001.png");
    activeThumbnailsBg->setContentSize(m_activeThumbnailsNode->getContentSize());
    activeThumbnailsBg->setOpacity(150);
    activeThumbnailsBg->setPosition(m_activeThumbnailsNode->getContentSize() / 2.f);
    m_activeThumbnailsNode->addChild(activeThumbnailsBg);

    auto activeThumbnailsTitle = CCLabelBMFont::create("Active Thumbnails", "goldFont.fnt");
    activeThumbnailsTitle->limitLabelWidth(m_activeThumbnailsNode->getContentWidth(), 0.5f, 0.2f);
    activeThumbnailsTitle->setPosition({m_activeThumbnailsNode->getContentSize().width / 2.f, m_activeThumbnailsNode->getContentSize().height - 10.f});
    m_activeThumbnailsNode->addChild(activeThumbnailsTitle);

    m_activeThumbnailsLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_activeThumbnailsLabel->limitLabelWidth(m_activeThumbnailsNode->getContentWidth(), 0.6f, 0.2f);
    m_activeThumbnailsLabel->setPosition({m_activeThumbnailsNode->getContentSize().width / 2.f, m_activeThumbnailsNode->getContentSize().height / 2.f - 5.f});
    m_activeThumbnailsNode->addChild(m_activeThumbnailsLabel);

    // Acceptance rate progress bar
    m_progressBar = ProgressBar::create(ProgressBarStyle::Solid);
    m_progressBar->setAnchorPoint({0.5f, 0.5f});
    m_progressBar->setPosition({screenSize.width / 2.f, screenSize.height - 130.f});
    m_progressBar->showProgressLabel(true);
    m_progressBar->updateProgress(0.f);
    m_progressBar->setFillColor({86, 211, 142});
    this->addChild(m_progressBar);

    // uploads
    m_uploadStatsNode = CCNode::create();
    m_uploadStatsNode->setContentSize({140.f, 60.f});
    m_uploadStatsNode->setAnchorPoint({0.5f, 0.5f});
    m_uploadStatsNode->setPosition({screenSize.width / 2.f - 150.f, screenSize.height - 180.f});
    this->addChild(m_uploadStatsNode);

    auto uploadStatsBg = NineSlice::create("square02_001.png");
    uploadStatsBg->setContentSize(m_uploadStatsNode->getContentSize());
    uploadStatsBg->setOpacity(150);
    uploadStatsBg->setPosition(m_uploadStatsNode->getContentSize() / 2.f);
    m_uploadStatsNode->addChild(uploadStatsBg);

    auto uploadStatsTitle = CCLabelBMFont::create("Uploads", "goldFont.fnt");
    uploadStatsTitle->limitLabelWidth(m_uploadStatsNode->getContentWidth(), 0.5f, 0.2f);
    uploadStatsTitle->setPosition({m_uploadStatsNode->getContentSize().width / 2.f, m_uploadStatsNode->getContentSize().height - 10.f});
    m_uploadStatsNode->addChild(uploadStatsTitle);

    m_uploadLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_uploadLabel->limitLabelWidth(m_uploadStatsNode->getContentWidth(), 0.6f, 0.2f);
    m_uploadLabel->setPosition({m_uploadStatsNode->getContentSize().width / 2.f, m_uploadStatsNode->getContentSize().height / 2.f - 5.f});
    m_uploadStatsNode->addChild(m_uploadLabel);

    // accepted
    m_acceptanceUploadsNode = CCNode::create();
    m_acceptanceUploadsNode->setContentSize({140.f, 60.f});
    m_acceptanceUploadsNode->setAnchorPoint({0.5f, 0.5f});
    m_acceptanceUploadsNode->setPosition({screenSize.width / 2.f, screenSize.height - 180.f});
    this->addChild(m_acceptanceUploadsNode);

    auto acceptanceStatsBg2 = NineSlice::create("square02_001.png");
    acceptanceStatsBg2->setContentSize(m_acceptanceUploadsNode->getContentSize());
    acceptanceStatsBg2->setOpacity(150);
    acceptanceStatsBg2->setPosition(m_acceptanceUploadsNode->getContentSize() / 2.f);
    m_acceptanceUploadsNode->addChild(acceptanceStatsBg2);

    auto acceptanceStatsTitle2 = CCLabelBMFont::create("Accepted", "goldFont.fnt");
    acceptanceStatsTitle2->limitLabelWidth(m_acceptanceUploadsNode->getContentWidth(), 0.5f, 0.2f);
    acceptanceStatsTitle2->setPosition({m_acceptanceUploadsNode->getContentSize().width / 2.f, m_acceptanceUploadsNode->getContentSize().height - 10.f});
    m_acceptanceUploadsNode->addChild(acceptanceStatsTitle2);

    m_acceptanceUploadsLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_acceptanceUploadsLabel->limitLabelWidth(m_acceptanceUploadsNode->getContentWidth(), 0.6f, 0.2f);
    m_acceptanceUploadsLabel->setPosition({m_acceptanceUploadsNode->getContentSize().width / 2.f, m_acceptanceUploadsNode->getContentSize().height / 2.f - 5.f});
    m_acceptanceUploadsNode->addChild(m_acceptanceUploadsLabel);

    // unique levels
    m_uniqueLevelsNode = CCNode::create();
    m_uniqueLevelsNode->setContentSize({140.f, 60.f});
    m_uniqueLevelsNode->setAnchorPoint({0.5f, 0.5f});
    m_uniqueLevelsNode->setPosition({screenSize.width / 2.f + 150.f, screenSize.height - 180.f});
    this->addChild(m_uniqueLevelsNode);

    auto uniqueLevelsBg = NineSlice::create("square02_001.png");
    uniqueLevelsBg->setContentSize(m_uniqueLevelsNode->getContentSize());
    uniqueLevelsBg->setOpacity(150);
    uniqueLevelsBg->setPosition(m_uniqueLevelsNode->getContentSize() / 2.f);
    m_uniqueLevelsNode->addChild(uniqueLevelsBg);

    auto uniqueLevelsTitle = CCLabelBMFont::create("Unique Levels", "goldFont.fnt");
    uniqueLevelsTitle->limitLabelWidth(m_uniqueLevelsNode->getContentWidth(), 0.5f, 0.2f);
    uniqueLevelsTitle->setPosition({m_uniqueLevelsNode->getContentSize().width / 2.f, m_uniqueLevelsNode->getContentSize().height - 10.f});
    m_uniqueLevelsNode->addChild(uniqueLevelsTitle);

    m_uniqueLevelsLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_uniqueLevelsLabel->limitLabelWidth(m_uniqueLevelsNode->getContentWidth(), 0.6f, 0.2f);
    m_uniqueLevelsLabel->setPosition({m_uniqueLevelsNode->getContentSize().width / 2.f, m_uniqueLevelsNode->getContentSize().height / 2.f - 5.f});
    m_uniqueLevelsNode->addChild(m_uniqueLevelsLabel);

    // rejected uploads (upload_count - accepted_upload_count)
    m_rejectedUploadsNode = CCNode::create();
    m_rejectedUploadsNode->setContentSize({140.f, 60.f});
    m_rejectedUploadsNode->setAnchorPoint({0.5f, 0.5f});
    m_rejectedUploadsNode->setPosition({screenSize.width / 2.f - 150.f, screenSize.height - 250.f});
    this->addChild(m_rejectedUploadsNode);

    auto rejectedUploadsBg = NineSlice::create("square02_001.png");
    rejectedUploadsBg->setContentSize(m_rejectedUploadsNode->getContentSize());
    rejectedUploadsBg->setOpacity(150);
    rejectedUploadsBg->setPosition(m_rejectedUploadsNode->getContentSize() / 2.f);
    m_rejectedUploadsNode->addChild(rejectedUploadsBg);

    auto rejectedUploadsTitle = CCLabelBMFont::create("Rejected Uploads", "goldFont.fnt");
    rejectedUploadsTitle->limitLabelWidth(m_rejectedUploadsNode->getContentWidth(), 0.5f, 0.2f);
    rejectedUploadsTitle->setPosition({m_rejectedUploadsNode->getContentSize().width / 2.f, m_rejectedUploadsNode->getContentSize().height - 10.f});
    m_rejectedUploadsNode->addChild(rejectedUploadsTitle);

    m_rejectedUploadsLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_rejectedUploadsLabel->limitLabelWidth(m_rejectedUploadsNode->getContentWidth(), 0.6f, 0.2f);
    m_rejectedUploadsLabel->setPosition({m_rejectedUploadsNode->getContentSize().width / 2.f, m_rejectedUploadsNode->getContentSize().height / 2.f - 5.f});
    m_rejectedUploadsNode->addChild(m_rejectedUploadsLabel);

    // pending uploads
    m_pendingUploadsNode = CCNode::create();
    m_pendingUploadsNode->setContentSize({140.f, 60.f});
    m_pendingUploadsNode->setAnchorPoint({0.5f, 0.5f});
    m_pendingUploadsNode->setPosition({screenSize.width / 2.f, screenSize.height - 250.f});
    this->addChild(m_pendingUploadsNode);

    auto pendingUploadsBg = NineSlice::create("square02_001.png");
    pendingUploadsBg->setContentSize(m_pendingUploadsNode->getContentSize());
    pendingUploadsBg->setOpacity(150);
    pendingUploadsBg->setPosition(m_pendingUploadsNode->getContentSize() / 2.f);
    m_pendingUploadsNode->addChild(pendingUploadsBg);

    auto pendingUploadsTitle = CCLabelBMFont::create("Pending Uploads", "goldFont.fnt");
    pendingUploadsTitle->limitLabelWidth(m_pendingUploadsNode->getContentWidth(), 0.5f, 0.2f);
    pendingUploadsTitle->setPosition({m_pendingUploadsNode->getContentSize().width / 2.f, m_pendingUploadsNode->getContentSize().height - 10.f});
    m_pendingUploadsNode->addChild(pendingUploadsTitle);

    m_pendingUploadsLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_pendingUploadsLabel->limitLabelWidth(m_pendingUploadsNode->getContentWidth(), 0.6f, 0.2f);
    m_pendingUploadsLabel->setPosition({m_pendingUploadsNode->getContentSize().width / 2.f, m_pendingUploadsNode->getContentSize().height / 2.f - 5.f});
    m_pendingUploadsNode->addChild(m_pendingUploadsLabel);

    // replaced thumbnails (accepted_upload_count - accepted_level_count)
    m_replacedThumbnailsNode = CCNode::create();
    m_replacedThumbnailsNode->setContentSize({140.f, 60.f});
    m_replacedThumbnailsNode->setAnchorPoint({0.5f, 0.5f});
    m_replacedThumbnailsNode->setPosition({screenSize.width / 2.f + 150.f, screenSize.height - 250.f});
    this->addChild(m_replacedThumbnailsNode);

    auto replacedThumbnailsBg = NineSlice::create("square02_001.png");
    replacedThumbnailsBg->setContentSize(m_replacedThumbnailsNode->getContentSize());
    replacedThumbnailsBg->setOpacity(150);
    replacedThumbnailsBg->setPosition(m_replacedThumbnailsNode->getContentSize() / 2.f);
    m_replacedThumbnailsNode->addChild(replacedThumbnailsBg);

    auto replacedThumbnailsTitle = CCLabelBMFont::create("Replaced Thumbnails", "goldFont.fnt");
    replacedThumbnailsTitle->limitLabelWidth(m_replacedThumbnailsNode->getContentWidth(), 0.5f, 0.2f);
    replacedThumbnailsTitle->setPosition({m_replacedThumbnailsNode->getContentSize().width / 2.f, m_replacedThumbnailsNode->getContentSize().height - 10.f});
    m_replacedThumbnailsNode->addChild(replacedThumbnailsTitle);

    m_replacedThumbnailsLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_replacedThumbnailsLabel->limitLabelWidth(m_replacedThumbnailsNode->getContentWidth(), 0.6f, 0.2f);
    m_replacedThumbnailsLabel->setPosition({m_replacedThumbnailsNode->getContentSize().width / 2.f, m_replacedThumbnailsNode->getContentSize().height / 2.f - 5.f});
    m_replacedThumbnailsNode->addChild(m_replacedThumbnailsLabel);

    this->fetchDashboard();
    this->setKeypadEnabled(true);
    return true;
}

void ThumbnailDashboardLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void ThumbnailDashboardLayer::fetchDashboard() {
    auto overlay = CCBlockLayer::create();
    overlay->setPosition({0, 0});
    auto loadingCircle = cue::LoadingCircle::create(true);
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    loadingCircle->setPosition({winSize.width / 2.f, winSize.height / 2.f});
    overlay->addChild(loadingCircle, 2);
    this->addChild(overlay, 100);
    addBackButton(overlay, BackButtonStyle::Blue);

    auto removeOverlay = [overlay]() {
        if (overlay) {
            overlay->removeFromParent();
        }
    };

    auto req = web::WebRequest();
    auto authHeader = fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token"));
    req.header("Authorization", authHeader.c_str());
    auto task = req.get("https://levelthumbs.prevter.me/user/me");

    m_listener.spawn(std::move(task), [this, removeOverlay](web::WebResponse res) {
        if (res.code() < 200 || res.code() > 299) {
            auto message = res.string().unwrapOr("Unknown error");
            auto notificationText = std::string("Dashboard fetch failed: ") + message;
            Notification::create(notificationText.c_str(), NotificationIcon::Error)->show();
            removeOverlay();
            return;
        }

        auto jsonRes = res.json();
        if (!jsonRes.isOk()) {
            Notification::create("Failed to parse dashboard response.", NotificationIcon::Error)->show();
            removeOverlay();
            return;
        }

        auto json = jsonRes.unwrap();
        auto data = json["data"];
        if (!data.isObject()) {
            Notification::create("Dashboard response missing data.", NotificationIcon::Error)->show();
            removeOverlay();
            return;
        }

        int acceptedLevelCount = data["accepted_level_count"].asInt().unwrapOrDefault();
        int acceptedUploadCount = data["accepted_upload_count"].asInt().unwrapOrDefault();
        int accountId = data["account_id"].asInt().unwrapOrDefault();
        int activeThumbnailCount = data["active_thumbnail_count"].asInt().unwrapOrDefault();
        int id = data["id"].asInt().unwrapOrDefault();
        int levelCount = data["level_count"].asInt().unwrapOrDefault();
        int pendingUploadCount = data["pending_upload_count"].asInt().unwrapOrDefault();
        std::string role = data["role"].asString().unwrapOr("unknown");
        int uploadCount = data["upload_count"].asInt().unwrapOrDefault();
        std::string username = data["username"].asString().unwrapOr("unknown");

        m_title->setString(fmt::format("{}'s Thumbnails Dashboard", username).c_str());
        int rate = uploadCount > 0 ? static_cast<int>((acceptedUploadCount * 100 + uploadCount / 2) / uploadCount) : 0;
        if (m_acceptanceLabel) {
            m_acceptanceLabel->setTargetCount(rate);
        }
        if (m_activeThumbnailsLabel) {
            m_activeThumbnailsLabel->setTargetCount(activeThumbnailCount);
        }
        if (m_uploadLabel) {
            m_uploadLabel->setTargetCount(uploadCount);
        }
        if (m_progressBar) {
            m_progressBar->updateProgress(static_cast<float>(rate));
        }
        if (m_acceptanceUploadsLabel) {
            m_acceptanceUploadsLabel->setTargetCount(acceptedUploadCount);
        }
        if (m_uniqueLevelsLabel) {
            m_uniqueLevelsLabel->setTargetCount(levelCount);
        }
        if (m_rejectedUploadsLabel) {
            m_rejectedUploadsLabel->setTargetCount(uploadCount - acceptedUploadCount);
        }
        if (m_pendingUploadsLabel) {
            m_pendingUploadsLabel->setTargetCount(pendingUploadCount);
        }
        if (m_replacedThumbnailsLabel) {
            m_replacedThumbnailsLabel->setTargetCount(acceptedUploadCount - activeThumbnailCount);
        }
        removeOverlay();
    });
}
