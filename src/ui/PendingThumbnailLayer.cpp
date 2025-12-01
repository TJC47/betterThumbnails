#include "PendingThumbnailLayer.hpp"

#include <Geode/Geode.hpp>

#include "../node/ThumbnailNode.hpp"
#include "BetterThumbnailLayer.hpp"

CCScene* PendingThumbnailLayer::scene() {
      auto scene = CCScene::create();
      scene->addChild(PendingThumbnailLayer::create());
      return scene;
}

bool PendingThumbnailLayer::init() {
      if (!CCLayer::init())
            return false;

      auto bg = createLayerBG();
      if (bg != nullptr) {
            this->addChild(bg, -1);
      } else {
            log::error("createLayerBG returned nullptr");
            return false;
      }

      addSideArt(this, SideArt::All, false);

      auto screenSize = CCDirector::sharedDirector()->getWinSize();

      CCMenu* menu = CCMenu::create();
      this->addChild(menu, 2);
      menu->setPosition(CCPointZero);

      GJListLayer* listLayer = GJListLayer::create(
          nullptr,               // BoomListView*
          "Pending Thumbnails",  // title
          {0, 0, 0, 100},
          356.f,  // width
          220.f,  // height
          0       // list type (default)
      );
      this->addChild(listLayer, 1);
      listLayer->setAnchorPoint(CCPoint(0.5f, 0.5f));
      listLayer->setPosition(CCPoint(screenSize / 2 - listLayer->getScaledContentSize() / 2));  // dont change this, its a werid way to center it

      ScrollLayer* scrollLayer = ScrollLayer::create(listLayer->getContentSize(), true, true);
      scrollLayer->setContentSize(listLayer->getContentSize());
      scrollLayer->setAnchorPoint({0.0f, 0.0f});
      listLayer->addChild(scrollLayer);
      scrollLayer->setContentSize(listLayer->getContentSize());

      LoadingSpinner* spinner = LoadingSpinner::create(100.f);
      spinner->setPosition(this->getContentSize().width / 2.f, this->getContentSize().height / 2.f);
      spinner->setTag(9999);
      this->addChild(spinner, 2);

      ColumnLayout* columnLayout = ColumnLayout::create();
      columnLayout->setAxisReverse(true);
      columnLayout->setAxisAlignment(AxisAlignment::Start);
      columnLayout->setCrossAxisAlignment(AxisAlignment::Start);
      columnLayout->setCrossAxisLineAlignment(AxisAlignment::Start);
      columnLayout->setAutoGrowAxis(true);

      auto contentLayer = scrollLayer->m_contentLayer;
      contentLayer->setContentSize(scrollLayer->getContentSize());
      contentLayer->setLayout(columnLayout);
      // store references for pagination
      m_scrollLayer = scrollLayer;
      m_contentLayer = contentLayer;

      m_navMenu = CCMenu::create();
      m_navMenu->setPosition({0.f, 0.f});
      this->addChild(m_navMenu, 2);

      m_filterMenu = CCMenu::create();
      m_filterMenu->setPosition({0.f, 0.f});
      m_filterMenu->setVisible(false);
      this->addChild(m_filterMenu, 2);

      m_infoLabel = CCLabelBMFont::create("1 to 1 of 0", "goldFont.fnt");
      m_infoLabel->setScale(0.4f);
      m_infoLabel->setAnchorPoint({0.5f, 1.0f});
      m_infoLabel->setPosition({screenSize.width / 2.f, screenSize.height - 2.f});
      this->addChild(m_infoLabel, 3);

      CCSprite* prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
      m_prevBtn = CCMenuItemSpriteExtra::create(prevSpr, this, menu_selector(PendingThumbnailLayer::onPrevPage));
      m_prevBtn->setPosition({20.f, screenSize.height / 2.f});
      m_navMenu->addChild(m_prevBtn);

      CCSprite* nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
      nextSpr->setFlipX(true);
      m_nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(PendingThumbnailLayer::onNextPage));
      m_nextBtn->setPosition({screenSize.width - 20.f, screenSize.height / 2.f});
      m_navMenu->addChild(m_nextBtn);

      // hide pagination controls until eaten the data
      m_navMenu->setVisible(false);

      auto listBottomY = screenSize.height / 2.f - listLayer->getContentSize().height / 2.f - 50.f;
      auto toggleY = listBottomY + 20.f;
      auto centerX = screenSize.width / 2.f;
      // 'All'
      // compute consistent width for all filter buttons
      float labelScale = 0.7f;
      auto measureLabel = [&](const char* text) -> float {
            auto tmp = CCLabelBMFont::create(text, "bigFont.fnt");
            tmp->setScale(labelScale);
            auto size = tmp->getContentSize();
            return size.width * labelScale;
      };
      auto widthAll = measureLabel("All");
      auto widthNew = measureLabel("New");
      auto widthRep = measureLabel("Replacement");

      // add some padding
      int commonDesiredPixels = (int)std::ceil(std::max({widthAll, widthNew, widthRep}) + 5.0f);

      // create a single button sprite and a MenuItem for 'All'
      int widthParam = 130.f;
      auto allSpr = ButtonSprite::create("All", widthParam, true, "bigFont.fnt", "GJ_button_01.png", 0.f, 1.f);
      allSpr->setScale(labelScale);
      m_allFilterBtn = CCMenuItemSpriteExtra::create(allSpr, this, menu_selector(PendingThumbnailLayer::onFilterAll));
      m_allFilterBtnSpr = allSpr;
      float gap = 1.f;
      float offset = (float)commonDesiredPixels + gap;
      m_allFilterBtn->setPosition({centerX - offset, toggleY});
      m_filterMenu->addChild(m_allFilterBtn);
      // mark all as selected by updating BG image
      if (m_allFilterBtnSpr) m_allFilterBtnSpr->updateBGImage("GJ_button_02.png");
      // 'New'
      auto newSpr = ButtonSprite::create("New", widthParam, true, "bigFont.fnt", "GJ_button_01.png", 0.f, 1.f);
      newSpr->setScale(labelScale);
      m_newFilterBtn = CCMenuItemSpriteExtra::create(newSpr, this, menu_selector(PendingThumbnailLayer::onFilterNew));
      m_newFilterBtnSpr = newSpr;
      m_newFilterBtn->setPosition({centerX, toggleY});
      m_filterMenu->addChild(m_newFilterBtn);
      if (m_newFilterBtnSpr) m_newFilterBtnSpr->updateBGImage("GJ_button_01.png");

      // 'Replacement'
      auto repSpr = ButtonSprite::create("Replacement", widthParam, true, "bigFont.fnt", "GJ_button_01.png", 0.f, 1.f);
      repSpr->setScale(labelScale);
      m_replacementFilterBtn = CCMenuItemSpriteExtra::create(repSpr, this, menu_selector(PendingThumbnailLayer::onFilterReplacement));
      m_replacementFilterBtnSpr = repSpr;
      m_replacementFilterBtn->setPosition({centerX + offset, toggleY});
      m_filterMenu->addChild(m_replacementFilterBtn);
      if (m_replacementFilterBtnSpr) m_replacementFilterBtnSpr->updateBGImage("GJ_button_01.png");

      // Fetch pending thumbnails from API
      auto req = web::WebRequest();
      req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
      auto task = req.get("https://levelthumbs.prevter.me/pending");
      m_listener.bind([this, scrollLayer, contentLayer, listLayer](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->code() < 200 || res->code() > 299) {
                log::error("Pending API error: {} {}", res->code(), res->string().unwrapOr(""));
                Notification::create(fmt::format("Pending API error: {}", res->string().unwrapOr("")), NotificationIcon::Error)->show();
                return;
            }
            auto json = res->json().unwrapOrDefault();
            if (!json.isArray()) {
                log::error("Pending API did not return an array");
                Notification::create("Pending API error: Invalid response format", NotificationIcon::Error)->show();
                return;
            }

            // Gather results into pending vector
            m_pendingItems.clear();
            for (auto& item : json) {
                auto id = item["id"].asInt().unwrapOrDefault();
                auto user_id = item["user_id"].asInt().unwrapOrDefault();
                auto username = item["username"].asString().unwrapOr("");
                auto level_id = item["level_id"].asInt().unwrapOrDefault();
                auto accepted = item["accepted"].asBool().unwrapOr(false);
                auto upload_time = item["upload_time"].asString().unwrapOr("");
                auto replacement = item["replacement"].asBool().unwrapOr(false);
                PendingThumbEntry e;
                e.id = id;
                e.user_id = user_id;
                e.username = username;
                e.level_id = level_id;
                e.accepted = accepted;
                e.upload_time = upload_time;
                e.replacement = replacement;
                m_pendingItems.push_back(e);
            }

            // Update the UI once all items are stored
            m_currentPage = 1;
            refreshPage();

            // Remove spinner when fetch completes
            auto spinner = this->getChildByTag(9999);
            if (spinner) spinner->setVisible(false);
            // Show filter menu now that data is loaded and ensure it reflects the current filter state
            if (m_filterMenu) {
                  m_filterMenu->setVisible(true);
                  switch (m_filterMode) {
                  case FilterMode::All:
                        if (m_allFilterBtnSpr) m_allFilterBtnSpr->updateBGImage("GJ_button_02.png");
                        if (m_newFilterBtnSpr) m_newFilterBtnSpr->updateBGImage("GJ_button_01.png");
                        if (m_replacementFilterBtnSpr) m_replacementFilterBtnSpr->updateBGImage("GJ_button_01.png");
                        break;
                  case FilterMode::NewOnly:
                        if (m_newFilterBtnSpr) m_newFilterBtnSpr->updateBGImage("GJ_button_02.png");
                        if (m_allFilterBtnSpr) m_allFilterBtnSpr->updateBGImage("GJ_button_01.png");
                        if (m_replacementFilterBtnSpr) m_replacementFilterBtnSpr->updateBGImage("GJ_button_01.png");
                        break;
                  case FilterMode::ReplacementOnly:
                        if (m_replacementFilterBtnSpr) m_replacementFilterBtnSpr->updateBGImage("GJ_button_02.png");
                        if (m_allFilterBtnSpr) m_allFilterBtnSpr->updateBGImage("GJ_button_01.png");
                        if (m_newFilterBtnSpr) m_newFilterBtnSpr->updateBGImage("GJ_button_01.png");
                        break;
                  }
            }
        } });
      m_listener.setFilter(task);

      // Back button at top left
      auto backButton = CCMenuItemSpriteExtra::create(
          CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
          this,
          menu_selector(PendingThumbnailLayer::onBackButton));
      backButton->setPosition({25.f, screenSize.height - 25.f});
      menu->addChild(backButton);

      this->setKeypadEnabled(true);

      return true;
}

PendingThumbnailLayer* PendingThumbnailLayer::create() {
      auto ret = new PendingThumbnailLayer;
      if (ret && ret->init()) {
            ret->autorelease();
            return ret;
      }
      CC_SAFE_DELETE(ret);
      return nullptr;
}

void PendingThumbnailLayer::keyBackClicked() {
      onBackButton(nullptr);
}

void PendingThumbnailLayer::onBackButton(CCObject*) {
      CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
}

void PendingThumbnailLayer::refreshPage() {
      if (!m_contentLayer || !m_scrollLayer) return;
      m_contentLayer->removeAllChildrenWithCleanup(true);

      // Apply filter to pending items
      std::vector<PendingThumbEntry> filtered;
      filtered.reserve(m_pendingItems.size());
      for (auto& it : m_pendingItems) {
            switch (m_filterMode) {
                  case PendingThumbnailLayer::FilterMode::All:
                        filtered.push_back(it);
                        break;
                  case PendingThumbnailLayer::FilterMode::NewOnly:
                        if (!it.replacement) filtered.push_back(it);
                        break;
                  case PendingThumbnailLayer::FilterMode::ReplacementOnly:
                        if (it.replacement) filtered.push_back(it);
                        break;
            }
      }
      auto total = (int)filtered.size();
      auto totalPages = (total + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
      if (totalPages <= 0) totalPages = 1;
      if (m_currentPage < 1) m_currentPage = 1;
      if (m_currentPage > totalPages) m_currentPage = totalPages;

      auto start = (m_currentPage - 1) * ITEMS_PER_PAGE;
      auto end = std::min(start + ITEMS_PER_PAGE, total);

      for (int i = start; i < end; ++i) {
            auto& item = filtered[i];
            auto thumbNode = ThumbnailNode::create(m_scrollLayer->getContentSize(), item.id, item.user_id, item.username, item.level_id, item.accepted, item.upload_time, item.replacement);
            if (thumbNode) {
                  thumbNode->setAnchorPoint({0.5f, 1.0f});
                  m_contentLayer->addChild(thumbNode);
            }
      }

      m_contentLayer->updateLayout();
      m_scrollLayer->scrollToTop();

      if (m_infoLabel) {
            auto displayStart = total == 0 ? 0 : (start + 1);
            auto displayEnd = total == 0 ? 0 : end;
            m_infoLabel->setString(fmt::format("{} to {} of {}", displayStart, displayEnd, total).c_str());
      }

      if (m_navMenu) {
            m_navMenu->setVisible(total > ITEMS_PER_PAGE);
            if (m_prevBtn) {
                  m_prevBtn->setEnabled(m_currentPage > 1);
                  m_prevBtn->setVisible(m_currentPage > 1);
            }
            if (m_nextBtn) {
                  m_nextBtn->setEnabled(m_currentPage < totalPages);
                  m_nextBtn->setVisible(m_currentPage < totalPages);
            }
      }
}

void PendingThumbnailLayer::onPrevPage(CCObject*) {
      if (m_currentPage > 1) {
            m_currentPage -= 1;
            refreshPage();
      }
}

void PendingThumbnailLayer::onNextPage(CCObject*) {
      // filtered total
      int filteredTotal = 0;
      for (auto& it : m_pendingItems) {
            switch (m_filterMode) {
                  case FilterMode::All:
                        ++filteredTotal;
                        break;
                  case FilterMode::NewOnly:
                        if (!it.replacement) ++filteredTotal;
                        break;
                  case FilterMode::ReplacementOnly:
                        if (it.replacement) ++filteredTotal;
                        break;
            }
      }
      int totalPages = (filteredTotal + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
      if (totalPages <= 0) totalPages = 1;
      if (m_currentPage < totalPages) {
            m_currentPage += 1;
            refreshPage();
      }
}

void PendingThumbnailLayer::onFilterAll(CCObject*) {
      // show all items
      m_filterMode = FilterMode::All;
      if (m_allFilterBtnSpr) m_allFilterBtnSpr->updateBGImage("GJ_button_02.png");
      if (m_replacementFilterBtnSpr) m_replacementFilterBtnSpr->updateBGImage("GJ_button_01.png");
      if (m_newFilterBtnSpr) m_newFilterBtnSpr->updateBGImage("GJ_button_01.png");
      m_currentPage = 1;
      refreshPage();
}

void PendingThumbnailLayer::onFilterReplacement(CCObject*) {
      // show replacement-only items
      m_filterMode = FilterMode::ReplacementOnly;
      if (m_replacementFilterBtnSpr) m_replacementFilterBtnSpr->updateBGImage("GJ_button_02.png");
      if (m_allFilterBtnSpr) m_allFilterBtnSpr->updateBGImage("GJ_button_01.png");
      if (m_newFilterBtnSpr) m_newFilterBtnSpr->updateBGImage("GJ_button_01.png");
      m_currentPage = 1;
      refreshPage();
}

void PendingThumbnailLayer::onFilterNew(CCObject*) {
      // show new items (not replacements)
      m_filterMode = FilterMode::NewOnly;
      if (m_newFilterBtnSpr) m_newFilterBtnSpr->updateBGImage("GJ_button_02.png");
      if (m_allFilterBtnSpr) m_allFilterBtnSpr->updateBGImage("GJ_button_01.png");
      if (m_replacementFilterBtnSpr) m_replacementFilterBtnSpr->updateBGImage("GJ_button_01.png");
      m_currentPage = 1;
      refreshPage();
}
