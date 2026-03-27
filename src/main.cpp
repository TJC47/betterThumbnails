#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <argon/argon.hpp>

#include "layer/BetterThumbnailLayer.hpp"
#include "layer/AuthLayer.hpp"

using namespace geode::prelude;

class $modify(BTNCreatorLayer, CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init())
            return false;

        auto myButton = CCMenuItemSpriteExtra::create(
            CCSprite::create("betterThumbnailButton.png"_spr),
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