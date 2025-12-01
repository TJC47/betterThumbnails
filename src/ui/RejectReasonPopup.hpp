#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <functional>

using namespace geode::prelude;

class RejectReasonPopup : public Popup<int, std::function<void(std::string)>> {
     public:
      static RejectReasonPopup* create(int thumbId, std::function<void(std::string)> onSend);

     protected:
      bool setup(int thumbId, std::function<void(std::string)> onSend) override;

     private:
      TextInput* m_input = nullptr;
      int m_id = 0;
      std::function<void(std::string)> m_callback;

      void onSend(CCObject*);
      void onCancel(CCObject*);
};
