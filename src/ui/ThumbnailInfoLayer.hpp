#pragma once

#include <Geode/Geode.hpp>

#include "RejectReasonPopup.hpp"

using namespace geode::prelude;

class ThumbnailInfoLayer : public CCLayer {
     public:
      static CCScene* scene(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);
      static ThumbnailInfoLayer* create(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);

      bool init(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement);

      void keyBackClicked() override;
      void onBackButton(CCObject*);

      // Actions
      void onAccept(CCObject*);
      void onReject(CCObject*);
      void onPlayLevelButton(CCObject*);
      void onShowOriginal(CCObject*);

     private:
      // Stored for API calls
      int m_id = 0;
      int m_userId = 0;
      std::string m_username;
      int m_levelId = 0;
      bool m_acceptedFlag = false;
      std::string m_uploadTime;
      bool m_replacementFlag = false;

      EventListener<web::WebTask> m_listener;
      LazySprite* m_thumbReplacement = nullptr;
      LazySprite* m_thumbOriginal = nullptr;
      LoadingSpinner* m_thumbSpinner = nullptr;
      CCMenuItemSpriteExtra* m_showOriginalBtn = nullptr;
      bool m_showingOriginal = false;
      bool m_originalLoaded = false;
      CCLabelBMFont* m_thumbLabel = nullptr;
};
