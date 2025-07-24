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
    delete ret;
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
    this->addChild(menu);
    menu->setPosition({0.f, 0.f});

    // user account
    std::string username = GJAccountManager::sharedState()->m_username;
    auto userLabel = CCLabelBMFont::create(username.c_str(), "goldFont.fnt");
    userLabel->setAnchorPoint({1.f, 1.f});
    userLabel->setScale(0.5f);
    userLabel->setAlignment(kCCTextAlignmentRight);
    float userLabelX = screenSize.width - 5.f;
    float userLabelY = screenSize.height - 5.f;
    userLabel->setPosition({userLabelX, userLabelY});
    this->addChild(userLabel, 10);

    // thumbnail coin counter
    float coinLabelY = userLabelY - 25.f;

    auto coinLabel = CCLabelBMFont::create("0", "bigFont.fnt"); // 0 as a placeholder
    coinLabel->setAnchorPoint({0.f, 1.f});
    coinLabel->setScale(0.5f);
    coinLabel->setAlignment(kCCTextAlignmentRight);

    float coinLabelX = userLabelX - 20.f;
    coinLabel->setPosition({coinLabelX, coinLabelY});
    this->addChild(coinLabel, 10);

    auto coinSprite = CCSprite::create("ThumbnailCoin.png"_spr);
    if (coinSprite) {
        coinSprite->setAnchorPoint({1.f, 1.f});
        coinSprite->setScale(0.65f);
        float labelWidth = coinLabel->getContentSize().width * coinLabel->getScale();
        float coinSpriteX = coinLabelX - 5.f;
        coinSpriteX = coinLabelX - 5.f;
        coinSprite->setPosition({coinSpriteX, coinLabelY + 2.f});
        this->addChild(coinSprite, 10);
    }

    // Back button at top left
    auto backButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this,
        menu_selector(BetterThumbnailLayer::onBackButton));
    backButton->setPosition({25.f, screenSize.height - 25.f});
    menu->addChild(backButton);

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