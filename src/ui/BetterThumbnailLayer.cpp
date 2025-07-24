#include <Geode/Geode.hpp>
#include "BetterThumbnailLayer.hpp"
#include "Geode/loader/Log.hpp"
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
        this->addChild(bg);
    }

    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    
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
    
    // user account
    std::string username =  Mod::get()->getSavedValue<std::string>("username");
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

    // thumbnail coin counter
    auto coinLabel = CCLabelBMFont::create("0", "bigFont.fnt"); // 0 as a placeholder
    coinLabel->setAnchorPoint({0.f, 1.f});
    coinLabel->setScale(0.5f);
    coinLabel->setAlignment(kCCTextAlignmentRight);

    auto coinSprite = CCSprite::create("ThumbnailCoin.png"_spr);
    if (coinSprite) {
        coinSprite->setAnchorPoint({1.f, 1.f});
        coinSprite->setScale(0.65f);
    }


    auto loadingCircle = LoadingCircle::create();
    if (loadingCircle) {
        loadingCircle->setAnchorPoint({0.5f, 0.5f});
        loadingCircle->setPosition({menuWidth / 2.f, menuHeight / 2.f});
        loadingCircle->setID("user-info-loading-circle");
        userInfoMenu->addChild(loadingCircle, 100);
    }

    float padding = 3.f;
    float startX = menuWidth;
    float startY = menuHeight;

    userLabel->setPosition({startX, startY});
    userInfoMenu->addChild(userLabel);

    float userRankY = startY - userLabel->getContentSize().height * userLabel->getScale() - padding;
    userRankLabel->setPosition({startX, userRankY});
    userInfoMenu->addChild(userRankLabel);

    float coinLabelY = userRankY - userRankLabel->getContentSize().height * userRankLabel->getScale() - padding - 3.f;
    float coinLabelX = startX - 20.f;
    coinLabel->setPosition({coinLabelX, coinLabelY});
    userInfoMenu->addChild(coinLabel);

    if (coinSprite) {
        float coinSpriteX = coinLabelX - 5.f;
        coinSprite->setPosition({coinSpriteX, coinLabelY + 2.f});
        userInfoMenu->addChild(coinSprite);
    }


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

    m_listener.bind([this, coinLabel](web::WebTask::Event* e){
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
        }
    });

    req.header("Authorization",fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
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
    auto activeThumbnails = Mod::get()->getSavedValue<long>("active_thumbnail_count");
    auto infoString = fmt::format("Rank: {}\nUser ID: {}\nActive Thumbnails: {}", userRank, userId, activeThumbnails);
    FLAlertLayer::create(Mod::get()->getSavedValue<std::string>("username").c_str(), infoString, "Ok")->show();
}