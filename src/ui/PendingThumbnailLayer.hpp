#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

struct PendingThumbEntry {
      int id = 0;
      int user_id = 0;
      std::string username;
      int level_id = 0;
      bool accepted = false;
      std::string upload_time;
      bool replacement = false;
};

class PendingThumbnailLayer : public CCLayer {
     public:
      EventListener<web::WebTask> m_listener;
      static PendingThumbnailLayer* create();
      static CCScene* scene();
      bool init() override;
      void onBackButton(CCObject*);
      void keyBackClicked() override;

     private:
      enum class FilterMode {
            All = 0,
            NewOnly = 1,
            ReplacementOnly = 2,
      };
      static constexpr int ITEMS_PER_PAGE = 20;
      int m_currentPage = 1;
      std::vector<PendingThumbEntry> m_pendingItems;

      ScrollLayer* m_scrollLayer = nullptr;
      cocos2d::CCLayer* m_contentLayer = nullptr;
      CCLabelBMFont* m_pageLabel = nullptr;
      CCLabelBMFont* m_infoLabel = nullptr;
      CCMenu* m_navMenu = nullptr;
      CCMenu* m_filterMenu = nullptr;
      CCMenuItemSpriteExtra* m_prevBtn = nullptr;
      CCMenuItemSpriteExtra* m_nextBtn = nullptr;

      CCMenuItemSpriteExtra* m_allFilterBtn = nullptr;
      CCMenuItemSpriteExtra* m_newFilterBtn = nullptr;
      CCMenuItemSpriteExtra* m_replacementFilterBtn = nullptr;

      ButtonSprite* m_allFilterBtnSpr = nullptr;
      ButtonSprite* m_newFilterBtnSpr = nullptr;
      ButtonSprite* m_replacementFilterBtnSpr = nullptr;
      FilterMode m_filterMode = FilterMode::All;

      void refreshPage();
      void onPrevPage(CCObject*);
      void onNextPage(CCObject*);
      void onFilterAll(CCObject*);
      void onFilterNew(CCObject*);
      void onFilterReplacement(CCObject*);
};
