#include "BetterThumbnailLayer.hpp"

#include <Geode/Geode.hpp>
#include <algorithm>

#include "Geode/ui/General.hpp"
#include "Geode/ui/OverlayManager.hpp"
#include "PendingThumbnailLayer.hpp"
#include "../node/NotificationNode.hpp"
#include "../popup/NotificationMenuPopup.hpp"

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
        bgImage->loadFromUrl("https://levelthumbs.prevter.me/thumbnail/random",
            LazySprite::Format::kFmtUnKnown,
            true);
        this->addChild(bgImage, -3);
    }
    auto menu = CCMenu::create();
    this->addChild(menu, 2);
    menu->setPosition({0.f, 0.f});

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
        badgeSprite = CCSprite::create("ownerBadge.png"_spr);
    } else if (userRank == "admin") {
        badgeSprite = CCSprite::create("adminBadge.png"_spr);
    } else if (userRank == "moderator") {
        badgeSprite = CCSprite::create("thumbmodBadge.png"_spr);
    } else if (userRank == "verified") {
        badgeSprite = CCSprite::create("verifiedBadge.png"_spr);
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
    auto coinLabel =
        CCLabelBMFont::create("-", "bigFont.fnt");  // - as a placeholder
    coinLabel->setAnchorPoint({1.f, 1.f});
    coinLabel->setScale(0.5f);
    coinLabel->setAlignment(kCCTextAlignmentRight);

    auto coinSprite = CCSprite::create("ThumbnailCoin.png"_spr);
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
    infoButton->setPosition({25.f, 25.f});
    menu->addChild(infoButton);

    // get user info
    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));

    auto task = req.get(fmt::format("https://levelthumbs.prevter.me/user/me"));
    async::spawn(std::move(task), [this, coinLabel](web::WebResponse res) {
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
        Mod::get()->setSavedValue<int>("active_thumbnail_count",
            activeThumbnailCount);
        Mod::get()->setSavedValue<int>("upload_count", uploadThumbnailCount);
        Mod::get()->setSavedValue<int>("accepted_upload_count",
            acceptUploadThumbnailCount);
        coinLabel->setCString(fmt::format("{}", activeThumbnailCount).c_str());
    });

    // Main buttons
    const float buttonSize = 75.f;
    const float spacing = 30.f;
    const float centerX = screenSize.width / 2.f;
    const float centerY = screenSize.height / 2.f;

    auto myThumbSprite = CCSprite::create("myThumbnailsButton.png"_spr);
    myThumbSprite->setScale(1.2f);
    auto myThumbBtn = CCMenuItemSpriteExtra::create(
        myThumbSprite, this, menu_selector(BetterThumbnailLayer::onMyThumbnail));

    recentSprite = CCSprite::create("recentlyAddedButton.png"_spr);
    recentSprite->setScale(1.2f);
    auto recentBtn = CCMenuItemSpriteExtra::create(
        recentSprite, this, menu_selector(BetterThumbnailLayer::onRecent));

    if (Mod::get()->getSavedValue<int>("role_num") >= 20) {
        pendingSprite = CCSprite::create("pendingButton.png"_spr);
    } else {
        pendingSprite = CCSpriteGrayscale::create("pendingButton.png"_spr);
    }

    pendingSprite->setScale(1.2f);
    auto pendingBtn = CCMenuItemSpriteExtra::create(
        pendingSprite, this, menu_selector(BetterThumbnailLayer::onPending));

    if (Mod::get()->getSavedValue<int>("role_num") >= 30) {
        manageSprite = CCSprite::create("manageUsersButton.png"_spr);
    } else {
        manageSprite = CCSpriteGrayscale::create("manageUsersButton.png"_spr);
    }
    if (manageSprite != nullptr) {
        manageSprite->setScale(1.2f);
        auto manageBtn = CCMenuItemSpriteExtra::create(
            manageSprite, this, menu_selector(BetterThumbnailLayer::onManage));
        manageBtn->setPosition({centerX + buttonSize / 2 + spacing / 2,
            centerY - buttonSize / 2 - spacing / 2});
        menu->addChild(manageBtn);
    }

    myThumbBtn->setPosition({centerX - buttonSize / 2 - spacing / 2,
        centerY + buttonSize / 2 + spacing / 2});
    recentBtn->setPosition({centerX + buttonSize / 2 + spacing / 2,
        centerY + buttonSize / 2 + spacing / 2});
    pendingBtn->setPosition({centerX - buttonSize / 2 - spacing / 2,
        centerY - buttonSize / 2 - spacing / 2});
    menu->addChild(myThumbBtn);
    menu->addChild(recentBtn);
    menu->addChild(pendingBtn);

    // funny side art
    auto sideArtLeft = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    auto sideArtRight = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    if (sideArtLeft) {
        sideArtLeft->setAnchorPoint({0.f, 0.f});
        sideArtLeft->setPosition({0.f, 0.f});
        this->addChild(sideArtLeft, 1);
    }
    if (sideArtRight) {
        sideArtRight->setAnchorPoint({1.f, 0.f});
        sideArtRight->setPosition({screenSize.width, 0.f});
        sideArtRight->setFlipX(true);
        this->addChild(sideArtRight, 1);
    }

    this->setKeypadEnabled(true);

    // fetch notifications for this user and show new ones
    this->fetchNotifications();

    return true;
}

void BetterThumbnailLayer::fetchNotifications() {
    auto userId = Mod::get()->getSavedValue<int>("user_id");
    if (userId <= 0) {
        return;
    }

    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));

    auto task = req.get(fmt::format("https://tjcsucht.net/api/bt/getnotif/{}", userId));
    async::spawn(std::move(task), [this](web::WebResponse res) {
        if (res.code() < 200 || res.code() > 299) {
            log::error("Notification API error {}: {}", res.code(), res.string().unwrapOrDefault());
            return;
        }
        auto jsonResult = res.json();
        if (!jsonResult.isOk()) {
            log::error("Notification API JSON parse error: {}",
                jsonResult.unwrapErr());
            return;
        }

        auto json = jsonResult.unwrap();
        if (!json.isObject() || !json["notifications"].isArray()) {
            log::error("Notification API invalid format: {}",
                res.string().unwrapOrDefault());
            return;
        }

        auto arr = json["notifications"].asArray().copied().unwrapOrDefault();
        int highestId = m_lastNotificationId;

        std::vector<NotificationMenuPopup::NotificationEntry> newNotifications;
        for (auto &item : arr) {
          auto itemId = item["id"].asInt().unwrapOrDefault();
          if (itemId <= 0 || itemId <= m_lastNotificationId) {
            continue;
          }

          auto title = item["title"].asString().unwrapOr("Notification");
          auto content = item["content"].asString().unwrapOr("New message");
          auto timestamp = item["timestamp"].asString().unwrapOr("unknown");
          newNotifications.push_back({title, content, timestamp});

            highestId = std::max(highestId, static_cast<int>(itemId));
        }

        if (!newNotifications.empty()) {
            std::string notifyTitle;
            std::string notifyMessage;

            if (newNotifications.size() == 1) {
                notifyTitle = newNotifications[0].title;
                notifyMessage = newNotifications[0].body;
            } else {
                notifyTitle = "Notifications";
                notifyMessage = fmt::format("You have {} notifications! Click view", newNotifications.size());
            }

            auto viewCallback = [this, notifications = std::move(newNotifications)]() mutable {
                auto popup = NotificationMenuPopup::create();
                if (!popup) {
                    Notification::create("Error opening notification list", NotificationIcon::Error)->show();
                    return;
                }
                popup->setNotifications(notifications, Mod::get()->getSavedValue<int>("user_id"));
                popup->show();
            };

            auto notifUI = NotificationNode::create(notifyTitle, notifyMessage, viewCallback);
            if (notifUI) {
                OverlayManager::get()->addChild(notifUI, 100);
            }
        }

        if (highestId > m_lastNotificationId) {
            m_lastNotificationId = highestId;
        }
    });
}

void BetterThumbnailLayer::onMyThumbnail(CCObject*) {
    // to do: my thumbnails
    FLAlertLayer::create("My Thumbnails", "This feature is not implemented yet.", "Ok")
        ->show();
}
void BetterThumbnailLayer::onRecent(CCObject*) {
    // to do: recent thumbnails
    FLAlertLayer::create("Recent Thumbnails",
        "This feature is not implemented yet.",
        "Ok")
        ->show();
}
void BetterThumbnailLayer::onPending(CCObject*) {
    if (Mod::get()->getSavedValue<int>("role_num") >= 20) {
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
    // to do: manage user
    if (Mod::get()->getSavedValue<int>("role_num") >= 30) {
        FLAlertLayer::create("Manage User", "This feature is not implemented yet.", "Ok")
            ->show();
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
    auto activeThumbnails =
        Mod::get()->getSavedValue<int>("active_thumbnail_count");
    auto userRankNum = Mod::get()->getSavedValue<int>("role_num");
    auto uploadThumbnailCount = Mod::get()->getSavedValue<int>("upload_count");
    auto acceptUploadThumbnailCount =
        Mod::get()->getSavedValue<int>("accepted_upload_count");
    auto infoString =
        fmt::format(
            "Rank: {}\nRank (numerical): {}\nUser ID: {}\nActive "
            "Thumbnails: {}\nUploaded Thumbnails: {}/{} accepted",
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