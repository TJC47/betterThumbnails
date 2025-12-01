#include "ThumbnailNode.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/LazySprite.hpp>

#include "../ui/ThumbnailInfoLayer.hpp"

using namespace geode::prelude;

ThumbnailNode* ThumbnailNode::create(const CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
      auto ret = new ThumbnailNode();
      if (ret && ret->init(size, id, user_id, username, level_id, accepted, upload_time, replacement)) {
            ret->autorelease();
            return ret;
      }
      CC_SAFE_DELETE(ret);
      return nullptr;
}

bool ThumbnailNode::init(const CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
      if (!CCLayer::init())
            return false;

      auto contentHeight = 100.f;  // node height
      this->setContentSize({size.width, contentHeight});
      this->ignoreAnchorPointForPosition(false);
      this->setAnchorPoint({0.5f, 1.0f});

      auto thumbnailBg = CCScale9Sprite::create("square02_small.png");
      thumbnailBg->setOpacity(100);
      thumbnailBg->setContentSize({size.width, 100.f});
      thumbnailBg->setScale(0.95f);
      thumbnailBg->setAnchorPoint({0.5f, 0.5f});
      thumbnailBg->setPosition({size.width / 2.f, contentHeight / 2.f});
      this->addChild(thumbnailBg);

      auto lazySprite = LazySprite::create({180.f, 101.25f}, true);
      lazySprite->setVisible(false);
      lazySprite->setAutoResize(true);

      auto stencil = CCScale9Sprite::create("square02_small.png");
      stencil->setContentSize(thumbnailBg->getContentSize());
      stencil->setScale(1.0f);
      stencil->setAnchorPoint({0.5f, 0.5f});
      stencil->setPosition({thumbnailBg->getContentSize().width / 2.f, thumbnailBg->getContentSize().height / 2.f});

      auto clip = CCClippingNode::create(stencil);
      // make clip centered and aligned exactly with thumbnail background
      clip->setAnchorPoint({0.5f, 0.5f});
      clip->setAlphaThreshold(0.05f);
      thumbnailBg->addChild(clip);
      // position clip in the center of the thumbnail background so stencil size matches
      lazySprite->setPosition({thumbnailBg->getContentSize().width / 2.f + 85.f, thumbnailBg->getContentSize().height / 2.f});  // offset to the right a bit
      // // backdrop lazy sprite to display the same image behind the thumbnail inside the same clip
      // auto backdropSprite = LazySprite::create({960, 540}, false);
      // backdropSprite->setVisible(false);
      // backdropSprite->setAnchorPoint({0.5f, 0.5f});
      // backdropSprite->setPosition({thumbnailBg->getContentSize().width / 2.f, thumbnailBg->getContentSize().height / 2.f});
      // backdropSprite->setOpacity(100);
      // clip->addChild(backdropSprite, -1);  // add behind other clip children (add before lazySprite filled when executed)
      // m_backdrop = backdropSprite;
      clip->addChild(lazySprite);
      auto spinner = LoadingSpinner::create(30.f);
      spinner->setPosition(lazySprite->getPosition());
      clip->addChild(spinner);
      spinner->setVisible(true);

      float rightX = 20.f;
      float baseY = contentHeight / 2.f;
      float usernameY = baseY + 35.f;
      float levelIdY = usernameY - 15.f;
      float replacementY = levelIdY - 15.f;

      auto userInfo = username + std::string(" (") + std::to_string(user_id) + ")";
      auto usernameLabel = CCLabelBMFont::create(userInfo.c_str(), "goldFont.fnt");
      usernameLabel->setAnchorPoint({0.f, 0.5f});
      usernameLabel->setPosition({rightX, usernameY});
      usernameLabel->setScale(0.5f);
      this->addChild(usernameLabel);

      auto levelIdText = std::string("Level ID: ") + std::to_string(level_id);
      auto levelIdLabel = CCLabelBMFont::create(levelIdText.c_str(), "bigFont.fnt");
      levelIdLabel->setAnchorPoint({0.f, 0.5f});
      levelIdLabel->setPosition({rightX, levelIdY});
      levelIdLabel->setScale(0.3f);
      this->addChild(levelIdLabel);

      auto replacementText = std::string("Replacement: ") + (replacement ? "Yes" : "No");
      auto replacementLabel = CCLabelBMFont::create(replacementText.c_str(), "bigFont.fnt");
      replacementLabel->setAnchorPoint({0.f, 0.5f});
      replacementLabel->setPosition({rightX, replacementY});
      replacementLabel->setScale(0.3f);
      this->addChild(replacementLabel);

      // Store data for info view
      m_thumbId = id;
      m_userId = user_id;
      m_username = username;
      m_levelId = level_id;
      m_accepted = accepted;
      m_uploadTime = upload_time;
      m_replacement = replacement;

      auto idText = std::string("ThumbID: ") + std::to_string(id);
      auto idLabel = CCLabelBMFont::create(idText.c_str(), "bigFont.fnt");
      idLabel->setAnchorPoint({0.f, 0.5f});
      float idLabelY = replacementY - 15.f;
      idLabel->setPosition({rightX, idLabelY});
      idLabel->setScale(0.3f);
      this->addChild(idLabel);

      auto buttonSprite = ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png", 0.8f);
      buttonSprite->setScale(0.65f);
      auto viewBtn = CCMenuItemSpriteExtra::create(
          buttonSprite,
          nullptr,
          this,
          menu_selector(ThumbnailNode::onViewButton));
      viewBtn->setAnchorPoint({0.f, 0.5f});
      viewBtn->setPosition({rightX, idLabelY - 20.f});

      auto playBtnSprite = ButtonSprite::create("Play Level", "goldFont.fnt", "GJ_button_01.png", 0.8f);
      playBtnSprite->setScale(0.65f);
      auto playBtn = CCMenuItemSpriteExtra::create(playBtnSprite, nullptr, this, menu_selector(ThumbnailNode::onPlayLevelButton));
      playBtn->setAnchorPoint({0.f, 0.5f});
      playBtn->setPosition({rightX + 50.f, idLabelY - 20.f});

      auto menu = CCMenu::create();
      menu->addChild(viewBtn);
      menu->addChild(playBtn);
      menu->setPosition({0.f, 0.f});
      this->addChild(menu);

      float bgWidth = thumbnailBg->getContentSize().width;
      auto uploadTimeLabel = CCLabelBMFont::create(upload_time.c_str(), "chatFont.fnt");
      uploadTimeLabel->setAnchorPoint({1.f, 0.f});
      uploadTimeLabel->setPosition({bgWidth - 15.f, 10.f});
      uploadTimeLabel->setScale(0.35f);
      this->addChild(uploadTimeLabel);

      // Fetch image from API and apply to LazySprite
      auto imageReq = web::WebRequest();
      imageReq.header("Authorization", std::string("Bearer ") + Mod::get()->getSavedValue<std::string>("token"));
      auto imageTask = imageReq.get(std::string("https://levelthumbs.prevter.me/pending/") + std::to_string(id) + "/image");
      m_listener.bind([this, lazySprite, thumbnailBg, spinner](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                  if (res->code() >= 200 && res->code() <= 299) {
                        auto data = res->data();
                        if (!data.empty()) {
                              lazySprite->loadFromData(data);
                              // if (m_backdrop) {
                              //       m_backdrop->loadFromData(data);
                              //       m_backdrop->setVisible(true);
                              //       m_backdrop->setOpacity(100);
                              // }
                              auto contentSize = lazySprite->getContentSize();
                              if (contentSize.width > 0 && contentSize.height > 0) {
                                    // force a fixed scale for list thumbnails so they are consistent
                                    lazySprite->setPosition({thumbnailBg->getContentSize().width / 2.f + 85.f, thumbnailBg->getContentSize().height / 2.f});
                                    lazySprite->setVisible(true);
                                    if (spinner) spinner->setVisible(false);
                              }
                        }
                  } else {
                        log::error("Image fetch error: {} {}", res->code(), res->string().unwrapOr(""));
                        if (spinner) spinner->setVisible(false);
                  }
            }
      });
      m_listener.setFilter(imageTask);

      /*
      auto info = fmt::format(
          "id: {}, user_id: {}, username: {}, level_id: {}\naccepted: {}\nupload_time: {}\nreplacement: {}",
          id, user_id, username, level_id, accepted ? "true" : "false", upload_time, replacement ? "true" : "false"
      );
      auto label = cocos2d::CCLabelBMFont::create(info.c_str(), "goldFont.fnt");
      label->setAnchorPoint({0.5f, 0.5f});
      label->setPosition({size.width / 2.f, 0.f});
      label->setScale(0.35f);
      this->addChild(label);
      */

      return true;
}

void ThumbnailNode::onViewButton(CCObject*) {
      CCDirector::get()->pushScene(
          CCTransitionFade::create(
              .5f,
              ThumbnailInfoLayer::scene(m_thumbId, m_userId, m_username, m_levelId, m_accepted, m_uploadTime, m_replacement)));
}

void ThumbnailNode::onPlayLevelButton(CCObject*) {
      auto search = GJSearchObject::create(SearchType::Type19, std::to_string(m_levelId));
      CCDirector::get()->pushScene(CCTransitionFade::create(.5f, LevelBrowserLayer::scene(search)));
}
