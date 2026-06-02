#include "NotificationOverlay.hpp"
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/OverlayManager.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/Geode.hpp>
#include <charconv>
#include <sstream>
#include "../layer/BetterThumbnailLayer.hpp"
#include "../node/NotificationNode.hpp"
#include "../popup/NotificationMenuPopup.hpp"

using namespace geode::prelude;

static bool isInPlayLayer();
static bool isOnBetterThumbnailLayer();

namespace {
    NotificationOverlay*& overlayInstance() {
        static NotificationOverlay* instance = nullptr;
        return instance;
    }
}

NotificationOverlay* NotificationOverlay::create() {
    auto ret = new NotificationOverlay();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NotificationOverlay::init() {
    if (!CCLayer::init()) {
        return false;
    }
    overlayInstance() = this;
    this->setVisible(true);
    return true;
}

void NotificationOverlay::onEnter() {
    CCLayer::onEnter();
    this->loadReadNotificationIds();
    this->fetchNotifications();
    this->schedule(schedule_selector(NotificationOverlay::pollNotifications), m_pollInterval);
}

void NotificationOverlay::onExit() {
    this->unschedule(schedule_selector(NotificationOverlay::pollNotifications));
    CCLayer::onExit();
}

void NotificationOverlay::pollNotifications(float) {
    this->fetchNotifications();
}

void NotificationOverlay::fetchNotifications() {
    auto userId = Mod::get()->getSavedValue<int>("user_id");
    if (userId <= 0) {
        return;
    }
    if (m_isNotificationFetchInProgress) {
        return;
    }

    m_isNotificationFetchInProgress = true;
    auto req = web::WebRequest();
    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
    auto task = req.get(fmt::format("https://tjcsucht.net/api/bt/getnotif/{}", userId));
    m_notificationListener.spawn(std::move(task), [this](web::WebResponse res) {
        this->processNotificationResponse(std::move(res));
    });
}

void NotificationOverlay::processNotificationResponse(web::WebResponse res) {
    if (res.code() < 200 || res.code() > 299) {
        log::error("Notification API error {}: {}", res.code(), res.string().unwrapOrDefault());
        m_isNotificationFetchInProgress = false;
        return;
    }

    auto jsonResult = res.json();
    if (!jsonResult.isOk()) {
        log::error("Notification API JSON parse error: {}", jsonResult.unwrapErr());
        m_isNotificationFetchInProgress = false;
        return;
    }

    auto json = jsonResult.unwrap();
    if (!json.isObject() || !json["notifications"].isArray()) {
        log::error("Notification API invalid format: {}", res.string().unwrapOrDefault());
        m_isNotificationFetchInProgress = false;
        return;
    }

    int interval = json["interval"].asInt().unwrapOr(static_cast<int>(m_pollInterval));
    if (interval > 0 && static_cast<float>(interval) != m_pollInterval) {
        this->updatePollingSchedule(static_cast<float>(interval));
    }

    auto arr = json["notifications"].asArray().copied().unwrapOrDefault();
    long long highestTimestamp = m_lastNotificationTimestamp;
    std::vector<NotificationMenuPopup::NotificationEntry> newNotifications;

    for (auto& item : arr) {
        int itemId = item["id"].asInt().unwrapOrDefault();
        long long timestampUnix = item["timestamp_unix"].asInt().unwrapOr(0);
        if (timestampUnix <= m_lastNotificationTimestamp || m_readNotificationIds.contains(itemId)) {
            continue;
        }

        highestTimestamp = std::max(highestTimestamp, timestampUnix);
        std::string title = item["title"].asString().unwrapOr("Notification");
        std::string content = item["content"].asString().unwrapOr("New message");
        std::string timestamp = item["timestamp"].asString().unwrapOr("unknown");
        /*
        --- Notification Types ---
        info: normal information, no Icon
        success: some operation was successful, Checkmark icon
        warn: some warning occurred, Warn symbol icon
        error: there was an error in an operation, cross symbol icon
        critical: there was a critical error in an operation, critical warn symbol icon ?!
        --- Notification Types ---
        */
        std::string type = item["notification_type"].asString().unwrapOr("info");
        /*
        --- Notification Priorities ---
        deferMenu: Notification is deferred until the user is in a menu
        immediate: Notification will be instantly displayed, no matter what the user is doing
        onLayer: Notification will be deferred until the betterThumbnailLayer is opened
        --- Notification Priorities ---
        */
        std::string priority = item["notification_priority"].asString().unwrapOr("deferMenu");
        bool isToast = item["toast"].asBool().unwrapOr(false);

        bool shouldShowNow = false;
        if (priority == "immediate" && Mod::get()->getSettingValue<bool>("allow-notifications-gameplay") == true) {
            shouldShowNow = true;
        } else if (priority == "deferMenu") {
            shouldShowNow = !isInPlayLayer();
        } else if (priority == "onLayer") {
            shouldShowNow = isOnBetterThumbnailLayer();
        } else {
            shouldShowNow = !isInPlayLayer();
        }

        newNotifications.push_back({title, content, timestamp, type, itemId, timestampUnix, shouldShowNow, isToast});
    }

    if (highestTimestamp > m_lastNotificationTimestamp) {
        m_lastNotificationTimestamp = highestTimestamp;
    }

    if (!newNotifications.empty()) {
        m_notifications.insert(m_notifications.end(), newNotifications.begin(), newNotifications.end());

        bool shouldDisplay = false;
        for (auto const& entry : newNotifications) {
            if (entry.shouldShowNow) {
                shouldDisplay = true;
                break;
            }
        }

        if (shouldDisplay) {
            std::vector<NotificationMenuPopup::NotificationEntry> visibleEntries;
            for (auto const& entry : newNotifications) {
                if (!entry.shouldShowNow) {
                    continue;
                }
                if (entry.isToast) {
                    // @geode-ignore(unknown-resource)
                    FMODAudioEngine::sharedEngine()->playEffect("geode.loader/newNotif03.ogg");
                    NotificationIcon icon = NotificationIcon::None;
                    if (entry.type == "success") {
                        icon = NotificationIcon::Success;
                    } else if (entry.type == "warn") {
                        icon = NotificationIcon::Warning;
                    } else if (entry.type == "error") {
                        icon = NotificationIcon::Error;
                    } else if (entry.type == "critical") {
                        icon = NotificationIcon::Error;
                    }
                    Notification::create(fmt::format("[BetterThumbs]: {}", entry.body).c_str(), icon)->show();
                } else {
                    visibleEntries.push_back(entry);
                }
            }

            if (!visibleEntries.empty()) {
                std::string notifyTitle;
                std::string notifyMessage;
                std::string notifyType = "info";
                if (visibleEntries.size() == 1) {
                    notifyTitle = visibleEntries[0].title;
                    notifyMessage = visibleEntries[0].body;
                    notifyType = visibleEntries[0].type;
                } else {
                    notifyTitle = "Notifications";
                    notifyMessage = fmt::format("You have {} notifications! Click view", visibleEntries.size());
                }

                auto viewCallback = [this, notifications = std::move(visibleEntries)]() mutable {
                    this->showNotificationList(std::move(notifications));
                };

                auto notifUI = NotificationNode::create(notifyTitle, notifyMessage, notifyType, viewCallback);
                if (notifUI) {
                    this->addChild(notifUI, 100);
                }
            }
        }
    }

    m_isNotificationFetchInProgress = false;
}

void NotificationOverlay::showNotificationList() {
    std::vector<NotificationMenuPopup::NotificationEntry> unread;
    for (auto const& entry : m_notifications) {
        if (!m_readNotificationIds.contains(entry.id)) {
            unread.push_back(entry);
        }
    }
    showNotificationList(std::move(unread));
}

void NotificationOverlay::showNotificationList(std::vector<NotificationMenuPopup::NotificationEntry> notifications) {
    auto popup = NotificationMenuPopup::create();
    if (!popup) {
        Notification::create("Error opening notification list", NotificationIcon::Error)->show();
        return;
    }

    for (auto const& entry : notifications) {
        m_readNotificationIds.insert(entry.id);
    }
    saveReadNotificationIds();

    popup->setNotifications(notifications, Mod::get()->getSavedValue<int>("user_id"));
    popup->show();
}

void NotificationOverlay::loadReadNotificationIds() {
    if (!Mod::get()->hasSavedValue("read_notification_ids")) {
        return;
    }

    auto raw = Mod::get()->getSavedValue<std::string>("read_notification_ids");
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (token.empty()) {
            continue;
        }
        int id = 0;
        auto result = std::from_chars(token.data(), token.data() + token.size(), id);
        if (result.ec == std::errc()) {
            m_readNotificationIds.insert(id);
        }
    }
}

void NotificationOverlay::saveReadNotificationIds() {
    std::string raw;
    raw.reserve(m_readNotificationIds.size() * 4);
    bool first = true;
    for (auto id : m_readNotificationIds) {
        if (!first) {
            raw.push_back(',');
        }
        first = false;
        raw += std::to_string(id);
    }
    Mod::get()->setSavedValue<std::string>("read_notification_ids", raw);
}

static bool isInPlayLayer() {
    return PlayLayer::get() != nullptr;
}

static bool isOnBetterThumbnailLayer() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) {
        return false;
    }

    auto children = scene->getChildren();
    if (!children) {
        return false;
    }

    for (unsigned i = 0; i < children->count(); ++i) {
        auto child = static_cast<CCObject*>(children->objectAtIndex(i));
        if (typeinfo_cast<BetterThumbnailLayer*>(child)) {
            return true;
        }
    }
    return false;
}

void NotificationOverlay::updatePollingSchedule(float interval) {
    m_pollInterval = interval;
    this->unschedule(schedule_selector(NotificationOverlay::pollNotifications));
    this->schedule(schedule_selector(NotificationOverlay::pollNotifications), m_pollInterval);
}

NotificationOverlay* NotificationOverlay::get() {
    return overlayInstance();
}
