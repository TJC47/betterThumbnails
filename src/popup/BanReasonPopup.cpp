#include "../popup/BanReasonPopup.hpp"
#include "../include/BetterThumbnailConstant.hpp"

using namespace geode::prelude;

BanReasonPopup* BanReasonPopup::create(int userId) {
    auto ret = new BanReasonPopup;
    if (ret && ret->init(userId)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BanReasonPopup::init(int userId) {
    if (!Popup::init(260.f, 140.f, "GJ_square05.png"))
        return false;

    m_userId = userId;
    setTitle(fmt::format("Ban User ID: {}", userId));

    m_input = TextInput::create(220.f, "Reason for ban");
    m_input->setLabel("Ban Reason");
    m_input->setPosition({m_mainLayer->getContentSize().width / 2.f, m_mainLayer->getContentSize().height / 2.f + 5.f});
    m_mainLayer->addChild(m_input);

    auto sendSpr = ButtonSprite::create("Ban", "goldFont.fnt", "GJ_button_06.png", 1.f);
    auto sendBtn = CCMenuItemSpriteExtra::create(sendSpr, this, menu_selector(BanReasonPopup::onSend));
    sendBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 25.f});
    m_buttonMenu->addChild(sendBtn);

    return true;
}

void BanReasonPopup::onSend(CCObject*) {
    createQuickPopup("Ban user?", fmt::format("Are you sure you want to ban this user <cc>{}</c>?", m_userId), "Cancel", "Ban", [this](auto, bool yes) {
        if (!yes) return;
        std::string reason = m_input ? m_input->getString() : "";
        auto popup = UploadActionPopup::create(nullptr, fmt::format("Banning user {}...", m_userId));
        if (popup)
            popup->show();

        Ref<UploadActionPopup> popupRef = popup;

        auto jsonBody = matjson::makeObject({
            {"reason", reason},
            {"expires_by", nullptr},
        });
        auto req = betterThumbnail::createWebRequest();
        req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
        req.header("Content-Type", "application/json");
        req.bodyJSON(jsonBody);

        auto url = fmt::format("https://levelthumbs.prevter.me/admin/ban/{}", m_userId);
        auto task = req.send("POST", url);
        this->m_listener.spawn(std::move(task), [this, popupRef](web::WebResponse res) {
            if (!popupRef) return;
            if (res.code() == 200) {
                if (popupRef) popupRef->showSuccessMessage("User banned successfully");
                this->onClose(nullptr);
            } else {
                if (popupRef) popupRef->showFailMessage(fmt::format("Ban failed: {}", res.string().unwrapOr("Unknown error")));
            }
        });
    });
}
