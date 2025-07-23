#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <gdutilsdevs.gdutils/include/RateEvent.hpp>
#include "webrequest/ThumbRequest.hpp"
#include "ui/BetterThumbnailLayer.hpp"
#include <string>

using namespace geode::prelude;

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

		return true;
	}

	/**
	REMINDER HOW TO USE MOD
			EventData data = {
			.type=EventType::Announcement, // type of notification
			.title="Your Thumbnail for Generation retro has been accepted!", // notification title,

		}; // Level ID is optional
		GDUtils::Events::RateEvent::emit(data);
	*/

	void onMyButton(CCObject *)
	{

		CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
	}
};