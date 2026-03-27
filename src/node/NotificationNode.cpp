#include <Geode/Geode.hpp>
#include "NotificationNode.hpp"

using namespace geode::prelude;

bool NotificationNode::init(const std::string& title, const std::string& message, Callback viewCallback) {
    if (!CCLayer::init())
        return false;

    float popupWidth = 300.f;
    float popupHeight = 100.f;

    auto bg = NineSlice::create("GJ_square05.png");
    bg->setContentSize({popupWidth, popupHeight});
    bg->setAnchorPoint({0.f, 0.f});
    this->addChild(bg, 0);

    // Title
    auto titleLabel = CCLabelBMFont::create(title.c_str(), "goldFont.fnt");
    titleLabel->setPosition({popupWidth / 2.f, popupHeight - 18.f});
    titleLabel->limitLabelWidth(popupWidth - 20, 1.f, 0.5f);
    this->addChild(titleLabel, 1);

    // Message
    auto messageLabel = CCLabelBMFont::create(message.c_str(), "bigFont.fnt");
    messageLabel->setPosition({popupWidth / 2.f, popupHeight / 2.f + 5.f});
    messageLabel->limitLabelWidth(popupWidth - 20, 0.5f, 0.3f);
    this->addChild(messageLabel, 1);

    m_viewCallback = std::move(viewCallback);

    if (m_viewCallback) {
        auto viewBtnSprite = ButtonSprite::create("View", "bigFont.fnt", "GJ_button_01.png", 0.6f);
        viewBtnSprite->setScale(0.6f);
        auto viewBtn = CCMenuItemSpriteExtra::create(viewBtnSprite, this, menu_selector(NotificationNode::onViewButton));
        viewBtn->setPosition({popupWidth / 2.f, 20.f});
        auto menu = CCMenu::create();
        menu->addChild(viewBtn);
        menu->setPosition({0.f, 0.f});
        this->addChild(menu, 2);
    }

    this->setContentSize({popupWidth, popupHeight});
    this->setAnchorPoint({0.5f, 0.5f});

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    float centerX = winSize.width / 2.f - popupWidth / 2.f;
    float startY = winSize.height + popupHeight;
    float endY = winSize.height - popupHeight / 2.f - 60.f;
    this->setPosition({centerX, startY});
    this->setAnchorPoint({0.5f, 1.f});
    this->setScale(0.8f);

    // animation for the notification
    auto moveIn = CCEaseIn::create(CCMoveTo::create(1.f, {centerX, endY}), 2.0f);
    auto delay = CCDelayTime::create(8.0f);
    auto moveOut = CCEaseOut::create(CCMoveTo::create(1.f, {centerX, startY}), 2.0f);
    auto removeSelf = CCCallFunc::create(this, callfunc_selector(NotificationNode::removeFromParent));

    this->runAction(CCSequence::create(moveIn, delay, moveOut, removeSelf, nullptr));
    // @geode-ignore(unknown-resource)
    FMODAudioEngine::sharedEngine()->playEffect("crystal01.ogg");

    return true;
}

NotificationNode* NotificationNode::create(const std::string& title, const std::string& message, Callback viewCallback) {
    auto ret = new NotificationNode();
    if (ret && ret->init(title, message, viewCallback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void NotificationNode::onViewButton(CCObject*) {
    if (m_viewCallback)
        m_viewCallback();
    removeFromParent();
}