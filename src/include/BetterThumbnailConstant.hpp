#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

#include <string>
#include <string_view>

using namespace geode::prelude;

namespace betterThumbnail {

    enum class RoleNum : int {
        User = 0,
        Verified = 10,
        Moderator = 20,
        Admin = 30,
        Owner = 40,
        Unknown = -1,
    };

    inline std::string baseUrl = "https://levelthumbs.prevter.me";

    inline int getRoleNum() {
        return Mod::get()->getSavedValue<int>("role_num");
    }

    inline bool hasRoleAtLeast(RoleNum role) {
        return getRoleNum() >= static_cast<int>(role);
    }

    inline std::string makeUrl(std::string_view path) {
        return baseUrl + std::string(path);
    }

    inline geode::utils::web::WebRequest createWebRequest() {
        return geode::utils::web::WebRequest().userAgent(fmt::format(
            "betterThumbnails/{}/{}",
            geode::PlatformID::toShortString(GEODE_PLATFORM_TARGET),
            geode::Mod::get()->getVersion().toVString()));
    }

}  // namespace betterThumbnail