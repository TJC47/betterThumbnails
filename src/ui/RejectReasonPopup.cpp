#include "RejectReasonPopup.hpp"

using namespace geode::prelude;

RejectReasonPopup* RejectReasonPopup::create(int thumbId, geode::Function<void(std::string)> onSend) {
      auto ret = new RejectReasonPopup;
      if (ret && ret->init(thumbId, std::move(onSend))) {
            ret->autorelease();
            return ret;
      }
      delete ret;
      return nullptr;
}

bool RejectReasonPopup::init(int thumbId, geode::Function<void(std::string)> onSend) {
      if (!Popup::init(260.f, 120.f, "GJ_square05.png"))
            return false;

      m_id = thumbId;
      m_callback = std::move(onSend);
      setTitle("Reject Reason");

      m_input = TextInput::create(220.f, "Reason for rejection");
      m_input->setPosition({m_mainLayer->getContentSize().width / 2.f, m_mainLayer->getContentSize().height / 2.f + 5.f});
      m_mainLayer->addChild(m_input);

      auto sendSpr = ButtonSprite::create("Reject", "goldFont.fnt", "GJ_button_06.png", 1.f);
      auto sendBtn = CCMenuItemSpriteExtra::create(sendSpr, this, menu_selector(RejectReasonPopup::onSend));
      sendBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 25.f});
      m_buttonMenu->addChild(sendBtn);

      return true;
}

void RejectReasonPopup::onSend(CCObject*) {
      std::string reason = m_input ? m_input->getString() : "";
      if (m_callback)
            m_callback(reason);
      this->onClose(nullptr);
}