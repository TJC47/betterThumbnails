#include "BetterThumbnailLayer.hpp"
#include "../overlay/NotificationOverlay.hpp"

#include <Geode/Geode.hpp>
#include "../popup/NotificationMenuPopup.hpp"
#include <string>

#include "Geode/ui/General.hpp"
#include "Geode/ui/Notification.hpp"
#include <Geode/ui/Button.hpp>
#include "../include/BetterThumbnailConstant.hpp"
#include "ManageUserLayer.hpp"
#include "PendingThumbnailLayer.hpp"
#include "MyThumbnailsLayer.hpp"
#include "ThumbnailDashboardLayer.hpp"

using namespace geode::prelude;

BetterThumbnailLayer* BetterThumbnailLayer::create() {
    auto ret = new BetterThumbnailLayer;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BetterThumbnailLayer::init() {
    if (!CCLayer::init())
        return false;

    auto bg = createLayerBG();
    if (bg != nullptr) {
        this->addChild(bg, -1);
    } else {
        log::error("createLayerBG returned nullptr");
        return false;
    }

    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    if (Mod::get()->getSettingValue<bool>("random-thumbnail-background") ==
        true) {
        auto bgImage = LazySprite::create(bg->getScaledContentSize(), true);
        bgImage->setID("better-thumbnail-bg");
        bgImage->setAutoResize(true);
        bgImage->setContentSize(bg->getScaledContentSize());
        bgImage->setPosition({0.f, 0.f});

        auto loadingImageLabel =
            CCLabelBMFont::create("Loading thumbnail...", "goldFont.fnt");
        loadingImageLabel->setPosition(
            {screenSize.width / 2.f, screenSize.height / 8.f});
        loadingImageLabel->setScale(0.5f);
        this->addChild(loadingImageLabel, 3);

        bgImage->setLoadCallback([this, screenSize, bg, bgImage, loadingImageLabel](
                                     geode::Result<void, std::string> result) {
            if (result.isOk() || bgImage->isLoaded()) {
                log::info("Thumbnail loaded, fading out background");
                loadingImageLabel->removeFromParent();
                bgImage->setAnchorPoint({0.5, 0.5});

                auto bgDark = NineSlice::create("square02_001.png");
                bgDark->setContentSize(
                    {screenSize.width + 10.f, screenSize.height + 10.f});
                bgDark->setPosition({screenSize.width / 2.f, screenSize.height / 2.f});
                bgDark->setOpacity(175);

                bgImage->setPosition({screenSize.width / 2.f, screenSize.height / 2.f});
                bgImage->setScale(screenSize.width / bgImage->getContentWidth());

                this->addChild(bgDark, -2);

                bg->runAction(CCFadeTo::create(1.f, 0));
            } else {
                log::error("Failed to load thumbnail: {}", result.unwrapErr());
                loadingImageLabel->setString("Failed to load thumbnail");
            }
        });

        // please laugh, lazysprite no supports webp
        bgImage->loadFromUrl(
            betterThumbnail::makeUrl("/thumbnail/random"),
            LazySprite::Format::kFmtUnKnown,
            true);
        this->addChild(bgImage, -3);
    }
    m_bottomLeftMenu = CCMenu::create();
    m_bottomLeftMenu->setLayout(ColumnLayout::create()->setAxisAlignment(AxisAlignment::Start));
    m_bottomLeftMenu->setContentSize({35.f, 100.f});
    m_bottomLeftMenu->setZOrder(1);
    this->addChildAtPosition(m_bottomLeftMenu, Anchor::BottomLeft, {25.f, 60.f}, false);

    m_menuButtons = CCMenu::create();
    m_menuButtons->setContentSize({450.f, 310.f});
    m_menuButtons->ignoreAnchorPointForPosition(false);
    m_menuButtons->setAnchorPoint({0.5f, 0.5f});
    m_menuButtons->setPosition({screenSize.width / 2.f, screenSize.height / 2.f});
    auto menuButtonsLayout = RowLayout::create();
    menuButtonsLayout->setGap(6.f);
    menuButtonsLayout->setAxisReverse(false);
    menuButtonsLayout->setGrowCrossAxis(false);
    m_menuButtons->setLayout(menuButtonsLayout);
    this->addChild(m_menuButtons, 2);

    // Top-right user info menu
    auto userInfoMenu = CCMenu::create();
    userInfoMenu->setID("user-info-menu");
    userInfoMenu->setAnchorPoint({1.f, 1.f});
    userInfoMenu->setContentSize({100.f, 100.f});
    userInfoMenu->ignoreAnchorPointForPosition(false);
    userInfoMenu->setPosition({screenSize.width - 5.f, screenSize.height - 5.f});
    this->addChild(userInfoMenu, 10);

    const float menuWidth = userInfoMenu->getContentSize().width;
    const float menuHeight = userInfoMenu->getContentSize().height;

    const float padding = 3.f;
    const float startX = menuWidth;
    const float startY = menuHeight;

    // user account
    std::string username = Mod::get()->getSavedValue<std::string>("username");
    auto userLabel = CCLabelBMFont::create(username.c_str(), "goldFont.fnt");
    userLabel->setAnchorPoint({1.f, 1.f});
    userLabel->setScale(0.5f);
    userLabel->setAlignment(kCCTextAlignmentRight);

    // user rank
    std::string userRank = Mod::get()->getSavedValue<std::string>("role");
    auto userRankLabel =
        CCLabelBMFont::create(("(" + userRank + ")").c_str(), "goldFont.fnt");
    userRankLabel->setAnchorPoint({1.f, 1.f});
    userRankLabel->setScale(0.3f);
    userRankLabel->setAlignment(kCCTextAlignmentRight);

    userLabel->setPosition({startX, startY});
    userInfoMenu->addChild(userLabel);

    CCSprite* badgeSprite = nullptr;

    if (userRank == "owner") {
        badgeSprite = CCSprite::create("BT_ownerBadge.png"_spr);
    } else if (userRank == "admin") {
        badgeSprite = CCSprite::create("BT_adminBadge.png"_spr);
    } else if (userRank == "moderator") {
        badgeSprite = CCSprite::create("BT_thumbmodBadge.png"_spr);
    } else if (userRank == "verified") {
        badgeSprite = CCSprite::create("BT_verifiedBadge.png"_spr);
    } else {
        badgeSprite = nullptr;
    }

    if (badgeSprite) {
        badgeSprite->setAnchorPoint({1.f, 1.f});
        badgeSprite->setScale(0.45f);
        float badgeOffset =
            userLabel->getContentSize().width * userLabel->getScale() + 1.f;
        badgeSprite->setPosition({startX - badgeOffset, startY});
        userInfoMenu->addChild(badgeSprite);
    }

    float userRankY = startY -
                      userLabel->getContentSize().height * userLabel->getScale() -
                      padding;
    userRankLabel->setPosition({startX, userRankY});
    userInfoMenu->addChild(userRankLabel);

    // thumbnail coin counter
    coinLabel = CCCounterLabel::create(0, "bigFont.fnt", static_cast<FormatterType>(0));
    coinLabel->setAnchorPoint({1.f, 1.f});
    coinLabel->setScale(0.5f);
    coinLabel->setAlignment(kCCTextAlignmentRight);

    auto coinSprite = CCSprite::create("BT_ThumbnailCoin.png"_spr);
    coinSprite->setAnchorPoint({1.f, 1.f});
    coinSprite->setScale(0.65f);

    float coinLabelY =
        userRankY -
        userRankLabel->getContentSize().height * userRankLabel->getScale() -
        padding - 3.f;
    float coinLabelX = startX - 25.f;

    coinSprite->setPosition({startX, coinLabelY + 2.f});
    userInfoMenu->addChild(coinSprite);

    coinLabel->setPosition({coinLabelX, coinLabelY});
    userInfoMenu->addChild(coinLabel);

    addBackButton(this, BackButtonStyle::Green);

    // info button
    auto infoButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(BetterThumbnailLayer::onInfoButton));
    m_bottomLeftMenu->addChild(infoButton);

    auto notificationButton = geode::Button::createWithNode(CircleButtonSprite::createWithSprite("BT_notificationIcon.png"_spr, .9f, CircleBaseColor::Green, CircleBaseSize::Small), [this](geode::Button* btn) {
        if (auto overlay = NotificationOverlay::get()) {
            overlay->showNotificationList();
            return;
        }

        auto popup = NotificationMenuPopup::create();
        if (popup) {
            popup->show();
        }
    });
    m_bottomLeftMenu->addChild(notificationButton);
    m_bottomLeftMenu->updateLayout();
    // get user info
    auto req = betterThumbnail::createWebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));

    auto task = req.get(betterThumbnail::makeUrl("/user/me"));
    async::spawn(std::move(task), [this](web::WebResponse res) {
        auto code = res.code();
        if (code < 200 || code > 299) {
            auto error = res.string().unwrapOr(std::string(res.errorMessage()));
            Notification::create(fmt::format("{}", error),
                NotificationIcon::Error)
                ->show();
            return;
        }
        log::info("{} {}", res.code(), res.string().unwrapOrDefault());
        auto json = res.json().unwrapOrDefault();
        log::info("{} {}", res.code(), json.dump());
        auto activeThumbnailCount =
            json["data"]["active_thumbnail_count"].asInt().unwrapOrDefault();
        auto uploadThumbnailCount =
            json["data"]["upload_count"].asInt().unwrapOrDefault();
        auto acceptUploadThumbnailCount =
            json["data"]["accepted_upload_count"].asInt().unwrapOrDefault();
        log::debug("{}", activeThumbnailCount);
        this->m_activeThumbnailCount = activeThumbnailCount;
        this->m_uploadThumbnailCount = uploadThumbnailCount;
        this->m_acceptedUploadThumbnailCount = acceptUploadThumbnailCount;
        if (this->coinLabel) {
            this->coinLabel->setTargetCount(activeThumbnailCount);
        }
    });

    // Main buttons

    auto myThumbSprite = CCSprite::create("BT_myThumbnailsButton.png"_spr);
    auto myThumbBtn = CCMenuItemSpriteExtra::create(
        myThumbSprite, this, menu_selector(BetterThumbnailLayer::onMyThumbnail));

    auto dashboardSprite = CCSprite::create("BT_dashboard.png"_spr);
    auto dashboardBtn = CCMenuItemSpriteExtra::create(
        dashboardSprite, this, menu_selector(BetterThumbnailLayer::onDashboard));

    if (betterThumbnail::hasRoleAtLeast(betterThumbnail::RoleNum::Moderator)) {
        pendingSprite = CCSprite::create("BT_pendingButton.png"_spr);
    } else {
        pendingSprite = CCSpriteGrayscale::create("BT_pendingButton.png"_spr);
    }

    auto pendingBtn = CCMenuItemSpriteExtra::create(
        pendingSprite, this, menu_selector(BetterThumbnailLayer::onPending));

    if (betterThumbnail::hasRoleAtLeast(betterThumbnail::RoleNum::Moderator)) {
        manageSprite = CCSprite::create("BT_manageUsersButton.png"_spr);
    } else {
        manageSprite = CCSpriteGrayscale::create("BT_manageUsersButton.png"_spr);
    }

    auto manageBtn = CCMenuItemSpriteExtra::create(
        manageSprite, this, menu_selector(BetterThumbnailLayer::onManage));

    m_menuButtons->addChild(dashboardBtn);
    m_menuButtons->addChild(myThumbBtn);
    m_menuButtons->addChild(pendingBtn);
    m_menuButtons->addChild(manageBtn);

    m_menuButtons->updateLayout();

    // funny side art
    addSideArt(this, SideArt::BottomLeft, SideArtStyle::Layer, false);
    addSideArt(this, SideArt::BottomRight, SideArtStyle::Layer, false);

    this->setKeypadEnabled(true);

    return true;
}

void BetterThumbnailLayer::onMyThumbnail(CCObject*) {
    auto layer = MyThumbnailsLayer::create();
    auto scene = CCScene::create();
    auto transition = CCTransitionFade::create(.5f, scene);
    scene->addChild(layer);
    CCDirector::get()->pushScene(transition);
}
void BetterThumbnailLayer::onDashboard(CCObject*) {
    auto layer = ThumbnailDashboardLayer::create();
    auto scene = CCScene::create();
    auto transition = CCTransitionFade::create(.5f, scene);
    scene->addChild(layer);
    CCDirector::get()->pushScene(transition);
}
void BetterThumbnailLayer::onPending(CCObject*) {
    if (betterThumbnail::hasRoleAtLeast(betterThumbnail::RoleNum::Moderator)) {
        auto layer = PendingThumbnailLayer::create();
        auto scene = CCScene::create();
        auto transition = CCTransitionFade::create(.5f, scene);
        scene->addChild(layer);
        CCDirector::get()->pushScene(transition);
    } else {
        FLAlertLayer::create("Pending Thumbnails",
            "You do not have permission to access this menu.",
            "Ok")
            ->show();
    }
}
void BetterThumbnailLayer::onManage(CCObject*) {
    if (betterThumbnail::hasRoleAtLeast(betterThumbnail::RoleNum::Moderator)) {
        auto scene = CCScene::create();
        scene->addChild(ManageUserLayer::create());
        CCDirector::get()->pushScene(CCTransitionFade::create(.5f, scene));
    } else {
        FLAlertLayer::create(
            "Manage User", "You do not have permission to access this menu.", "Ok")
            ->show();
    }
}

void BetterThumbnailLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f,
        PopTransition::kPopTransitionFade);
}
void BetterThumbnailLayer::onInfoButton(CCObject*) {
    std::string userRank = Mod::get()->getSavedValue<std::string>("role");
    auto userId = Mod::get()->getSavedValue<int>("user_id");
    auto activeThumbnails = m_activeThumbnailCount;
    auto userRankNum = betterThumbnail::getRoleNum();
    auto uploadThumbnailCount = m_uploadThumbnailCount;
    auto acceptUploadThumbnailCount = m_acceptedUploadThumbnailCount;
    auto infoString =
        fmt::format(
            "<cg>Rank:</c> {}\n<cc>Rank (numerical)</c>: {}\n<cl>User ID:</c> {}\n<ca>Active Thumbnails:</c> {}\n<co>Uploaded Thumbnails:</c> {}/{} accepted",
            userRank,
            userRankNum,
            userId,
            activeThumbnails,
            acceptUploadThumbnailCount,
            uploadThumbnailCount);
    FLAlertLayer::create(
        Mod::get()->getSavedValue<std::string>("username").c_str(), infoString, "Ok")
        ->show();
}