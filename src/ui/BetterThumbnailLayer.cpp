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

    // Back button at top left
    auto backButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this,
        menu_selector(BetterThumbnailLayer::onBackButton));
    backButton->setPosition({25.f, screenSize.height - 25.f});
    menu->addChild(backButton);

    float buttonSize = 80.f;
    float spacing = 30.f;
    float centerX = screenSize.width / 2.f;
    float centerY = screenSize.height / 2.f;

    auto myThumbBtn = CCMenuItemSpriteExtra::create(
        CCSprite::create("myThumbnailsButton.png"_spr),
        this,
        menu_selector(BetterThumbnailLayer::onMyThumbnail));
    auto recentBtn = CCMenuItemSpriteExtra::create(
        CCSprite::create("recentlyAddedButton.png"_spr),
        this,
        menu_selector(BetterThumbnailLayer::onRecent));
    auto pendingBtn = CCMenuItemSpriteExtra::create(
        CCSprite::create("pendingButton.png"_spr),
        this,
        menu_selector(BetterThumbnailLayer::onPending));
    auto manageBtn = CCMenuItemSpriteExtra::create(
        CCSprite::create("manageUsersButton.png"_spr),
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