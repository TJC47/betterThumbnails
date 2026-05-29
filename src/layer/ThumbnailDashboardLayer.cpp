#include "ThumbnailDashboardLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>

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

    addSideArt(this, SideArt::TopLeft, false);
    addSideArt(this, SideArt::Bottom, false);

    auto bg = createLayerBG();
    if (bg != nullptr) {
        this->addChild(bg, -1);
    }

    addBackButton(this, BackButtonStyle::Green);

    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    m_title = CCLabelBMFont::create("Welcome...", "goldFont.fnt");
    m_title->setAnchorPoint({0.5f, 1.f});
    m_title->setPosition({screenSize.width / 2.f, screenSize.height - 40.f});
    m_title->setScale(0.8f);
    this->addChildAtPosition(m_title, Anchor::Top, {0.f, -20.f}, false);

    m_statsNode = CCNode::create();
    m_statsNode->setAnchorPoint({0.5, 0.5});
    m_statsNode->setContentWidth(255.f);
    m_statsNode->setPosition({screenSize.width / 2.f, screenSize.height - 60.f});
    this->addChild(m_statsNode);

    auto statsBg = NineSlice::create("square02_001.png");
    statsBg->setContentSize({280.f, 40.f});
    statsBg->setOpacity(150);
    statsBg->setAnchorPoint({0.f, 1.f});
    statsBg->setPosition({-5.f, 5.f});
    m_statsNode->addChild(statsBg);

    m_acceptanceLabel = CCLabelBMFont::create("Acceptance Rate:\n-", "bigFont.fnt");
    m_acceptanceLabel->setScale(0.4f);
    m_acceptanceLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_acceptanceLabel->setAnchorPoint({0.f, 1.f});
    m_acceptanceLabel->setPosition({0.f, m_statsNode->getContentSize().height});
    m_statsNode->addChild(m_acceptanceLabel);

    m_activeThumbnailsLabel = CCLabelBMFont::create("Active Thumbnails:\n-", "bigFont.fnt");
    m_activeThumbnailsLabel->setScale(0.4f);
    m_activeThumbnailsLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_activeThumbnailsLabel->setAnchorPoint({0.f, 1.f});
    m_activeThumbnailsLabel->setPosition({140.f, 0.f});
    m_statsNode->addChild(m_activeThumbnailsLabel);

    m_uploadStatsNode = CCNode::create();
    m_uploadStatsNode->setPosition({screenSize.width / 2.f, screenSize.height - 110.f});
    m_uploadStatsNode->setAnchorPoint({0.5, 0.5});
    m_uploadStatsNode->setContentWidth(255.f);
    this->addChild(m_uploadStatsNode);

    auto uploadStatsBg = NineSlice::create("square02_001.png");
    uploadStatsBg->setContentSize({260.f, 40.f});
    uploadStatsBg->setOpacity(150);
    uploadStatsBg->setAnchorPoint({0.f, 1.f});
    uploadStatsBg->setPosition({-5.f, 5.f});
    m_uploadStatsNode->addChild(uploadStatsBg);

    m_acceptedUploadsLabel = CCLabelBMFont::create("Accepted:\n-", "bigFont.fnt");
    m_acceptedUploadsLabel->setScale(0.39f);
    m_acceptedUploadsLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_acceptedUploadsLabel->setAnchorPoint({0.f, 1.f});
    m_acceptedUploadsLabel->setPosition({0.f, 0.f});
    m_uploadStatsNode->addChild(m_acceptedUploadsLabel);

    m_acceptedLevelsLabel = CCLabelBMFont::create("Unique Levels:\n-", "bigFont.fnt");
    m_acceptedLevelsLabel->setScale(0.39f);
    m_acceptedLevelsLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_acceptedLevelsLabel->setAnchorPoint({0.f, 1.f});
    m_acceptedLevelsLabel->setPosition({80.f, 0.f});
    m_uploadStatsNode->addChild(m_acceptedLevelsLabel);

    m_pendingUploadsLabel = CCLabelBMFont::create("Pending:\n-", "bigFont.fnt");
    m_pendingUploadsLabel->setScale(0.39f);
    m_pendingUploadsLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_pendingUploadsLabel->setAnchorPoint({0.f, 1.f});
    m_pendingUploadsLabel->setPosition({195.f, 0.f});
    m_uploadStatsNode->addChild(m_pendingUploadsLabel);

    this->fetchDashboard();
    this->setKeypadEnabled(true);
    return true;
}

void ThumbnailDashboardLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void ThumbnailDashboardLayer::fetchDashboard() {
    auto req = web::WebRequest();
    auto authHeader = fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token"));
    req.header("Authorization", authHeader.c_str());
    auto task = req.get("https://levelthumbs.prevter.me/user/me");

    m_listener.spawn(std::move(task), [this](web::WebResponse res) {
        if (res.code() < 200 || res.code() > 299) {
            auto message = res.string().unwrapOr("Unknown error");
            auto notificationText = std::string("Dashboard fetch failed: ") + message;
            Notification::create(notificationText.c_str(), NotificationIcon::Error)->show();
            return;
        }

        auto jsonRes = res.json();
        if (!jsonRes.isOk()) {
            Notification::create("Failed to parse dashboard response.", NotificationIcon::Error)->show();
            return;
        }

        auto json = jsonRes.unwrap();
        auto data = json["data"];
        if (!data.isObject()) {
            Notification::create("Dashboard response missing data.", NotificationIcon::Error)->show();
            return;
        }

        auto acceptedLevelCount = data["accepted_level_count"].asInt().unwrapOrDefault();
        auto acceptedUploadCount = data["accepted_upload_count"].asInt().unwrapOrDefault();
        auto accountId = data["account_id"].asInt().unwrapOrDefault();
        auto activeThumbnailCount = data["active_thumbnail_count"].asInt().unwrapOrDefault();
        auto id = data["id"].asInt().unwrapOrDefault();
        auto levelCount = data["level_count"].asInt().unwrapOrDefault();
        auto pendingUploadCount = data["pending_upload_count"].asInt().unwrapOrDefault();
        auto role = data["role"].asString().unwrapOr("unknown");
        auto uploadCount = data["upload_count"].asInt().unwrapOrDefault();
        auto username = data["username"].asString().unwrapOr("unknown");

        m_title->setString(fmt::format("Welcome, {}!", username).c_str());
        if (m_acceptanceLabel) {
            auto rate = uploadCount > 0 ? static_cast<int>((acceptedUploadCount * 100 + uploadCount / 2) / uploadCount) : 0;
            m_acceptanceLabel->setString(fmt::format("Acceptance Rate:\n{}%", rate).c_str());
        }
        if (m_activeThumbnailsLabel) {
            m_activeThumbnailsLabel->setString(fmt::format("Active Thumbnails:\n{}", activeThumbnailCount).c_str());
        }
        if (m_acceptedUploadsLabel) {
            m_acceptedUploadsLabel->setString(fmt::format("Accepted:\n{}", acceptedUploadCount).c_str());
        }
        if (m_acceptedLevelsLabel) {
            m_acceptedLevelsLabel->setString(fmt::format("Unique Levels:\n{}", acceptedLevelCount).c_str());
        }
        if (m_pendingUploadsLabel) {
            m_pendingUploadsLabel->setString(fmt::format("Pending:\n{}", pendingUploadCount).c_str());
        }
    });
}
