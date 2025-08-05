#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <argon/argon.hpp>

#include "webrequest/ThumbRequest.hpp"

#include "ui/BetterThumbnailLayer.hpp"
#include "ui/NotificationUI.hpp"
#include "ui/AuthLayer.hpp"

using namespace geode::prelude;

EventListener<web::WebTask> m_listener;

class $modify(MyCreatorLayer, CreatorLayer)
{
	bool init()
	{
		if (!CreatorLayer::init())
			return false;

		auto myButton = CCMenuItemSpriteExtra::create(
			CCSprite::create("betterThumbnailButton.png"_spr),
			this,
			menu_selector(MyCreatorLayer::onMyButton));

		auto menu = this->getChildByID("bottom-right-menu");
		menu->addChild(myButton);

		myButton->setID("better-thumbnails-button"_spr);

		menu->updateLayout();

		this->setTouchEnabled(false);

		return true;
	}

	void onMyButton(CCObject *)
	{
		if (Mod::get()->hasSavedValue("token") &! Mod::get()->getSettingValue<bool>("dev-force-reauth")
)
		{
			CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
		}
		else
		{
			geode::createQuickPopup(
				"Notice",
				"To use the Better Level Thumbnails Mod you must <cg>authenticate</c> with your Geometry Dash account and the Level Thumbnails servers.",
				"No",
				"Authenticate",
				[this](FLAlertLayer *alert, bool btn2)
				{
					if (btn2)
					{
						auto authLayer = AuthLayer::create();
						auto scene = CCDirector::sharedDirector()->getRunningScene();
						if (scene && authLayer)
						{
							scene->addChild(authLayer, 9999);
						}
					}
				});
		}
	}
};