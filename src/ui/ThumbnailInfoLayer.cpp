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

      // Submitter
      auto submitter = CCLabelBMFont::create(fmt::format("Submitter: {} ({})", username, user_id).c_str(), "goldFont.fnt");
      submitter->setPosition({screenSize.width / 2.f, screenSize.height - 25.f});
      submitter->setAlignment(kCCTextAlignmentCenter);
      this->addChild(submitter, 1);

      // Timestamp
      auto timestamp = CCLabelBMFont::create(fmt::format("Uploaded: {}", upload_time).c_str(), "chatFont.fnt");
      timestamp->setPosition({screenSize.width / 2.f, screenSize.height - 50.f});
      timestamp->setScale(0.75f);
      timestamp->setAlignment(kCCTextAlignmentCenter);
      this->addChild(timestamp, 1);

      // thumb bg
      auto thumbBg = CCScale9Sprite::create("GJ_square06.png");
      thumbBg->setPosition({screenSize.width / 2.f - 90.f, screenSize.height / 2.f - 20.f});
      thumbBg->setContentSize({300.f, 170.f});
      this->addChild(thumbBg);

      // Thumbnail (replacement + original)
      auto thumbReplacement = LazySprite::create({300.f, 170.f}, true);
      thumbReplacement->setVisible(true);  // show replacement first by default
      thumbReplacement->setAutoResize(true);
      thumbReplacement->setAnchorPoint({0.5f, 0.5f});
      m_thumbReplacement = thumbReplacement;

      auto thumbOriginal = LazySprite::create({300.f, 170.f}, false);
      thumbOriginal->setVisible(false);  // original starts hidden by default
      thumbOriginal->setAutoResize(true);
      thumbOriginal->setAnchorPoint({0.5f, 0.5f});
      m_thumbOriginal = thumbOriginal;

      auto stencil = CCScale9Sprite::create("GJ_square06.png");
      stencil->setContentSize(thumbBg->getContentSize());
      stencil->setPosition({0, 0});

      auto clip = CCClippingNode::create(stencil);
      clip->setAnchorPoint(thumbBg->getAnchorPoint());
      clip->setPosition(thumbBg->getContentSize());
      thumbBg->addChild(clip);

      // add both sprites to the clipped area
      clip->addChild(thumbReplacement);
      clip->addChild(thumbOriginal);

      auto spinner = LoadingSpinner::create(30.f);
      spinner->setPosition({thumbBg->getContentSize().width / 2.f, thumbBg->getContentSize().height / 2.f});
      clip->addChild(spinner);
      clip->setContentSize(thumbBg->getContentSize());
      spinner->setVisible(true);
      m_thumbSpinner = spinner;

      // Label above thumbnail indicating which image is shown
      auto thumbLabel = CCLabelBMFont::create(m_replacementFlag ? "Replacement" : "Original", "bigFont.fnt");
      thumbLabel->setScale(0.5f);
      thumbLabel->setAnchorPoint({0.5f, 0.5f});
      thumbLabel->setPosition({thumbBg->getPositionX(), thumbBg->getPositionY() + thumbBg->getContentSize().height / 2.f + 12.f});
      this->addChild(thumbLabel, 2);
      m_thumbLabel = thumbLabel;

      if (!m_replacementFlag) thumbLabel->setVisible(false);  // hide label if no replacement

      auto req = web::WebRequest();
      req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
      auto imageTask = req.get(fmt::format("https://levelthumbs.prevter.me/pending/{}/image", id));
      m_listener.bind([this, thumbReplacement, thumbOriginal, thumbBg, id, spinner](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->code() >= 200 && res->code() <= 299) {
                auto data = res->data();
                                    if (!data.empty()) {
                                    thumbReplacement->loadFromData(data);
                        if (spinner) spinner->setVisible(false);
                }
            } else {
                log::error("Image fetch error: {} {}", res->code(), res->string().unwrapOr(""));
                if (spinner) spinner->setVisible(false);
            }
        } });
      m_listener.setFilter(imageTask);

      // If this thumbnail is a replacement, add a Show original button under the thumb
      if (m_replacementFlag) {
            auto showSpr = ButtonSprite::create("Show original", 140, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 1.f);
            m_showOriginalBtn = CCMenuItemSpriteExtra::create(showSpr, this, menu_selector(ThumbnailInfoLayer::onShowOriginal));
            // position beneath the thumb background
            float btnX = thumbBg->getPositionX();
            float btnY = thumbBg->getPositionY() - thumbBg->getContentSize().height / 2.f - 26.f;
            m_showOriginalBtn->setPosition({btnX, btnY});
            menu->addChild(m_showOriginalBtn);
      }

      // Info box
      float panelX = screenSize.width * 0.55f + 75.f;
      float panelY = screenSize.height * 0.7f;
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
            auto acceptBtnSprite = ButtonSprite::create("Accept", 55, true, "bigFont.fnt", "GJ_button_01.png", 0.f, 1.f);
            auto acceptBtn = CCMenuItemSpriteExtra::create(acceptBtnSprite, this, menu_selector(ThumbnailInfoLayer::onAccept));
            acceptBtn->setPosition({panelX + 20.f, panelY - line - 35.f});

            auto rejectBtnSprite = ButtonSprite::create("Reject", 55, true, "bigFont.fnt", "GJ_button_06.png", 0.f, 1.f);
            auto rejectBtn = CCMenuItemSpriteExtra::create(rejectBtnSprite, this, menu_selector(ThumbnailInfoLayer::onReject));
            rejectBtn->setPosition({panelX + 120.f, panelY - line - 35.f});

            auto playBtnSprite = ButtonSprite::create("View Level", 130, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 1.f);
            auto playBtn = CCMenuItemSpriteExtra::create(playBtnSprite, this, menu_selector(ThumbnailInfoLayer::onPlayLevelButton));
            playBtn->setPosition({panelX + 70.f, panelY - line - 80.f});

            auto actionMenu = CCMenu::create();
            actionMenu->setPosition({0.f, 0.f});
            actionMenu->addChild(playBtn);
            actionMenu->addChild(acceptBtn);
            actionMenu->addChild(rejectBtn);

            this->addChild(actionMenu);
      }

      this->setKeypadEnabled(true);
      return true;
}

void ThumbnailInfoLayer::keyBackClicked() { onBackButton(nullptr); }

void ThumbnailInfoLayer::onBackButton(CCObject*) {
      CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void ThumbnailInfoLayer::onAccept(CCObject*) {
      auto req = web::WebRequest();
      req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
      req.bodyJSON(matjson::makeObject({{"accepted", true}}));
      auto task = req.post(fmt::format("https://levelthumbs.prevter.me/pending/{}", m_id));

      m_listener.bind([this](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->code() >= 200 && res->code() <= 299) {
                CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
                Notification::create("Thumbnail accepted.", NotificationIcon::Success)->show();
            } else {
                Notification::create("Failed to accept thumbnail.", NotificationIcon::Error)->show();
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
                    CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
                    Notification::create("Thumbnail rejected.", NotificationIcon::Success)->show();
                } else {
                    Notification::create("Failed to reject thumbnail.", NotificationIcon::Error)->show();
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

void ThumbnailInfoLayer::onShowOriginal(CCObject*) {
      // Toggle back to the replacement
      if (m_showingOriginal) {
            if (m_thumbOriginal) m_thumbOriginal->setVisible(false);
            if (m_thumbReplacement) m_thumbReplacement->setVisible(true);
            if (m_thumbLabel) m_thumbLabel->setString("Replacement");
            m_showingOriginal = false;
            // update the button label back to 'Show original'
            if (m_showOriginalBtn) {
                  auto newSprite = ButtonSprite::create("Show original", 140, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 1.f);
                  m_showOriginalBtn->setNormalImage(newSprite);
            }
            return;
      }

      // If original already loaded, just show it and hide replacement
      if (m_originalLoaded) {
            if (m_thumbOriginal) m_thumbOriginal->setVisible(true);
            if (m_thumbReplacement) m_thumbReplacement->setVisible(false);
            if (m_thumbLabel) m_thumbLabel->setString("Original");
            m_showingOriginal = true;
            if (m_showOriginalBtn) {
                  auto newSprite = ButtonSprite::create("Show replacement", 160, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 1.f);
                  m_showOriginalBtn->setNormalImage(newSprite);
            }
            return;
      }

      // Otherwise, fetch original image
      if (!m_thumbOriginal || !m_thumbSpinner) return;
      m_thumbSpinner->setVisible(true);
      auto req = web::WebRequest();
      req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
      auto task = req.get(fmt::format("https://levelthumbs.prevter.me/thumbnail/{}", m_levelId));

      m_listener.bind([this, task](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                  if (res->code() >= 200 && res->code() <= 299) {
                        auto data = res->data();
                        if (!data.empty()) {
                              if (m_thumbOriginal) {
                                    m_thumbOriginal->loadFromData(data);
                                    m_thumbOriginal->setVisible(true);
                                    if (m_thumbLabel) m_thumbLabel->setString("Original");
                              }
                              if (m_thumbReplacement) {
                                    m_thumbReplacement->setVisible(false);
                              }
                              m_originalLoaded = true;
                              m_showingOriginal = true;
                              if (m_thumbSpinner) m_thumbSpinner->setVisible(false);
                              if (m_showOriginalBtn) {
                                    auto newSprite = ButtonSprite::create("Show replacement", 160, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 1.f);
                                    m_showOriginalBtn->setNormalImage(newSprite);
                              }
                        }
                  } else {
                        log::error("Original image fetch error: {} {}", res->code(), res->string().unwrapOr(""));
                        if (m_thumbSpinner) m_thumbSpinner->setVisible(false);
                        Notification::create("Error loading original image.", NotificationIcon::Error)->show();
                  }
            }
      });
      m_listener.setFilter(task);
}
