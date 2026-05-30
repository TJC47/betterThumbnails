#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <argon/argon.hpp>

#include "layer/BetterThumbnailLayer.hpp"
#include "layer/AuthLayer.hpp"
#include "node/NotificationNode.hpp"
#include "popup/NotificationMenuPopup.hpp"
#include "Geode/ui/OverlayManager.hpp"

using namespace geode::prelude;

class $modify(BTNMenuLayer, MenuLayer) {
    struct Fields {
        int m_lastNotificationId = 0;
        bool m_isNotificationFetchInProgress = false;
        async::TaskHolder<web::WebResponse> m_notificationListener;
    };
    bool init() {
        if (!MenuLayer::init()) return false;
        // fetch notifications for this user and show new ones
        this->fetchNotifications();
        this->schedule(schedule_selector(BTNMenuLayer::pollNotifications), 30.0f);
        return true;
    }
    void pollNotifications(float) {
        this->fetchNotifications();
    }
    void fetchNotifications() {
        auto userId = Mod::get()->getSavedValue<int>("user_id");
        if (userId <= 0) {
            return;
        }

        if (m_fields->m_isNotificationFetchInProgress) {
            return;
        }
        m_fields->m_isNotificationFetchInProgress = true;

        auto req = web::WebRequest();
        req.header("Authorization",
            fmt::format("Bearer {}",
                Mod::get()->getSavedValue<std::string>("token")));

        auto task = req.get(fmt::format("https://tjcsucht.net/api/bt/getnotif/{}", userId));
        m_fields->m_notificationListener.spawn(std::move(task), [this](web::WebResponse res) {
            if (res.code() < 200 || res.code() > 299) {
                log::error("Notification API error {}: {}", res.code(), res.string().unwrapOrDefault());
                m_fields->m_isNotificationFetchInProgress = false;
                return;
            }
            auto jsonResult = res.json();
            if (!jsonResult.isOk()) {
                log::error("Notification API JSON parse error: {}",
                    jsonResult.unwrapErr());
                m_fields->m_isNotificationFetchInProgress = false;
                return;
            }

            auto json = jsonResult.unwrap();
            if (!json.isObject() || !json["notifications"].isArray()) {
                log::error("Notification API invalid format: {}",
                    res.string().unwrapOrDefault());
                m_fields->m_isNotificationFetchInProgress = false;
                return;
            }

            auto arr = json["notifications"].asArray().copied().unwrapOrDefault();
            int highestId = m_fields->m_lastNotificationId;

            std::vector<NotificationMenuPopup::NotificationEntry> newNotifications;
            for (auto& item : arr) {
                auto itemId = item["id"].asInt().unwrapOrDefault();
                if (itemId <= 0 || itemId <= m_fields->m_lastNotificationId) {
                    continue;
                }

                auto title = item["title"].asString().unwrapOr("Notification");
                auto content = item["content"].asString().unwrapOr("New message");
                auto timestamp = item["timestamp"].asString().unwrapOr("unknown");
                /*
                --- Notification Types ---
                info: normal information, no Icon
                success: some operation was successful, Checkmark icon
                warn: some warning occurred, Warn symbol icon
                error: there was an error in an operation, cross symbol icon
                critical: there was a critical error in an operation, critical warn symbol icon ?!
                --- Notification Types ---
                */
                auto type = item["notification_type"].asString().unwrapOr("info");
                /*
                --- Notification Priorities ---
                deferMenu: Notification is deferred until the user is in a menu
                immediate: Notification will be instantly displayed, no matter what the user is doing
                onLayer: Notification will be deferred until the betterThumbnailLayer is opened
                --- Notification Priorities ---
                */
                auto priority = item["notification_priority"].asString().unwrapOr("deferMenu");

                auto toast = item["toast"].asBool().unwrapOr(false);
                if (toast) {
                    FMODAudioEngine::sharedEngine()->playEffect("geode.loader/newNotif03.ogg");
                } else {
                    newNotifications.push_back({title, content, timestamp});
                    highestId = std::max(highestId, static_cast<int>(itemId));
                }
            }

            if (!newNotifications.empty()) {
                std::string notifyTitle;
                std::string notifyMessage;

                if (newNotifications.size() == 1) {
                    notifyTitle = newNotifications[0].title;
                    notifyMessage = newNotifications[0].body;
                } else {
                    notifyTitle = "Notifications";
                    notifyMessage = fmt::format("You have {} notifications! Click view", newNotifications.size());
                }

                auto viewCallback = [this, notifications = std::move(newNotifications)]() mutable {
                    auto popup = NotificationMenuPopup::create();
                    if (!popup) {
                        Notification::create("Error opening notification list", NotificationIcon::Error)->show();
                        return;
                    }
                    popup->setNotifications(notifications, Mod::get()->getSavedValue<int>("user_id"));
                    popup->show();
                };

                auto notifUI = NotificationNode::create(notifyTitle, notifyMessage, viewCallback);
                if (notifUI) {
                    OverlayManager::get()->addChild(notifUI, 100);
                }
            }

            if (highestId > m_fields->m_lastNotificationId) {
                m_fields->m_lastNotificationId = highestId;
            }
            m_fields->m_isNotificationFetchInProgress = false;
        });
    }
};

class $modify(BTNCreatorLayer, CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init())
            return false;

        auto myButton = CCMenuItemSpriteExtra::create(
            CCSprite::create("BT_betterThumbnailButton.png"_spr),
            this,
            menu_selector(BTNCreatorLayer::onBTNButton));

        auto menu = this->getChildByID("bottom-right-menu");
        menu->addChild(myButton);
        myButton->setID("better-thumbnails-button"_spr);
        menu->updateLayout();
        this->setTouchEnabled(false);

        return true;
    }

    void onBTNButton(CCObject*) {
        if (Mod::get()->hasSavedValue("token") & !Mod::get()->getSettingValue<bool>("dev-force-reauth")) {
            auto layer = BetterThumbnailLayer::create();
            auto scene = CCScene::create();
            auto transition = CCTransitionFade::create(.5f, scene);
            scene->addChild(layer);
            CCDirector::get()->pushScene(transition);
        } else {
            geode::createQuickPopup(
                "Notice",
                "To use the Better Level Thumbnails Mod you must <cg>authenticate</c> with your Geometry Dash account and the Level Thumbnails servers.",
                "No",
                "Authenticate",
                [this](FLAlertLayer* alert, bool btn2) {
                    if (btn2) {
                        auto authLayer = AuthLayer::create();
                        auto scene = CCDirector::sharedDirector()->getRunningScene();
                        if (scene && authLayer) {
                            scene->addChild(authLayer, 9999);
                        } else {
                            FLAlertLayer::create(
                                "Error",
                                "Could not open authentication layer.",
                                "OK")
                                ->show();
                        }
                    }
                });
        }
    }
};