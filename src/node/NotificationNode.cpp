#include <Geode/Geode.hpp>
#include "NotificationNode.hpp"

using namespace geode::prelude;

bool NotificationNode::init(const std::string& title, const std::string& message, const std::string& type, Callback viewCallback) {
    if (!CCLayer::init())
        return false;

    float popupWidth = 300.f;
    float popupHeight = 100.f;

    auto bg = NineSlice::create("GJ_square05.png");
    bg->setContentSize({popupWidth, popupHeight});
    bg->setAnchorPoint({0.f, 0.f});
    this->addChild(bg, 0);

    // Title
    const char* iconName =
        type == "success" ? "GJ_completesIcon_001.png" :
        // @geode-ignore(unknown-resource)
        type == "warn" ? "geode.loader/info-warning.png" :
        type == "error" ? "GJ_deleteIcon_001.png" :
        // @geode-ignore(unknown-resource)
        type == "critical" ? "geode.loader/info-alert.png" :
        nullptr;

    CCSprite* icon = nullptr;
    if (iconName) {
        auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(iconName);
        if (frame) {
            icon = CCSprite::createWithSpriteFrameName(iconName);
        } else {
            icon = CCSprite::create(iconName);
        }
    }

    float textOffset = 10.f;
    if (icon) {
        icon->setAnchorPoint({0.f, 0.5f});
        icon->setPosition({10.f, popupHeight - 22.f});
        icon->setScale(0.8f);
        this->addChild(icon, 1);
        textOffset += 26.f;
    }

    auto titleLabel = CCLabelBMFont::create(title.c_str(), "goldFont.fnt");
    titleLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    titleLabel->setPosition({textOffset, popupHeight - 5.f});
    titleLabel->setAnchorPoint({0.f, 1.f});
    titleLabel->limitLabelWidth(popupWidth - textOffset - 10.f, 1.f, 0.5f);
    this->addChild(titleLabel, 1);

    // Message
    auto messageLabel = SimpleTextArea::create(message.c_str(), "bigFont.fnt", 0.5f, popupWidth - 25.f);
    messageLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    messageLabel->setAnchorPoint({0.f, 1.f});
    messageLabel->setMaxLines(4);
    messageLabel->setScale(0.3f);
    messageLabel->setPosition({10, popupHeight - 30.f});
    this->addChild(messageLabel, 1);

    m_viewCallback = std::move(viewCallback);

    if (m_viewCallback) {
        auto viewBtnSprite = ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png", 1.f);
        viewBtnSprite->setScale(0.6f);
        auto viewBtn = CCMenuItemSpriteExtra::create(viewBtnSprite, this, menu_selector(NotificationNode::onViewButton));
        viewBtn->setPosition({popupWidth - 30, 20.f});
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

NotificationNode* NotificationNode::create(const std::string& title, const std::string& message, const std::string& type, Callback viewCallback) {
    auto ret = new NotificationNode();
    if (ret && ret->init(title, message, type, viewCallback)) {
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