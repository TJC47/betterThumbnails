#include "FilterThumbnailPopup.hpp"

#include <charconv>

#include "PendingThumbnailLayer.hpp"

using namespace geode::prelude;

FilterThumbnailPopup* FilterThumbnailPopup::create(std::function<void(std::string, bool, int)> onApply) {
      auto ret = new FilterThumbnailPopup;
      if (ret && ret->initAnchored(300.f, 160.f, 0, std::move(onApply), "GJ_square02.png")) {
            ret->autorelease();
            return ret;
      }
      CC_SAFE_DELETE(ret);
      return nullptr;
}

bool FilterThumbnailPopup::setup(int, std::function<void(std::string, bool, int)> onApply) {
      m_callback = std::move(onApply);
      setTitle("Filter Thumbnails");

      m_usernameInput = TextInput::create(240.f, "Search by username");
      m_usernameInput->setPosition({m_mainLayer->getContentSize().width / 2.f, m_mainLayer->getContentSize().height - 60.f});
      m_mainLayer->addChild(m_usernameInput);

      m_levelIdInput = TextInput::create(240.f, "Search by level ID");
      m_levelIdInput->setPosition({m_mainLayer->getContentSize().width / 2.f, m_mainLayer->getContentSize().height - 110.f});
      m_mainLayer->addChild(m_levelIdInput);

      auto applySpr = ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", 1.f);
      auto applyBtn = CCMenuItemSpriteExtra::create(applySpr, this, menu_selector(FilterThumbnailPopup::onApply));
      applyBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 0.f});

      auto menu = CCMenu::create();
      menu->addChild(applyBtn);
      menu->setPosition({0.f, 0.f});
      m_mainLayer->addChild(menu);

      return true;
}

void FilterThumbnailPopup::onApply(CCObject*) {
      std::string username = m_usernameInput ? m_usernameInput->getString() : std::string();
      std::string levelStr = m_levelIdInput ? m_levelIdInput->getString() : std::string();
      bool hasLevelId = false;
      int levelId = 0;
      if (!levelStr.empty()) {
            int parsed = 0;
            const char* begin = levelStr.data();
            const char* end = levelStr.data() + levelStr.size();
            auto res = std::from_chars(begin, end, parsed);
            if (res.ec == std::errc() && res.ptr == end) {
                  levelId = parsed;
                  hasLevelId = true;
            } else {
                  hasLevelId = false;
            }
      }
      if (m_callback) {
            m_callback(username, hasLevelId, levelId);
      }
      this->onClose(nullptr);
}
