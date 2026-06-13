#include "ThumbnailDashboardLayer.hpp"
#include "../include/BetterThumbnailConstant.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/ProgressBar.hpp>
#include <cue/LoadingCircle.hpp>
#include "Geode/ui/BasedButtonSprite.hpp"

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
        bg->setColor(ccc3(49, 44, 133));
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
    m_acceptanceStatsNode->setContentSize({140.f, 60.f});
    m_acceptanceStatsNode->setPosition({screenSize.width / 2.f - 100.f, screenSize.height - 80.f});
    this->addChild(m_acceptanceStatsNode);

    auto acceptanceStatsBg = NineSlice::create("square02_001.png");
    acceptanceStatsBg->setContentSize(m_acceptanceStatsNode->getContentSize());
    acceptanceStatsBg->setOpacity(150);
    acceptanceStatsBg->setPosition(m_acceptanceStatsNode->getContentSize() / 2.f);
    m_acceptanceStatsNode->addChild(acceptanceStatsBg);

    m_acceptanceTitle = CCLabelBMFont::create("Acceptance Rate", "goldFont.fnt");
    m_acceptanceTitle->limitLabelWidth(m_acceptanceStatsNode->getContentWidth(), 0.5f, 0.2f);
    m_acceptanceTitle->setPosition({m_acceptanceStatsNode->getContentSize().width / 2.f, m_acceptanceStatsNode->getContentSize().height - 10.f});
    m_acceptanceStatsNode->addChild(m_acceptanceTitle);

    m_acceptanceLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_acceptanceLabel->limitLabelWidth(m_acceptanceStatsNode->getContentWidth(), 0.6f, 0.2f);
    m_acceptanceLabel->setPosition({m_acceptanceStatsNode->getContentSize().width / 2.f, m_acceptanceStatsNode->getContentSize().height / 2.f - 5.f});
    m_acceptanceStatsNode->addChild(m_acceptanceLabel);

    // active thumbnails
    m_activeThumbnailsNode = CCNode::create();
    m_activeThumbnailsNode->setContentSize({140.f, 60.f});
    m_activeThumbnailsNode->setAnchorPoint({0.5f, 0.5f});
    m_activeThumbnailsNode->setPosition({screenSize.width / 2.f + 100.f, screenSize.height - 80.f});
    this->addChild(m_activeThumbnailsNode);

    auto activeThumbnailsBg = NineSlice::create("square02_001.png");
    activeThumbnailsBg->setContentSize(m_activeThumbnailsNode->getContentSize());
    activeThumbnailsBg->setOpacity(150);
    activeThumbnailsBg->setPosition(m_activeThumbnailsNode->getContentSize() / 2.f);
    m_activeThumbnailsNode->addChild(activeThumbnailsBg);

    m_activeThumbnailsTitle = CCLabelBMFont::create("Active Thumbnails", "goldFont.fnt");
    m_activeThumbnailsTitle->limitLabelWidth(m_activeThumbnailsNode->getContentWidth(), 0.5f, 0.2f);
    m_activeThumbnailsTitle->setPosition({m_activeThumbnailsNode->getContentSize().width / 2.f, m_activeThumbnailsNode->getContentSize().height - 10.f});
    m_activeThumbnailsNode->addChild(m_activeThumbnailsTitle);

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

    m_uploadTitle = CCLabelBMFont::create("Uploads", "goldFont.fnt");
    m_uploadTitle->limitLabelWidth(m_uploadStatsNode->getContentWidth(), 0.5f, 0.2f);
    m_uploadTitle->setPosition({m_uploadStatsNode->getContentSize().width / 2.f, m_uploadStatsNode->getContentSize().height - 10.f});
    m_uploadStatsNode->addChild(m_uploadTitle);

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

    m_acceptanceUploadsTitle = CCLabelBMFont::create("Accepted", "goldFont.fnt");
    m_acceptanceUploadsTitle->limitLabelWidth(m_acceptanceUploadsNode->getContentWidth(), 0.5f, 0.2f);
    m_acceptanceUploadsTitle->setPosition({m_acceptanceUploadsNode->getContentSize().width / 2.f, m_acceptanceUploadsNode->getContentSize().height - 10.f});
    m_acceptanceUploadsNode->addChild(m_acceptanceUploadsTitle);

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

    m_uniqueLevelsTitle = CCLabelBMFont::create("Unique Levels", "goldFont.fnt");
    m_uniqueLevelsTitle->limitLabelWidth(m_uniqueLevelsNode->getContentWidth(), 0.5f, 0.2f);
    m_uniqueLevelsTitle->setPosition({m_uniqueLevelsNode->getContentSize().width / 2.f, m_uniqueLevelsNode->getContentSize().height - 10.f});
    m_uniqueLevelsNode->addChild(m_uniqueLevelsTitle);

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

    m_rejectedUploadsTitle = CCLabelBMFont::create("Rejected Uploads", "goldFont.fnt");
    m_rejectedUploadsTitle->limitLabelWidth(m_rejectedUploadsNode->getContentWidth(), 0.5f, 0.2f);
    m_rejectedUploadsTitle->setPosition({m_rejectedUploadsNode->getContentSize().width / 2.f, m_rejectedUploadsNode->getContentSize().height - 10.f});
    m_rejectedUploadsNode->addChild(m_rejectedUploadsTitle);

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

    m_pendingUploadsTitle = CCLabelBMFont::create("Pending Uploads", "goldFont.fnt");
    m_pendingUploadsTitle->limitLabelWidth(m_pendingUploadsNode->getContentWidth(), 0.5f, 0.2f);
    m_pendingUploadsTitle->setPosition({m_pendingUploadsNode->getContentSize().width / 2.f, m_pendingUploadsNode->getContentSize().height - 10.f});
    m_pendingUploadsNode->addChild(m_pendingUploadsTitle);

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

    m_replacedThumbnailsTitle = CCLabelBMFont::create("Replaced Thumbnails", "goldFont.fnt");
    m_replacedThumbnailsTitle->limitLabelWidth(m_replacedThumbnailsNode->getContentWidth(), 0.5f, 0.2f);
    m_replacedThumbnailsTitle->setPosition({m_replacedThumbnailsNode->getContentSize().width / 2.f, m_replacedThumbnailsNode->getContentSize().height - 10.f});
    m_replacedThumbnailsNode->addChild(m_replacedThumbnailsTitle);

    m_replacedThumbnailsLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_replacedThumbnailsLabel->limitLabelWidth(m_replacedThumbnailsNode->getContentWidth(), 0.6f, 0.2f);
    m_replacedThumbnailsLabel->setPosition({m_replacedThumbnailsNode->getContentSize().width / 2.f, m_replacedThumbnailsNode->getContentSize().height / 2.f - 5.f});
    m_replacedThumbnailsNode->addChild(m_replacedThumbnailsLabel);

    // users total
    m_usersTotalNode = CCNode::create();
    m_usersTotalNode->setContentSize({140.f, 60.f});
    m_usersTotalNode->setAnchorPoint({0.5f, 0.5f});
    m_usersTotalNode->setPosition({screenSize.width / 2.f - 150.f, screenSize.height - 280.f});
    this->addChild(m_usersTotalNode);
    m_usersTotalNode->setVisible(false);

    auto usersTotalBg = NineSlice::create("square02_001.png");
    usersTotalBg->setContentSize(m_usersTotalNode->getContentSize());
    usersTotalBg->setOpacity(150);
    usersTotalBg->setPosition(m_usersTotalNode->getContentSize() / 2.f);
    m_usersTotalNode->addChild(usersTotalBg);

    m_usersTotalTitle = CCLabelBMFont::create("Total Users", "goldFont.fnt");
    m_usersTotalTitle->limitLabelWidth(m_usersTotalNode->getContentWidth(), 0.5f, 0.2f);
    m_usersTotalTitle->setPosition({m_usersTotalNode->getContentSize().width / 2.f, m_usersTotalNode->getContentSize().height - 10.f});
    m_usersTotalNode->addChild(m_usersTotalTitle);

    m_usersTotalLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_usersTotalLabel->limitLabelWidth(m_usersTotalNode->getContentWidth(), 0.6f, 0.2f);
    m_usersTotalLabel->setPosition({m_usersTotalNode->getContentSize().width / 2.f, m_usersTotalNode->getContentSize().height / 2.f - 5.f});
    m_usersTotalNode->addChild(m_usersTotalLabel);

    // users per month
    m_usersPerMonthNode = CCNode::create();
    m_usersPerMonthNode->setContentSize({140.f, 60.f});
    m_usersPerMonthNode->setAnchorPoint({0.5f, 0.5f});
    m_usersPerMonthNode->setPosition({screenSize.width / 2.f, screenSize.height - 280.f});
    this->addChild(m_usersPerMonthNode);
    m_usersPerMonthNode->setVisible(false);

    auto usersPerMonthBg = NineSlice::create("square02_001.png");
    usersPerMonthBg->setContentSize(m_usersPerMonthNode->getContentSize());
    usersPerMonthBg->setOpacity(150);
    usersPerMonthBg->setPosition(m_usersPerMonthNode->getContentSize() / 2.f);
    m_usersPerMonthNode->addChild(usersPerMonthBg);

    m_usersPerMonthTitle = CCLabelBMFont::create("Monthly Users", "goldFont.fnt");
    m_usersPerMonthTitle->limitLabelWidth(m_usersPerMonthNode->getContentWidth(), 0.5f, 0.2f);
    m_usersPerMonthTitle->setPosition({m_usersPerMonthNode->getContentSize().width / 2.f, m_usersPerMonthNode->getContentSize().height - 10.f});
    m_usersPerMonthNode->addChild(m_usersPerMonthTitle);

    m_usersPerMonthLabel = CCCounterLabel::create(0, "bigFont.fnt", FormatterType::Integer);
    m_usersPerMonthLabel->limitLabelWidth(m_usersPerMonthNode->getContentWidth(), 0.6f, 0.2f);
    m_usersPerMonthLabel->setPosition({m_usersPerMonthNode->getContentSize().width / 2.f, m_usersPerMonthNode->getContentSize().height / 2.f - 5.f});
    m_usersPerMonthNode->addChild(m_usersPerMonthLabel);

    // storage
    m_storageNode = CCNode::create();
    m_storageNode->setContentSize({140.f, 60.f});
    m_storageNode->setAnchorPoint({0.5f, 0.5f});
    m_storageNode->setPosition({screenSize.width / 2.f + 150.f, screenSize.height - 280.f});
    this->addChild(m_storageNode);
    m_storageNode->setVisible(false);

    auto storageBg = NineSlice::create("square02_001.png");
    storageBg->setContentSize(m_storageNode->getContentSize());
    storageBg->setOpacity(150);
    storageBg->setPosition(m_storageNode->getContentSize() / 2.f);
    m_storageNode->addChild(storageBg);

    m_storageTitle = CCLabelBMFont::create("Storage Used", "goldFont.fnt");
    m_storageTitle->limitLabelWidth(m_storageNode->getContentWidth(), 0.5f, 0.2f);
    m_storageTitle->setPosition({m_storageNode->getContentSize().width / 2.f, m_storageNode->getContentSize().height - 10.f});
    m_storageNode->addChild(m_storageTitle);

    m_storageLabel = CCLabelBMFont::create("0 B", "bigFont.fnt");
    m_storageLabel->limitLabelWidth(m_storageNode->getContentWidth(), 0.6f, 0.2f);
    m_storageLabel->setPosition({m_storageNode->getContentSize().width / 2.f, m_storageNode->getContentSize().height / 2.f - 5.f});
    m_storageNode->addChild(m_storageLabel);

    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    this->addChild(menu);

    auto statsBtnSprite = CircleButtonSprite::createWithSprite("BT_meIcon.png"_spr, .8f, CircleBaseColor::Green, CircleBaseSize::Medium);
    auto statsBtnSpriteOn = CircleButtonSprite::createWithSprite("BT_globalIcon.png"_spr, 1.f, CircleBaseColor::Green, CircleBaseSize::Medium);

    m_statsToggler = CCMenuItemToggler::create(statsBtnSprite, statsBtnSpriteOn, this, menu_selector(ThumbnailDashboardLayer::onToggleStats));
    m_statsToggler->setPosition({screenSize.width - 35.f, screenSize.height - 35.f});
    menu->addChild(m_statsToggler);

    this->fetchDashboard();
    this->fetchGlobalStats();
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

    auto req = betterThumbnail::createWebRequest();
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

        m_userStats.levelCount = levelCount;
        m_userStats.uploadCount = uploadCount;
        m_userStats.acceptedUploadCount = acceptedUploadCount;
        m_userStats.activeThumbnailCount = activeThumbnailCount;
        m_userStats.pendingUploadCount = pendingUploadCount;
        m_userStats.username = username;

        this->updateUI();
        removeOverlay();
    });
}

void ThumbnailDashboardLayer::fetchGlobalStats() {
    auto req = betterThumbnail::createWebRequest();
    auto task = req.get("https://levelthumbs.prevter.me/stats");

    m_statsListener.spawn(std::move(task), [this](web::WebResponse res) {
        if (res.code() < 200 || res.code() > 299) return;
        auto jsonRes = res.json();
        if (!jsonRes.isOk()) return;
        auto json = jsonRes.unwrap();
        auto data = json["data"];
        if (!data.isObject()) return;

        m_globalStats.accepted_uploads_total = data["accepted_uploads_total"].asInt().unwrapOrDefault();
        m_globalStats.current_pending_uploads = data["current_pending_uploads"].asInt().unwrapOrDefault();
        m_globalStats.pending_uploads_total = data["pending_uploads_total"].asInt().unwrapOrDefault();
        m_globalStats.storage = data["storage"].asInt().unwrapOrDefault();
        m_globalStats.thumbnails = data["thumbnails"].asInt().unwrapOrDefault();
        m_globalStats.total_levels = data["total_levels"].asInt().unwrapOrDefault();
        m_globalStats.uploads_total = data["uploads_total"].asInt().unwrapOrDefault();
        m_globalStats.users_per_month = data["users_per_month"].asInt().unwrapOrDefault();
        m_globalStats.users_total = data["users_total"].asInt().unwrapOrDefault();

        this->updateUI();
    });
}

void ThumbnailDashboardLayer::onToggleStats(CCObject* sender) {
    m_showingGlobalStats = !m_statsToggler->isToggled();
    this->updateUI();
}

void ThumbnailDashboardLayer::updateUI() {
    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    if (m_showingGlobalStats) {
        m_title->setString("Global Thumbnails Dashboard");

        int uploadCount = m_globalStats.uploads_total;
        int acceptedUploadCount = m_globalStats.accepted_uploads_total;
        int rate = uploadCount > 0 ? static_cast<int>((acceptedUploadCount * 100 + uploadCount / 2) / uploadCount) : 0;

        m_acceptanceStatsNode->setVisible(false);
        m_progressBar->setVisible(false);
        m_activeThumbnailsNode->setPosition({screenSize.width / 2.f, screenSize.height - 80.f});
        m_uploadStatsNode->setPosition({screenSize.width / 2.f - 150.f, screenSize.height - 150.f});
        m_acceptanceUploadsNode->setPosition({screenSize.width / 2.f, screenSize.height - 150.f});
        m_uniqueLevelsNode->setPosition({screenSize.width / 2.f + 150.f, screenSize.height - 150.f});
        m_rejectedUploadsNode->setPosition({screenSize.width / 2.f - 150.f, screenSize.height - 215.f});
        m_pendingUploadsNode->setPosition({screenSize.width / 2.f, screenSize.height - 215.f});
        m_replacedThumbnailsNode->setPosition({screenSize.width / 2.f + 150.f, screenSize.height - 215.f});

        m_activeThumbnailsLabel->setTargetCount(m_globalStats.thumbnails);
        m_uploadLabel->setTargetCount(uploadCount);
        m_acceptanceUploadsLabel->setTargetCount(acceptedUploadCount);
        m_uniqueLevelsLabel->setTargetCount(m_globalStats.total_levels);
        m_rejectedUploadsLabel->setTargetCount(uploadCount - acceptedUploadCount);
        m_pendingUploadsLabel->setTargetCount(m_globalStats.current_pending_uploads);
        m_replacedThumbnailsLabel->setTargetCount(acceptedUploadCount - m_globalStats.thumbnails);

        m_usersTotalNode->setVisible(true);
        m_usersTotalLabel->setTargetCount(m_globalStats.users_total);
        m_usersPerMonthNode->setVisible(true);
        m_usersPerMonthLabel->setTargetCount(m_globalStats.users_per_month);
        m_storageNode->setVisible(true);

        double storageGB = m_globalStats.storage / (1024.0 * 1024.0 * 1024.0);
        double storageMB = m_globalStats.storage / (1024.0 * 1024.0);
        if (storageGB >= 1.0) {
            m_storageLabel->setString(fmt::format("{:.2f} GB", storageGB).c_str());
        } else {
            m_storageLabel->setString(fmt::format("{:.2f} MB", storageMB).c_str());
        }
    } else {
        m_title->setString(fmt::format("{}'s Thumbnails Dashboard", m_userStats.username).c_str());

        int uploadCount = m_userStats.uploadCount;
        int acceptedUploadCount = m_userStats.acceptedUploadCount;
        int rate = uploadCount > 0 ? static_cast<int>((acceptedUploadCount * 100 + uploadCount / 2) / uploadCount) : 0;

        m_acceptanceStatsNode->setVisible(true);
        m_progressBar->setVisible(true);
        m_activeThumbnailsNode->setPosition({screenSize.width / 2.f + 100.f, screenSize.height - 80.f});
        m_uploadStatsNode->setPosition({screenSize.width / 2.f - 150.f, screenSize.height - 180.f});
        m_acceptanceUploadsNode->setPosition({screenSize.width / 2.f, screenSize.height - 180.f});
        m_uniqueLevelsNode->setPosition({screenSize.width / 2.f + 150.f, screenSize.height - 180.f});
        m_rejectedUploadsNode->setPosition({screenSize.width / 2.f - 150.f, screenSize.height - 250.f});
        m_pendingUploadsNode->setPosition({screenSize.width / 2.f, screenSize.height - 250.f});
        m_replacedThumbnailsNode->setPosition({screenSize.width / 2.f + 150.f, screenSize.height - 250.f});

        m_acceptanceLabel->setTargetCount(rate);
        m_activeThumbnailsLabel->setTargetCount(m_userStats.activeThumbnailCount);
        m_uploadLabel->setTargetCount(uploadCount);
        m_progressBar->updateProgress(static_cast<float>(rate));
        m_acceptanceUploadsLabel->setTargetCount(acceptedUploadCount);
        m_uniqueLevelsLabel->setTargetCount(m_userStats.levelCount);
        m_rejectedUploadsLabel->setTargetCount(uploadCount - acceptedUploadCount);
        m_pendingUploadsLabel->setTargetCount(m_userStats.pendingUploadCount);
        m_replacedThumbnailsLabel->setTargetCount(acceptedUploadCount - m_userStats.activeThumbnailCount);

        m_usersTotalNode->setVisible(false);
        m_usersPerMonthNode->setVisible(false);
        m_storageNode->setVisible(false);
    }
}
