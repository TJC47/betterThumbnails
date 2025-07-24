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
		if (Mod::get()->hasSavedValue("token") && false) // remove the && false pls
		{
			CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
		}
		else
		{
			geode::createQuickPopup("Notice", "To use the Better Level Thumbnails Mod you must <cg>authenticate</c> with your Geometry Dash account and the Level Thumbnails servers.", "No", "Authenticate", [this](auto alert, bool btn2)
									{
            if (btn2){
				// loading background
				auto winSize = CCDirector::sharedDirector()->getWinSize();
				auto bg = CCLayerColor::create({0, 0, 0, 200});
				bg->setPosition({0, 0});
				this->addChild(bg, 99);

				auto loadingLabel = CCLabelBMFont::create("Authenticating with Argon...", "bigFont.fnt");
				loadingLabel->setAnchorPoint({0.5f, 0.5f});
				loadingLabel->setPosition({winSize.width / 2.f, winSize.height / 2.f});
				loadingLabel->setScale(0.5f);
				this->addChild(loadingLabel, 100);

				// start the argon auth process
				auto res = argon::startAuth([this, bg, loadingLabel](Result<std::string> res){
					if (!res){
						FLAlertLayer::create("Oops!","The <cy>Argon</c> auth process failed.","OK")->show();
						bg->removeFromParent();
						loadingLabel->removeFromParent();
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

    				auto task = req.post("https://levelthumbs.prevter.me/auth/login");

					m_listener.bind([this, loadingLabel, bg](web::WebTask::Event* e){
						if (auto res = e->getValue()){
							auto code = res->code();
							if (code<200||code>299){
								auto error = res->string().unwrapOr(res->errorMessage());
								FLAlertLayer::create("Oops",error,"OK")->show();
								bg->removeFromParent();
								loadingLabel->removeFromParent();
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
							Mod::get()->setSavedValue<std::string>("username", username);
							Mod::get()->setSavedValue<long>("user_id", id);

							CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
						}
					});
					m_listener.setFilter(task);
                });
            } });
		}
	}
};