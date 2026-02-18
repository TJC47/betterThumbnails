#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class FilterThumbnailPopup : public Popup {
     public:
      static FilterThumbnailPopup* create(geode::Function<void(std::string, bool, int)> onApply);

     protected:
      bool init(int, geode::Function<void(std::string, bool, int)> onApply);

     private:
      TextInput* m_usernameInput = nullptr;
      TextInput* m_levelIdInput = nullptr;
      geode::Function<void(std::string, bool, int)> m_callback;

      void onApply(CCObject*);
};
