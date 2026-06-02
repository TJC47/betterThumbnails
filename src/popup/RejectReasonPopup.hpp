#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cue/ListNode.hpp>

using namespace geode::prelude;

class RejectReasonPopup : public Popup {
     public:
      static RejectReasonPopup* create(int thumbId, geode::Function<void(std::string)> onSend);

     protected:
      bool init(int thumbId, geode::Function<void(std::string)> onSend);

     private:
      TextInput* m_input = nullptr;
      cue::ListNode* m_reasonList = nullptr;
      int m_id = 0;
      geode::Function<void(std::string)> m_callback;

      void onSend(CCObject*);
};
