#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <argon/argon.hpp>

#include "webrequest/ThumbRequest.hpp"
#include "ui/BetterThumbnailLayer.hpp"
#include "ui/NotificationUI.hpp"

using namespace geode::prelude;

EventListener<web::WebTask> m_listener;

class $modify(MyCreatorLayer, CreatorLayer)
{

	class AuthLayer : public CCLayerColor
	{
	public:
		CCLabelBMFont *loadingLabel = nullptr;
		CCSprite *loadingSpinner = nullptr;
		bool argonResponded = false;
		bool apiResponded = false;
		EventListener<web::WebTask> m_listener;

		static AuthLayer *create()
		{
			auto ret = new AuthLayer();
			if (ret && ret->init())
			{
				ret->autorelease();
				ret->startAuthProcess();
				return ret;
			}
			CC_SAFE_DELETE(ret);
			return nullptr;
		}

		bool init()
		{
			if (!CCLayerColor::initWithColor({0, 0, 0, 150}))
				return false;

			cocos2d::CCTouchDispatcher::get()->registerForcePrio(this, 2);

			auto winSize = CCDirector::sharedDirector()->getWinSize();
			this->setPosition({0, 0});

			// Create loading spinner at center
			auto loadingSpinner = LoadingSpinner::create(100.f);
			loadingSpinner->setAnchorPoint({0.5f, 0.5f});
			loadingSpinner->setPosition({winSize.width / 2.f, winSize.height / 2.f});
			this->addChild(loadingSpinner, 2);

			// Move loading text above spinner
			loadingLabel = CCLabelBMFont::create("Authenticating with Argon...", "bigFont.fnt");
			loadingLabel->setAnchorPoint({0.5f, 0.5f});
			loadingLabel->setPosition({winSize.width / 2.f, winSize.height / 2.f + 50});
			loadingLabel->setScale(0.5f);
			this->addChild(loadingLabel, 1);

			this->setTouchEnabled(true);
			this->setKeypadEnabled(true);
			this->setZOrder(999);

			return true;
		}

		void removeLoading()
		{
			this->removeFromParent();
		}

		void startAuthProcess()
		{
			argonResponded = false;
			this->scheduleOnce(schedule_selector(AuthLayer::onArgonTimeout), 10.0f);
			auto res = argon::startAuth([this](Result<std::string> res)
										{
			argonResponded = true;
			this->unschedule(schedule_selector(AuthLayer::onArgonTimeout));
			if (!res){
				FLAlertLayer::create("Oops!","The <cy>Argon</c> auth process failed.","OK")->show();
				this->removeLoading();
				return;
			}
			auto argon_token = res.unwrap();
			Mod::get()->setSavedValue<std::string>("token", argon_token);

			auto req = web::WebRequest();
			req.bodyJSON(
				matjson::makeObject({
					{"account_id", GJAccountManager::get()->m_accountID},
					{"user_id", (int)GameManager::get()->m_playerUserID},
					{"username", GJAccountManager::get()->m_username},
					{"argon_token", argon_token}
				})
			);

			// Change loading text before API login request
			if (loadingLabel) loadingLabel->setString("Logging in to Level Thumbnails API...");

			auto task = req.post("https://levelthumbs.prevter.me/auth/login");

			apiResponded = false;
			this->scheduleOnce(schedule_selector(AuthLayer::onApiTimeout), 10.0f);

			m_listener.bind([this](web::WebTask::Event* e){
				if (auto res = e->getValue()){
					apiResponded = true;
					this->unschedule(schedule_selector(AuthLayer::onApiTimeout));
					auto code = res->code();
					auto responseStr = res->string().unwrapOr("");
					if (responseStr.find("Request cancelled") != std::string::npos) {
						FLAlertLayer::create("Error", "The request was cancelled. Please try again.", "OK")->show();
						this->removeLoading();
						delete this;
						return;
					}
					if (code<200||code>299){
						auto error = responseStr.empty() ? res->errorMessage() : responseStr;
						FLAlertLayer::create("Oops",error,"OK")->show();
						this->removeLoading();
						delete this;
						return;
					}
					auto json = res->json().unwrapOrDefault();
					log::info("{} {}",res->code(),json.dump());
					auto token = json["token"].asString().unwrapOrDefault();
					auto role = json["user"]["role"].asString().unwrapOrDefault();
					auto id = json["user"]["id"].asInt().unwrapOrDefault();
					auto username = json["user"]["username"].asString().unwrapOrDefault();
					Mod::get()->setSavedValue<std::string>("token", token);
					Mod::get()->setSavedValue<std::string>("role", role);
					long user_role_num;
					if (role == "user") user_role_num = 0;
					else if (role == "verified") user_role_num = 10;
					else if (role == "moderator") user_role_num = 20;
					else if (role == "admin") user_role_num = 30;
					else if (role == "owner") user_role_num = 40;
					else user_role_num = -1;
					Mod::get()->setSavedValue<long>("role_num", user_role_num);
					Mod::get()->setSavedValue<std::string>("username", username);
					Mod::get()->setSavedValue<long>("user_id", id);
					CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
					this->removeLoading();
				}
			});
			m_listener.setFilter(task); });
		}

		void onArgonTimeout(float)
		{
			if (!argonResponded)
			{
				FLAlertLayer::create("Argon Timeout", "Argon authentication is taking too long. Please try again.", "OK")->show();
				this->removeLoading();
			}
		}

		void onApiTimeout(float)
		{
			if (!apiResponded)
			{
				FLAlertLayer::create("API Timeout", "API authentication request is taking too long. Please try again.", "OK")->show();
				this->removeLoading();
			}
		}
	};

	struct Fields
	{
		AuthLayer *authLayer = nullptr;
	};

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
		if (Mod::get()->hasSavedValue("token") && false)
		{
			CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
		}
		else
		{
			geode::createQuickPopup("Notice", "To use the Better Level Thumbnails Mod you must <cg>authenticate</c> with your Geometry Dash account and the Level Thumbnails servers.", "No", "Authenticate", std::bind(&MyCreatorLayer::onAuthPopup, this, std::placeholders::_1, std::placeholders::_2));
		}
	}

	void onAuthPopup(FLAlertLayer *alert, bool btn2)
	{
		if (btn2)
		{
			// Show AuthLayer and start authentication on the current scene
			auto authLayer = AuthLayer::create();
			auto scene = CCDirector::sharedDirector()->getRunningScene();
			if (scene && authLayer)
			{
				scene->addChild(authLayer, 9999);
				this->setTouchEnabled(false);
				authLayer->setTag(2); // Use tag to identify
				this->m_fields->authLayer = authLayer;
			}
		}
	}
};
