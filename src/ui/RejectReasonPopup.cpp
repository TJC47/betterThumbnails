#include "RejectReasonPopup.hpp"

using namespace geode::prelude;

RejectReasonPopup* RejectReasonPopup::create(int thumbId, std::function<void(std::string)> onSend) {
      auto ret = new RejectReasonPopup;
      if (ret && ret->initAnchored(260.f, 120.f, thumbId, std::move(onSend), "GJ_square05.png")) {
            ret->autorelease();
            return ret;
      }
      CC_SAFE_DELETE(ret);
      return nullptr;
}

bool RejectReasonPopup::setup(int thumbId, std::function<void(std::string)> onSend) {
      m_id = thumbId;
      m_callback = std::move(onSend);
      setTitle("Reject Reason");

      m_input = TextInput::create(220.f, "Reason for rejection");
      m_input->setPosition({m_mainLayer->getContentSize().width / 2.f, m_mainLayer->getContentSize().height / 2.f});
      m_mainLayer->addChild(m_input);

      auto sendSpr = ButtonSprite::create("Reject", "goldFont.fnt", "GJ_button_06.png", 1.f);
      auto sendBtn = CCMenuItemSpriteExtra::create(sendSpr, this, menu_selector(RejectReasonPopup::onSend));
      sendBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 0.f});

      auto menu = CCMenu::create();
      menu->addChild(sendBtn);
      menu->setPosition({0.f, 0.f});
      m_mainLayer->addChild(menu);

      return true;
}

void RejectReasonPopup::onSend(CCObject*) {
      std::string reason = m_input ? m_input->getString() : "";
      if (m_callback)
            m_callback(reason);
      this->onClose(nullptr);
}