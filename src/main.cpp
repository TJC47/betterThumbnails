#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>

#include <argon/argon.hpp>
#include "Geode/binding/FLAlertLayer.hpp"
#include "Geode/loader/Log.hpp"
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
		if (Mod::get()->hasSavedValue("token")) {
			CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
		}
		else {

        createQuickPopup("Notice","To use the Better Level Thumbnails Mod you must <cg>authenticate</c> with your Geometry Dash account and the Level Thumbnails servers.","No","Authenticate",[this](auto alert,bool btn2){
            if (btn2){
				auto notif = NotificationUI::create("Please Hold", "Authenticating with argon...");
				if (notif)
				{
					this->addChild(notif, 100);
				}
                auto res = argon::startAuth([this](Result<std::string> res){
                    if (!res){
                        FLAlertLayer::create("Oops!","The <cy>Argon</c> auth process failed.","OK")->show();
                        return;
                    }
					auto argon_token = res.unwrap();
					Mod::get()->setSavedValue<std::string>("token", argon_token);



					auto req = web::WebRequest();

					std::string form = fmt::format("account_id={}&&user_id={}&&username={}&&argon_token={}",
					GJAccountManager::get()->m_accountID,
					(int)GameManager::get()->m_playerUserID,
					GJAccountManager::get()->m_username,
					argon_token
					);
					req.bodyString(form);


    				auto task = req.post("https://levelthumbs.prevter.me/auth/login");

					m_listener.bind([this](web::WebTask::Event* e){
						if (auto res = e->getValue()){
							auto code = res->code();
							if (code<200||code>299){
								auto error = res->string().unwrapOr(res->errorMessage());
								FLAlertLayer::create("Oops",error,"OK")->show();
								delete this;
								return;
							}
							auto json = res->json().unwrapOrDefault();
							geode::log::info("{} {}",res->code(),json.dump());
							auto token = json["token"].asString().unwrapOrDefault();
							auto role = json["user"]["role"].asString().unwrapOrDefault();
							auto id = json["user"]["id"].asInt().unwrapOrDefault();
							Mod::get()->setSavedValue<std::string>("token", token);
							Mod::get()->setSavedValue<std::string>("role", role);
							Mod::get()->setSavedValue<long>("user_id", id);
							CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
						}
					});
					m_listener.setFilter(task);
                });
            }
        });
		}
	}
};