#include "BetterThumbnailLayer.hpp"

BetterThumbnailLayer *BetterThumbnailLayer::create() {
    auto ret = new BetterThumbnailLayer;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene *BetterThumbnailLayer::scene() {
    auto scene = CCScene::create();
    scene->addChild(BetterThumbnailLayer::create());
    return scene;
}

bool BetterThumbnailLayer::init() {
    if (!CCLayer::init())
        return false;

    CCSize screenSize = CCDirector::sharedDirector()->getWinSize();

    addChild(createLayerBG());
    auto menu = CCMenu::create();
    addChild(menu);
    auto backButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
        this,
        menu_selector(BetterThumbnailLayer::onBackButton));
    backButton->setPosition({30.f, screenSize.height - 30.f});
    menu->setPosition({0.f, 0.f});
    menu->addChild(backButton);

    auto banner = LazySprite::create({512.f, 256.f});
    banner->setPosition({screenSize.width / 2.f, screenSize.height * 0.85f});
    addChild(banner);

    auto info = CCLabelBMFont::create("loading...", "bigFont.fnt");
    info->setPosition({screenSize});
    info->setAnchorPoint({1.f, 1.f});
    info->setScale(0.5f);
    addChild(info);

    // Setup all request logic using ThumbRequest
    thumbRequest.setupRequests(info, banner, screenSize);
    thumbRequest.startMetaRequest();

    return true;
}

void BetterThumbnailLayer::onBackButton(CCObject *) {
    CCDirector::get()->pushScene(CCTransitionFade::create(.5f, CreatorLayer::scene()));
}
