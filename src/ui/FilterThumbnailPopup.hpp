#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <functional>

using namespace geode::prelude;

class FilterThumbnailPopup : public Popup<int, std::function<void(std::string, bool, int)>> {
     public:
      static FilterThumbnailPopup* create(std::function<void(std::string, bool, int)> onApply);

     protected:
      bool setup(int, std::function<void(std::string, bool, int)> onApply) override;

     private:
      TextInput* m_usernameInput = nullptr;
      TextInput* m_levelIdInput = nullptr;
      std::function<void(std::string, bool, int)> m_callback;

      void onApply(CCObject*);
};
