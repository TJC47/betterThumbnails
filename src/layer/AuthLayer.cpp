#include "AuthLayer.hpp"
#include "../layer/BetterThumbnailLayer.hpp"

using namespace geode::prelude;

AuthLayer* AuthLayer::create() {
    auto ret = new AuthLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        ret->startAuthProcess();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AuthLayer::init() {
    if (!CCBlockLayer::init())
        return false;

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

    return true;
}

void AuthLayer::removeLoading() {
    this->removeFromParent();
}

void AuthLayer::startAuthProcess() {
    argonResponded = false;
    this->scheduleOnce(schedule_selector(AuthLayer::onArgonTimeout), 30.0f);
    async::spawn(
        argon::startAuth(),
        [this](Result<std::string> res) {
            argonResponded = true;
            this->unschedule(schedule_selector(AuthLayer::onArgonTimeout));
            if (!res) {
                FLAlertLayer::create("Oops!", "<cy>Argon</c> auth process <cr>failed</c>.", "OK")->show();
                this->removeLoading();
                return;
            }
            auto argon_token = res.unwrap();
            Mod::get()->setSavedValue<std::string>("token", argon_token);

            auto req = web::WebRequest();
            req.bodyJSON(
                matjson::makeObject({{"account_id", GJAccountManager::get()->m_accountID},
                    {"user_id", static_cast<int>(GameManager::get()->m_playerUserID)},
                    {"username", std::string(GJAccountManager::get()->m_username.c_str())},
                    {"argon_token", argon_token}}));

            // Change loading text before API login request
            if (loadingLabel) loadingLabel->setString("Logging in to Level Thumbnails API...");

            auto task = req.post("https://levelthumbs.prevter.me/auth/login");

            apiResponded = false;
            this->scheduleOnce(schedule_selector(AuthLayer::onApiTimeout), 30.0f);

            m_listener.spawn(std::move(task), [this](web::WebResponse res) {
                apiResponded = true;
                this->unschedule(schedule_selector(AuthLayer::onApiTimeout));
                auto code = res.code();
                auto responseStr = res.string().unwrapOr("");
                if (res.ok()) {
                    auto json = res.json().unwrapOrDefault();
                    log::info("{} {}", res.code(), json.dump());
                    auto token = json["token"].asString().unwrapOrDefault();
                    auto role = json["user"]["role"].asString().unwrapOrDefault();
                    auto id = json["user"]["id"].asInt().unwrapOrDefault();
                    auto username = json["user"]["username"].asString().unwrapOrDefault();
                    Mod::get()->setSavedValue<std::string>("token", token);
                    Mod::get()->setSavedValue<std::string>("role", role);
                    /*
                    --- Role Numerical Reference ---
                    0 - User
                    10 - Verified
                    20 - Moderator
                    30 - Admin
                    40 - Owner

                    -1 - Unknown
                    ---------------------------------

                    (Using offset of 10 incase any roles should get added inbetween later)
                    */
                    long user_role_num;
                    if (role == "user")
                        user_role_num = 0;
                    else if (role == "verified")
                        user_role_num = 10;
                    else if (role == "moderator")
                        user_role_num = 20;
                    else if (role == "admin")
                        user_role_num = 30;
                    else if (role == "owner")
                        user_role_num = 40;
                    else
                        user_role_num = -1;
                    Mod::get()->setSavedValue<long>("role_num", user_role_num);
                    Mod::get()->setSavedValue<std::string>("username", username);
                    Mod::get()->setSavedValue<long>("user_id", id);

                    auto layer = BetterThumbnailLayer::create();
                    auto scene = CCScene::create();
                    auto transition = CCTransitionFade::create(.5f, scene);
                    scene->addChild(layer);
                    CCDirector::get()->pushScene(transition);
                    this->removeLoading();
                } else {
                    FLAlertLayer::create(
                        "Login Failed",
                        fmt::format("API responded with code {}:\n{}", code, responseStr),
                        "OK")
                        ->show();
                    this->removeLoading();
                }
            });
        });
}

void AuthLayer::onArgonTimeout(float) {
    if (!argonResponded) {
        FLAlertLayer::create("Argon Timeout", "Argon authentication is taking too long. Please try again.", "OK")->show();
        this->removeLoading();
    }
}

void AuthLayer::onApiTimeout(float) {
    if (!apiResponded) {
        FLAlertLayer::create("API Timeout", "API authentication request is taking too long. Please try again.", "OK")->show();
        this->removeLoading();
    }
}
