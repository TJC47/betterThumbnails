#include "FilterUsersPopup.hpp"

using namespace geode::prelude;

FilterUsersPopup* FilterUsersPopup::create(geode::Function<void(std::string, std::string, std::string, std::string)> onApply) {
    auto ret = new FilterUsersPopup;
    if (ret && ret->init(std::move(onApply))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool FilterUsersPopup::init(geode::Function<void(std::string, std::string, std::string, std::string)> onApply) {
    if (!Popup::init(340.f, 280.f))
        return false;

    m_callback = std::move(onApply);
    setTitle("Filter Users");

    const float inputWidth = 260.f;
    const float startY = m_mainLayer->getContentSize().height - 60.f;
    const float rowHeight = 50.f;

    m_usernameInput = TextInput::create(inputWidth, "Username");
    m_usernameInput->setLabel("Username");
    m_usernameInput->setPosition({m_mainLayer->getContentSize().width / 2.f, startY});
    m_mainLayer->addChild(m_usernameInput);

    m_userIdInput = TextInput::create(inputWidth, "User ID");
    m_userIdInput->setLabel("User ID");
    m_userIdInput->setPosition({m_mainLayer->getContentSize().width / 2.f, startY - rowHeight});
    m_mainLayer->addChild(m_userIdInput);

    m_accountIdInput = TextInput::create(inputWidth, "GD Account ID");
    m_accountIdInput->setLabel("GD Account ID");
    m_accountIdInput->setPosition({m_mainLayer->getContentSize().width / 2.f, startY - rowHeight * 2.f});
    m_mainLayer->addChild(m_accountIdInput);

    m_discordIdInput = TextInput::create(inputWidth, "Discord ID");
    m_discordIdInput->setLabel("Discord ID");
    m_discordIdInput->setPosition({m_mainLayer->getContentSize().width / 2.f, startY - rowHeight * 3.f});
    m_mainLayer->addChild(m_discordIdInput);

    auto applySpr = ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", 1.f);
    auto applyBtn = CCMenuItemSpriteExtra::create(applySpr, this, menu_selector(FilterUsersPopup::onApply));
    applyBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 30.f});
    m_buttonMenu->addChildAtPosition(applyBtn, Anchor::Bottom, {0.f, 23.f}, false);

    return true;
}

void FilterUsersPopup::onApply(CCObject*) {
    std::string username = m_usernameInput ? std::string(m_usernameInput->getString()) : std::string();
    std::string userId = m_userIdInput ? std::string(m_userIdInput->getString()) : std::string();
    std::string accountId = m_accountIdInput ? std::string(m_accountIdInput->getString()) : std::string();
    std::string discordId = m_discordIdInput ? std::string(m_discordIdInput->getString()) : std::string();

    if (m_callback) {
        m_callback(std::move(username), std::move(userId), std::move(accountId), std::move(discordId));
    }
    this->onClose(nullptr);
}
