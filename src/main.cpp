#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>

#include "webrequest/ThumbRequest.hpp"
#include "ui/BetterThumbnailLayer.hpp"

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

	void onMyButton(CCObject *)
	{

		CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
	}
};