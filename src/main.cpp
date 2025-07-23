#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <gdutilsdevs.gdutils/include/RateEvent.hpp>
#include "ThumbRequest.hpp"
#include "ui/BetterThumbnailLayer.hpp"
#include <string>

using namespace geode::prelude;
// ...existing code...

class $modify(MyCreatorLayer, CreatorLayer)
{
	/**
	 * Typically classes in GD are initialized using the `init` function, (though not always!),
	 * so here we use it to add our own button to the bottom menu.
	 *
	 * Note that for all hooks, your signature has to *match exactly*,
	 * `void init()` would not place a hook!
	 */

	bool init()
	{
		/**
		 * We call the original init function so that the
		 * original class is properly initialized.
		 */
		if (!CreatorLayer::init())
		{
			return false;
		}

		/**
		 * You can use methods from the `geode::log` namespace to log messages to the console,
		 * being useful for debugging and such. See this page for more info about logging:
		 * https://docs.geode-sdk.org/tutorials/logging
		 */
		log::debug("Hello from my MenuLayer::init hook! This layer has {} children.", this->getChildrenCount());

		/**
		 * See this page for more info about buttons
		 * https://docs.geode-sdk.org/tutorials/buttons
		 */
		auto myButton = CCMenuItemSpriteExtra::create(
			CCSprite::create("betterThumbnailButton.png"_spr),
			this,
			/**
			 * Here we use the name we set earlier for our modify class.
			 */
			menu_selector(MyCreatorLayer::onMyButton));

		/**
		 * Here we access the `bottom-menu` node by its ID, and add our button to it.
		 * Node IDs are a Geode feature, see this page for more info about it:
		 * https://docs.geode-sdk.org/tutorials/nodetree
		 */
		auto menu = this->getChildByID("bottom-right-menu");
		menu->addChild(myButton);

		/**
		 * The `_spr` string literal operator just prefixes the string with
		 * your mod id followed by a slash. This is good practice for setting your own node ids.
		 */
		myButton->setID("better-thumbnails-button"_spr);

		/**
		 * We update the layout of the menu to ensure that our button is properly placed.
		 * This is yet another Geode feature, see this page for more info about it:
		 * https://docs.geode-sdk.org/tutorials/layouts
		 */
		menu->updateLayout();

		/**
		 * We return `true` to indicate that the class was properly initialized.
		 */
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

	/**
	 * This is the callback function for the button we created earlier.
	 * The signature for button callbacks must always be the same,
	 * return type `void` and taking a `CCObject*`.
	 */

	void onMyButton(CCObject *)
	{

		CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
	}
};