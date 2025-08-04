#include "PendingThumbnailLayer.hpp"
#include "BetterThumbnailLayer.hpp"

CCScene *PendingThumbnailLayer::scene()
{
    auto scene = CCScene::create();
    scene->addChild(PendingThumbnailLayer::create());
    return scene;
}

bool PendingThumbnailLayer::init()
{
    if (!CCLayer::init())
        return false;

    auto bg = createLayerBG();
    if (bg != nullptr)
    {
        this->addChild(bg, -1);
    }
    else
    {
        log::error("createLayerBG returned nullptr");
        return false;
    }

    auto screenSize = CCDirector::sharedDirector()->getWinSize();

    auto menu = CCMenu::create();
    this->addChild(menu, 2);
    menu->setPosition(CCPointZero);

    // Back button at top left
    auto backButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this,
        menu_selector(PendingThumbnailLayer::onBackButton));
    backButton->setPosition(CCPoint(25.f, screenSize.height - 25.f));
    menu->addChild(backButton);

    this->setKeypadEnabled(true);

    return true;
}

PendingThumbnailLayer *PendingThumbnailLayer::create()
{
    auto ret = new PendingThumbnailLayer;
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void PendingThumbnailLayer::keyBackClicked()
{
    onBackButton(nullptr);
}

void PendingThumbnailLayer::onBackButton(CCObject *)
{
    CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
}
