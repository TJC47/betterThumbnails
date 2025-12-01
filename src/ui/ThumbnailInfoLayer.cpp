#include "ThumbnailInfoLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/LazySprite.hpp>

#include "PendingThumbnailLayer.hpp"
#include "RejectReasonPopup.hpp"

using namespace geode::prelude;

CCScene* ThumbnailInfoLayer::scene(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
      auto scene = CCScene::create();
      scene->addChild(ThumbnailInfoLayer::create(id, user_id, username, level_id, accepted, upload_time, replacement));
      return scene;
}

ThumbnailInfoLayer* ThumbnailInfoLayer::create(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
      auto ret = new ThumbnailInfoLayer;
      if (ret && ret->init(id, user_id, username, level_id, accepted, upload_time, replacement)) {
            ret->autorelease();
            return ret;
      }
      CC_SAFE_DELETE(ret);
      return nullptr;
}

bool ThumbnailInfoLayer::init(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
      if (!CCLayer::init())
            return false;

      addSideArt(this, SideArt::All, false);

      // store fields for later actions
      m_id = id;
      m_userId = user_id;
      m_username = username;
      m_levelId = level_id;
      m_acceptedFlag = accepted;
      m_uploadTime = upload_time;
      m_replacementFlag = replacement;

      auto bg = createLayerBG();
      if (bg != nullptr)
            this->addChild(bg, -1);

      auto screenSize = CCDirector::sharedDirector()->getWinSize();

      auto menu = CCMenu::create();
      this->addChild(menu, 2);
      menu->setPosition({0.f, 0.f});

      // Back button
      auto backButton = CCMenuItemSpriteExtra::create(
          CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
          this,
          menu_selector(ThumbnailInfoLayer::onBackButton));
      backButton->setPosition({25.f, screenSize.height - 25.f});
      menu->addChild(backButton);

      // Title
      auto title = CCLabelBMFont::create("Thumbnail Details", "bigFont.fnt");
      title->setPosition({screenSize.width / 2.f, screenSize.height - 25.f});
      title->setScale(0.8f);
      title->setAlignment(kCCTextAlignmentCenter);
      this->addChild(title, 1);

      // Submitter
      auto submitter = CCLabelBMFont::create(fmt::format("Submitter: {} ({})", username, user_id).c_str(), "goldFont.fnt");
      submitter->setPosition({screenSize.width / 2.f, screenSize.height - 50.f});
      submitter->setScale(0.6f);
      submitter->setAlignment(kCCTextAlignmentCenter);
      this->addChild(submitter, 1);

      // Timestamp
      auto timestamp = CCLabelBMFont::create(fmt::format("Uploaded: {}", upload_time).c_str(), "chatFont.fnt");
      timestamp->setPosition({screenSize.width / 2.f, screenSize.height - 65.f});
      timestamp->setScale(0.4f);
      timestamp->setAlignment(kCCTextAlignmentCenter);
      this->addChild(timestamp, 1);

      // thumb bg
      auto thumbBg = CCScale9Sprite::create("GJ_square06.png");
      thumbBg->setPosition({screenSize.width / 2.f - 90.f, screenSize.height / 2.f - 20.f});
      thumbBg->setContentSize({300.f, 170.f});
      this->addChild(thumbBg);

      // Thumbnail
      auto thumb = LazySprite::create({1920.f, 1080.f}, true);
      thumb->setVisible(false);
      thumb->setScale(.625f);
      thumb->setAnchorPoint({0.5f, 0.5f});

      auto stencil = CCScale9Sprite::create("GJ_square06.png");
      stencil->setContentSize(thumbBg->getContentSize());

      auto clip = CCClippingNode::create(stencil);
      clip->setAnchorPoint(thumbBg->getAnchorPoint());
      clip->setPosition(thumbBg->getContentSize());
      thumbBg->addChild(clip);
      clip->addChild(thumb);

      auto spinner = LoadingSpinner::create(30.f);
      spinner->setPosition(thumb->getPosition());
      clip->addChild(spinner);
      clip->setContentSize(thumbBg->getContentSize());
      spinner->setVisible(true);

      {
            auto req = web::WebRequest();
            req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
            auto imageTask = req.get(fmt::format("https://levelthumbs.prevter.me/pending/{}/image", id));
            m_listener.bind([this, thumb, thumbBg, id, spinner](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->code() >= 200 && res->code() <= 299) {
                auto data = res->data();
                                    if (!data.empty()) {
                    thumb->loadFromData(data);
                    // After loading, compute proper scale to fit inside thumbBg while preserving aspect
                    auto contentSize = thumb->getContentSize();
                    if (contentSize.width > 0 && contentSize.height > 0) {
                        const float padding = 4.f; // small padding inside bg
                        float targetW = thumbBg->getContentSize().width - padding;
                        float targetH = thumbBg->getContentSize().height - padding;
                        float scaleX = targetW / contentSize.width;
                        float scaleY = targetH / contentSize.height;
                        const float forcedScale = 0.625f;
                        thumb->setScale(forcedScale);
                        thumb->setVisible(true);
                        if (spinner) spinner->setVisible(false);
                    }
                }
            } else {
                log::error("Image fetch error: {} {}", res->code(), res->string().unwrapOr(""));
                if (spinner) spinner->setVisible(false);
            }
        } });
            m_listener.setFilter(imageTask);
      }

      // Info box
      float panelX = screenSize.width * 0.55f + 80.f;
      float panelY = screenSize.height * 0.65f;
      float line = 0.f;
      auto makeLine = [&](const std::string& text, float scale = 0.35f) {
            auto lbl = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
            lbl->setAnchorPoint({0.f, 1.f});
            lbl->setPosition({panelX, panelY - line});
            lbl->setScale(scale);
            this->addChild(lbl);
            line += 22.f;
            return lbl;
      };

      // makeLine(fmt::format("Submitter: {} ({})", username, user_id));
      makeLine(fmt::format("Level ID: {}", level_id));
      makeLine(fmt::format("Accepted: {}", accepted ? "Yes" : "No"));
      makeLine(fmt::format("Replacement: {}", replacement ? "Yes" : "No"));
      makeLine(fmt::format("Thumbnail ID: {}", id));

      // Info Blackbox
      auto blackbox = CCScale9Sprite::create("square02_small.png");
      blackbox->setPosition({panelX + 70.f, panelY - 40.f});
      blackbox->setContentSize({160.f, line + 20.f});
      blackbox->setOpacity(100);
      this->addChild(blackbox, -1);

      // check if user role is a moderator/admin, show the button
      if (Mod::get()->getSavedValue<int>("role_num") >= 20) {
            auto acceptBtnSprite = ButtonSprite::create("Accept", 40, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 1.f);
            auto acceptBtn = CCMenuItemSpriteExtra::create(acceptBtnSprite, this, menu_selector(ThumbnailInfoLayer::onAccept));
            acceptBtn->setPosition({panelX + 30.f, panelY - line - 40.f});

            auto rejectBtnSprite = ButtonSprite::create("Reject", 40, true, "bigFont.fnt", "GJ_button_06.png", 30.f, 1.f);
            auto rejectBtn = CCMenuItemSpriteExtra::create(rejectBtnSprite, this, menu_selector(ThumbnailInfoLayer::onReject));
            rejectBtn->setPosition({panelX + 110.f, panelY - line - 40.f});

            auto actionMenu = CCMenu::create();
            actionMenu->addChild(acceptBtn);
            actionMenu->addChild(rejectBtn);
            auto playBtnSprite = ButtonSprite::create("Play Level", 80, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 1.f);
            auto playBtn = CCMenuItemSpriteExtra::create(playBtnSprite, this, menu_selector(ThumbnailInfoLayer::onPlayLevelButton));
            playBtn->setPosition({panelX + 70.f, panelY - line - 80.f});
            actionMenu->addChild(playBtn);
            actionMenu->setPosition({0.f, 0.f});
            this->addChild(actionMenu);
      }

      this->setKeypadEnabled(true);
      return true;
}

void ThumbnailInfoLayer::keyBackClicked() { onBackButton(nullptr); }

void ThumbnailInfoLayer::onBackButton(CCObject*) {
      CCDirector::get()->pushScene(CCTransitionFade::create(.5f, PendingThumbnailLayer::scene()));
}

void ThumbnailInfoLayer::onAccept(CCObject*) {
      auto req = web::WebRequest();
      req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
      req.bodyJSON(matjson::makeObject({{"accepted", true}}));
      auto task = req.post(fmt::format("https://levelthumbs.prevter.me/pending/{}", m_id));

      m_listener.bind([this](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->code() >= 200 && res->code() <= 299) {
                CCDirector::get()->pushScene(CCTransitionFade::create(.3f, PendingThumbnailLayer::scene()));
                Notification::create("Success! Thumbnail accepted.", NotificationIcon::Success)->show();
            } else {
                Notification::create("Error! Failed to accept thumbnail.", NotificationIcon::Error)->show();
            }
        } });
      m_listener.setFilter(task);
}

void ThumbnailInfoLayer::onReject(CCObject*) {
      auto popup = RejectReasonPopup::create(m_id, [this](std::string reason) {
        auto req = web::WebRequest();
        req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
        req.bodyJSON(matjson::makeObject({ {"accepted", false}, {"reason", reason} }));
        auto task = req.post(fmt::format("https://levelthumbs.prevter.me/pending/{}", m_id));

        m_listener.bind([this](web::WebTask::Event* e){
            if (auto res = e->getValue()) {
                if (res->code() >= 200 && res->code() <= 299) {
                    CCDirector::get()->pushScene(CCTransitionFade::create(.3f, PendingThumbnailLayer::scene()));
                    Notification::create("Success! Thumbnail rejected.", NotificationIcon::Success)->show();
                } else {
                    Notification::create("Error! Failed to reject thumbnail.", NotificationIcon::Error)->show();
                }
            }
        });
        m_listener.setFilter(task); });
      if (popup)
            popup->show();
}

void ThumbnailInfoLayer::onPlayLevelButton(CCObject*) {
      auto search = GJSearchObject::create(SearchType::Type19, std::to_string(m_levelId));
      CCDirector::get()->pushScene(CCTransitionFade::create(.5f, LevelBrowserLayer::scene(search)));
}
