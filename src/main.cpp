/**
 * Include the Geode headers.
 */
#include "Geode/binding/CreatorLayer.hpp"
#include "Geode/binding/FLAlertLayer.hpp"
#include "Geode/c++stl/string.hpp"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/ui/General.hpp"
#include "Geode/ui/LazySprite.hpp"
#include <Geode/Geode.hpp>
#include <gdutilsdevs.gdutils/include/RateEvent.hpp>
#include <Geode/utils/web.hpp>
#include <string>
/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;

/**
 * `$modify` lets you extend and modify GD's classes.
 * To hook a function in Geode, simply $modify the class
 * and write a new function definition with the signature of
 * the function you want to hook.
 *
 * Here we use the overloaded `$modify` macro to set our own class name,
 * so that we can use it for button callbacks.
 *
 * Notice the header being included, you *must* include the header for
 * the class you are modifying, or you will get a compile error.
 *
 * Another way you could do this is like this:
 *
 * struct MyMenuLayer : Modify<MyMenuLayer, MenuLayer> {};
 */
#include <Geode/modify/CreatorLayer.hpp>


class BetterThumbnailLayer : public CCLayer {


EventListener<web::WebTask> m_listener;
EventListener<web::WebTask> e_listener;

  static BetterThumbnailLayer* create() {
    auto ret = new BetterThumbnailLayer;
    if (ret->init()) {
      ret->autorelease();
      return ret;
    }

    delete ret;
    return nullptr;
  }
  
  // optional and i dont tihnk this is correct
  public: static CCScene* scene() {
    auto scene = CCScene::create();
    scene->addChild(BetterThumbnailLayer::create());

    return scene;
  }



  bool init() {
    if (!CCLayer::init()) return false;

	CCSize screenSize = CCDirector::sharedDirector()->getWinSize();

    addChild(createLayerBG());
	auto menu = CCMenu::create();
	addChild(menu);
	auto backButton = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
			this,
			menu_selector(BetterThumbnailLayer::onBackButton)
		);
	backButton->setPosition({30.f , screenSize.height-30.f});
	menu->setPosition({0.f,0.f});
	menu->addChild(backButton);

	auto banner = LazySprite::create({512.f, 256.f});
	banner->setPosition({screenSize.width / 2.f, screenSize.height * 0.85f});
	//banner->setAnchorPoint({0.5f, 1.f});
	//banner->setAutoResize(true);
	//banner->setScaleX(512.f);
	//banner->setScaleY(256.f);
	addChild(banner);

    // yo internet shit
	auto info = CCLabelBMFont::create("loading...", "bigFont.fnt");
	info->setPosition({screenSize});
	info->setAnchorPoint({1.f, 1.f});
	info->setScale(0.5f);
	addChild(info);
	
	auto meta_url = std::string(""); // this one fetches the meta url

		// this one fgetches the "message of the day"
    e_listener.bind([info, banner, screenSize] (web::WebTask::Event* e) {
            if (web::WebResponse* res = e->getValue()) {
                log::info("{}", res->string().unwrapOr("Uh oh!"));
				info->setString(res->string().unwrapOr("Uh oh!").c_str());
				EventData data = {
					.type=EventType::Announcement, // type of notification
					.title=fmt::format("{}", res->string().unwrapOr("Uh oh!")) // notification title,
				}; // Level ID is optional
				GDUtils::Events::RateEvent::emit(data);
				banner->loadFromUrl("https://tjcsucht.net/static/banner.png");
				//banner->setPosition({screenSize.width / 2.f, screenSize.height * 0.95f});
				//banner->setAnchorPoint({0.5f, 1.f});
            } else if (web::WebProgress* p = e->getProgress()) {
                log::info("progress: {}", p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::info("The request was cancelled... So sad :(");
				info->setString("There was an error");
				FLAlertLayer::create("Request cancelled", "It should not be possible to get this error", "Ok")->show();
				CCDirector::get()->pushScene(CCTransitionFade::create(.5f, CreatorLayer::scene()));
            }
    });

    m_listener.bind([info, meta_url, this] (web::WebTask::Event* e) {
            if (web::WebResponse* res = e->getValue()) {
				auto new_meta_url = fmt::format("{}",res->string().unwrapOr("Uh oh!"));
                log::info("{}", res->string().unwrapOr("Uh oh!"));
				info->setString(fmt::format("Meta server: {}",res->string().unwrapOr("Uh oh!").c_str()).c_str());
				EventData data = {
					.type=EventType::Announcement, // type of notification
					.title=fmt::format("{}{} as meta server\n(Images are supplied by vanilla thumbnails)", "Using ", res->string().unwrapOr("Uh oh!")) // notification title,
				}; // Level ID is optional
				GDUtils::Events::RateEvent::emit(data);
				auto req = web::WebRequest();
        		e_listener.setFilter(req.get(fmt::format("{}/motd", new_meta_url)));
            } else if (web::WebProgress* p = e->getProgress()) {
                log::info("progress: {}", p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::info("The request was cancelled... So sad :(");
				info->setString("There was an error");
				FLAlertLayer::create("Request cancelled", "It should not be possible to get this error", "Ok")->show();
				CCDirector::get()->pushScene(CCTransitionFade::create(.5f, CreatorLayer::scene()));
            }
    });

        auto req = web::WebRequest();
        // Let's fetch... uhh...
        m_listener.setFilter(req.get("https://tjcsucht.net/api/btserver"));

    return true;
  };

	void onBackButton(CCObject*) {
		
		CCDirector::get()->pushScene(CCTransitionFade::create(.5f, CreatorLayer::scene()));

		
	}
};

class $modify(MyCreatorLayer, CreatorLayer) {
	/**
	 * Typically classes in GD are initialized using the `init` function, (though not always!),
	 * so here we use it to add our own button to the bottom menu.
	 *
	 * Note that for all hooks, your signature has to *match exactly*,
	 * `void init()` would not place a hook!
	*/

	bool init() {
		/**
		 * We call the original init function so that the
		 * original class is properly initialized.
		 */
		if (!CreatorLayer::init()) {
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
			menu_selector(MyCreatorLayer::onMyButton)
		);

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


	void onMyButton(CCObject*) {
		
		CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));

		
	}
};