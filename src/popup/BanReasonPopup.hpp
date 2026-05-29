#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class BanReasonPopup : public Popup {
public:
    static BanReasonPopup* create(int userId);

protected:
    bool init(int userId);

private:
    TextInput* m_input = nullptr;
    int m_userId = 0;
    async::TaskHolder<web::WebResponse> m_listener;

    void onSend(CCObject*);
};
