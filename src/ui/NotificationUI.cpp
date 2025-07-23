#include <Geode/Geode.hpp>
#include "NotificationUI.hpp"
#include "BetterThumbnailLayer.hpp"

bool NotificationUI::init(const std::string &title, const std::string &message)
{
    if (!CCLayer::init())
        return false;

    float popupWidth = 300.f;
    float popupHeight = 80.f;

    auto bg = CCScale9Sprite::create("GJ_square05.png");
    bg->setContentSize({popupWidth, popupHeight});
    bg->setAnchorPoint({0.f, 0.f});
    this->addChild(bg, 0);

    // Title
    auto titleLabel = CCLabelBMFont::create(title.c_str(), "goldFont.fnt");
    titleLabel->setPosition({popupWidth / 2.f, popupHeight - 25.f});
    titleLabel->setScale(0.6f);
    this->addChild(titleLabel, 1);

    // Message
    auto messageLabel = CCLabelBMFont::create(message.c_str(), "bigFont.fnt");
    messageLabel->setPosition({popupWidth / 2.f, popupHeight / 2.f});
    messageLabel->setScale(0.5f);
    this->addChild(messageLabel, 1);

    this->setContentSize({popupWidth, popupHeight});
    this->setAnchorPoint({0.5f, 0.5f});

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    float centerX = winSize.width / 2.f - 149.f;
    float startY = winSize.height + popupHeight;
    float endY = winSize.height - popupHeight / 2.f - 50.f;
    this->setPosition({centerX, startY});
    this->setAnchorPoint({0.5f, 1.f});
    this->setScale(0.8f);

    // animation for the notification
    auto moveIn = CCEaseIn::create(CCMoveTo::create(0.5f, {centerX, endY}), 2.0f);
    auto delay = CCDelayTime::create(5.0f);
    auto moveOut = CCEaseOut::create(CCMoveTo::create(0.5f, {centerX, startY}), 2.0f);
    auto removeSelf = CCCallFunc::create(this, callfunc_selector(NotificationUI::removeFromParent));

    this->runAction(CCSequence::create(moveIn, delay, moveOut, removeSelf, nullptr));

    return true;
}

NotificationUI *NotificationUI::create(const std::string &title, const std::string &message)
{
    auto ret = new NotificationUI();
    if (ret && ret->init(title, message))
    {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}