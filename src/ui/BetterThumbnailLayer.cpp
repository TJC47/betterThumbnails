#include <Geode/Geode.hpp>
#include "BetterThumbnailLayer.hpp"
#include "NotificationUI.hpp"

CCScene *BetterThumbnailLayer::scene()
{
    auto scene = CCScene::create();
    scene->addChild(BetterThumbnailLayer::create());
    return scene;
}

BetterThumbnailLayer *BetterThumbnailLayer::create()
{
    auto ret = new BetterThumbnailLayer;
    if (ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BetterThumbnailLayer::init()
{
    if (!CCLayer::init())
        return false;

    auto bg = createLayerBG();

    if (bg)
    {
        this->addChild(bg, -1);
    }

    auto screenSize = CCDirector::sharedDirector()->getWinSize();

    auto bgImage = LazySprite::create(bg->getScaledContentSize(), true);
    bgImage->setID("better-thumbnail-bg");
    bgImage->setPosition({0.f, 0.f});

    bgImage->setLoadCallback([this, screenSize, bg, bgImage](geode::Result<void, std::string> result)
                             {
        if (result.isOk() || bgImage->isLoaded())
        {
            log::debug("Thumbnail loaded, fading out background");
            bgImage->setAutoResize(true);
            bgImage->setAnchorPoint({0.f, 0.f});

            auto bgDark = CCScale9Sprite::create("square02_001.png");
            bgDark->setContentSize(screenSize);
            bgDark->setOpacity(100);
            bgDark->setAnchorPoint({0.f, 0.f});
            this->addChild(bgDark, -2);

            bg->runAction(CCFadeTo::create(1.f, 0));
        }
        else
        {
            log::error("Failed to load thumbnail: {}", result.unwrapErr());
        }
    });
    
    // please laugh, lazysprite no supports webp
    bgImage->loadFromUrl("https://levelthumbs.prevter.me/thumbnail/random", LazySprite::Format::kFmtUnKnown, false);
    this->addChild(bgImage, -3);

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

    float menuWidth = userInfoMenu->getContentSize().width;
    float menuHeight = userInfoMenu->getContentSize().height;

    float padding = 3.f;
    float startX = menuWidth;
    float startY = menuHeight;

    // user account
    std::string username = GJAccountManager::sharedState()->m_username;
    auto userLabel = CCLabelBMFont::create(username.c_str(), "goldFont.fnt");
    userLabel->setAnchorPoint({1.f, 1.f});
    userLabel->setScale(0.5f);
    userLabel->setAlignment(kCCTextAlignmentRight);

    // user rank
    std::string userRank = Mod::get()->getSavedValue<std::string>("role");
    auto userRankLabel = CCLabelBMFont::create(("(" + userRank + ")").c_str(), "goldFont.fnt");
    userRankLabel->setAnchorPoint({1.f, 1.f});
    userRankLabel->setScale(0.3f);
    userRankLabel->setAlignment(kCCTextAlignmentRight);

    userLabel->setPosition({startX, startY});
    userInfoMenu->addChild(userLabel);

    float userRankY = startY - userLabel->getContentSize().height * userLabel->getScale() - padding;
    userRankLabel->setPosition({startX, userRankY});
    userInfoMenu->addChild(userRankLabel);

    // thumbnail coin counter
    auto coinLabel = CCLabelBMFont::create("-", "bigFont.fnt"); // - as a placeholder
    coinLabel->setAnchorPoint({1.f, 1.f});
    coinLabel->setScale(0.5f);
    coinLabel->setAlignment(kCCTextAlignmentRight);

    auto coinSprite = CCSprite::create("ThumbnailCoin.png"_spr);
    coinSprite->setAnchorPoint({1.f, 1.f});
    coinSprite->setScale(0.65f);

    float coinLabelY = userRankY - userRankLabel->getContentSize().height * userRankLabel->getScale() - padding - 3.f;
    float coinLabelX = startX - 25.f;

    coinSprite->setPosition({startX, coinLabelY + 2.f});
    userInfoMenu->addChild(coinSprite);

    coinLabel->setPosition({coinLabelX, coinLabelY});
    userInfoMenu->addChild(coinLabel);

    // Back button at top left
    auto backButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this,
        menu_selector(BetterThumbnailLayer::onBackButton));
    backButton->setPosition({25.f, screenSize.height - 25.f});
    menu->addChild(backButton);

    // info button
    auto infoButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"),
        this,
        menu_selector(BetterThumbnailLayer::onInfoButton));
    infoButton->setPosition({25.f, 25.f});
    menu->addChild(infoButton);

    // get user info
    auto req = web::WebRequest();

    m_listener.bind([this, coinLabel](web::WebTask::Event *e)
                    {
        if (auto res = e->getValue()){
            auto code = res->code();
            if (code<200||code>299){
                auto error = res->string().unwrapOr(res->errorMessage());
                FLAlertLayer::create("Oops",error,"OK")->show();
                delete this;
                return;
            }
            geode::log::info("{} {}",res->code(),res->string().unwrapOrDefault());
            auto json = res->json().unwrapOrDefault();
			log::info("{} {}",res->code(),json.dump());
			auto activeThumbnailCount = json["data"]["active_thumbnail_count"].asInt().unwrapOrDefault();
            log::debug("{}", activeThumbnailCount);
			Mod::get()->setSavedValue<long>("active_thumbnail_count", activeThumbnailCount);
            coinLabel->setCString(fmt::format("{}", activeThumbnailCount).c_str());
        } });

    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
    auto task = req.get(fmt::format("https://levelthumbs.prevter.me/user/me"));
    m_listener.setFilter(task);

    // Main buttons
    float buttonSize = 75.f;
    float spacing = 30.f;
    float centerX = screenSize.width / 2.f;
    float centerY = screenSize.height / 2.f;

    auto myThumbSprite = CCSprite::create("myThumbnailsButton.png"_spr);
    myThumbSprite->setScale(1.2f);
    auto myThumbBtn = CCMenuItemSpriteExtra::create(
        myThumbSprite,
        this,
        menu_selector(BetterThumbnailLayer::onMyThumbnail));

    auto recentSprite = CCSprite::create("recentlyAddedButton.png"_spr);
    recentSprite->setScale(1.2f);
    auto recentBtn = CCMenuItemSpriteExtra::create(
        recentSprite,
        this,
        menu_selector(BetterThumbnailLayer::onRecent));

    auto pendingSprite = CCSprite::create("pendingButton.png"_spr);
    pendingSprite->setScale(1.2f);
    auto pendingBtn = CCMenuItemSpriteExtra::create(
        pendingSprite,
        this,
        menu_selector(BetterThumbnailLayer::onPending));

    auto manageSprite = CCSprite::create("manageUsersButton.png"_spr);
    manageSprite->setScale(1.2f);
    auto manageBtn = CCMenuItemSpriteExtra::create(
        manageSprite,
        this,
        menu_selector(BetterThumbnailLayer::onManage));

    myThumbBtn->setPosition({centerX - buttonSize / 2 - spacing / 2, centerY + buttonSize / 2 + spacing / 2});
    recentBtn->setPosition({centerX + buttonSize / 2 + spacing / 2, centerY + buttonSize / 2 + spacing / 2});
    pendingBtn->setPosition({centerX - buttonSize / 2 - spacing / 2, centerY - buttonSize / 2 - spacing / 2});
    manageBtn->setPosition({centerX + buttonSize / 2 + spacing / 2, centerY - buttonSize / 2 - spacing / 2});

    menu->addChild(myThumbBtn);
    menu->addChild(recentBtn);
    menu->addChild(pendingBtn);
    menu->addChild(manageBtn);

    // funny side art
    auto sideArtLeft = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    auto sideArtRight = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    if (sideArtLeft)
    {
        sideArtLeft->setAnchorPoint({0.f, 0.f});
        sideArtLeft->setPosition({0.f, 0.f});
        this->addChild(sideArtLeft, 1);
    }
    if (sideArtRight)
    {
        sideArtRight->setAnchorPoint({1.f, 0.f});
        sideArtRight->setPosition({screenSize.width, 0.f});
        sideArtRight->setFlipX(true);
        this->addChild(sideArtRight, 1);
    }

    this->setKeypadEnabled(true);

    // notification test (do this way if you want to use notifications)
    auto notif = NotificationUI::create("insert title", "hi there");
    if (notif)
    {
        this->addChild(notif, 100);
    }

    return true;
}

void BetterThumbnailLayer::onMyThumbnail(CCObject *)
{
    // to do: my thumbnails
    FLAlertLayer::create("My Thumbnails", "This feature is not implemented yet.", "Ok")->show();
}
void BetterThumbnailLayer::onRecent(CCObject *)
{
    // to do: recent thumbnails
    FLAlertLayer::create("Recent Thumbnails", "This feature is not implemented yet.", "Ok")->show();
}
void BetterThumbnailLayer::onPending(CCObject *)
{
    // to do: pending thumbnail
    FLAlertLayer::create("Pending Thumbnails", "This feature is not implemented yet.", "Ok")->show();
}
void BetterThumbnailLayer::onManage(CCObject *)
{
    // to do: manage user
    FLAlertLayer::create("Manage User", "This feature is not implemented yet.", "Ok")->show();
}

void BetterThumbnailLayer::keyBackClicked()
{
    onBackButton(nullptr);
}

void BetterThumbnailLayer::onBackButton(CCObject *)
{
    CCDirector::get()->pushScene(CCTransitionFade::create(.5f, CreatorLayer::scene()));
}
void BetterThumbnailLayer::onInfoButton(CCObject *)
{
    std::string userRank = Mod::get()->getSavedValue<std::string>("role");
    auto userId = Mod::get()->getSavedValue<long>("user_id");
    auto infoString = fmt::format("Rank: {}\nUser ID: {}", userRank, userId);
    FLAlertLayer::create(GJAccountManager::get()->m_username.c_str(), infoString, "Ok")->show();
}