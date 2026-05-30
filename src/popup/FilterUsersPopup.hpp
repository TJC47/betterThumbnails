#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class FilterUsersPopup : public Popup {
public:
    static FilterUsersPopup* create(geode::Function<void(std::string, std::string, std::string, std::string)> onApply);

protected:
    bool init(geode::Function<void(std::string, std::string, std::string, std::string)> onApply);

private:
    TextInput* m_usernameInput = nullptr;
    TextInput* m_userIdInput = nullptr;
    TextInput* m_accountIdInput = nullptr;
    TextInput* m_discordIdInput = nullptr;
    geode::Function<void(std::string, std::string, std::string, std::string)> m_callback;

    void onApply(CCObject*);
};
