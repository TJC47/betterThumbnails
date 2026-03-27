#include "PendingThumbnailLayer.hpp"

#include <Geode/Geode.hpp>

#include "../node/ThumbnailNode.hpp"
#include "FilterThumbnailPopup.hpp"
#include "Geode/ui/General.hpp"

bool PendingThumbnailLayer::init() {
    if (!CCLayer::init())
        return false;

    auto bg = createLayerBG();
    if (bg)
        this->addChild(bg, -1);
    else
        return false;

    addSideArt(this, SideArt::All, false);
    auto screenSize = CCDirector::sharedDirector()->getWinSize();

    CCMenu* menu = CCMenu::create();
    this->addChild(menu, 2);
    menu->setPosition(CCPointZero);

    float listWidth = 356.f;
    float listHeight = 220.f;
    m_listNode = cue::ListNode::create({listWidth, listHeight}, {0, 0, 0, 100}, cue::ListBorderStyle::SlimLevels);
    m_listNode->setAnchorPoint({0.5f, 0.5f});
    m_listNode->setPosition({screenSize.width / 2.f, screenSize.height / 2.f - 10.f});
    m_listNode->getScrollLayer()->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(0.f));
    this->addChild(m_listNode, 1);

    m_listNode->setCellHeight(100.f);

    LoadingSpinner* spinner = LoadingSpinner::create(100.f);
    spinner->setPosition(this->getContentSize().width / 2.f,
        this->getContentSize().height / 2.f);
    spinner->setTag(9999);
    this->addChild(spinner, 2);

    m_navMenu = CCMenu::create();
    m_navMenu->setPosition({0.f, 0.f});
    this->addChild(m_navMenu, 2);

    m_filterMenu = CCMenu::create();
    m_filterMenu->setContentSize(m_listNode->getContentSize());
    m_filterMenu->setPosition({0, 0});
    m_listNode->addChild(m_filterMenu, -1);

    m_infoLabel = CCLabelBMFont::create("1 to 1 of 0", "goldFont.fnt");
    m_infoLabel->setScale(0.4f);
    m_infoLabel->setAnchorPoint({0.5f, 1.0f});
    m_infoLabel->setPosition({screenSize.width / 2.f, screenSize.height - 2.f});
    this->addChild(m_infoLabel, 3);

    CCSprite* prevSpr =
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    m_prevBtn = CCMenuItemSpriteExtra::create(
        prevSpr, this, menu_selector(PendingThumbnailLayer::onPrevPage));
    m_prevBtn->setPosition({20.f, screenSize.height / 2.f});
    m_navMenu->addChild(m_prevBtn);

    CCSprite* nextSpr =
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    m_nextBtn = CCMenuItemSpriteExtra::create(
        nextSpr, this, menu_selector(PendingThumbnailLayer::onNextPage));
    m_nextBtn->setPosition({screenSize.width - 20.f, screenSize.height / 2.f});
    m_navMenu->addChild(m_nextBtn);

    // Hide pagination until data received
    m_navMenu->setVisible(false);

    auto toggleY = m_filterMenu->getContentHeight() + 30.f;
    auto centerX = m_filterMenu->getContentSize().width / 2.f;

    const float offset = 120.f;

    auto allBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::UnselectedDark, "All", this, menu_selector(PendingThumbnailLayer::onFilterAll));
    m_allFilterBtn = allBtn;
    m_allFilterBtn->setPosition({centerX - offset, toggleY});
    m_filterMenu->addChild(m_allFilterBtn);

    auto newBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::UnselectedDark, "New", this, menu_selector(PendingThumbnailLayer::onFilterNew));
    m_newFilterBtn = newBtn;
    m_newFilterBtn->setPosition({centerX, toggleY});
    m_filterMenu->addChild(m_newFilterBtn);

    auto repBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::UnselectedDark, "Replacement", this, menu_selector(PendingThumbnailLayer::onFilterReplacement));
    m_replacementFilterBtn = repBtn;
    m_replacementFilterBtn->setPosition({centerX + offset, toggleY});
    m_filterMenu->addChild(m_replacementFilterBtn);

    // select default filter
    m_allFilterBtn->toggle(true);
    m_newFilterBtn->toggle(false);
    m_replacementFilterBtn->toggle(false);

    // @geode-ignore(unknown-resource)
    auto searchIcon = CCSprite::createWithSpriteFrameName("geode.loader/search.png");
    auto searchSpr = AccountButtonSprite::create(
        searchIcon, AccountBaseColor::Gray, AccountBaseSize::Normal);
    m_searchFilterBtn = CCMenuItemSpriteExtra::create(
        searchSpr, this, menu_selector(PendingThumbnailLayer::onOpenFilterPopup));
    m_searchFilterBtnSpr = searchSpr;
    m_searchFilterBtn->setPosition(
        {screenSize.width - 30.f, screenSize.height - 30.f});
    m_navMenu->addChild(m_searchFilterBtn);

    addBackButton(this, BackButtonStyle::Green);

    // Start fetching the first page from server
    fetchPage(m_currentPage);

    this->setKeypadEnabled(true);
    return true;
}

PendingThumbnailLayer* PendingThumbnailLayer::create() {
    auto ret = new PendingThumbnailLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void PendingThumbnailLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f,
        PopTransition::kPopTransitionFade);
}

void PendingThumbnailLayer::onPrevPage(CCObject*) {
    if (m_currentPage > 1) {
        --m_currentPage;
        fetchPage(m_currentPage);
    }
}
void PendingThumbnailLayer::onNextPage(CCObject*) {
    fetchPage(m_currentPage + 1);
}

void PendingThumbnailLayer::onFilterAll(CCObject*) {
    m_filterMode = FilterMode::All;
    m_currentPage = 1;
    if (m_allFilterBtn)
        m_allFilterBtn->toggle(true);
    if (m_newFilterBtn)
        m_newFilterBtn->toggle(false);
    if (m_replacementFilterBtn)
        m_replacementFilterBtn->toggle(false);
    m_queryUsername.clear();
    m_queryHasLevelId = false;
    m_queryLevelId = 0;
    fetchPage(m_currentPage);
}
void PendingThumbnailLayer::onFilterReplacement(CCObject*) {
    m_filterMode = FilterMode::ReplacementOnly;
    m_currentPage = 1;
    if (m_allFilterBtn)
        m_allFilterBtn->toggle(false);
    if (m_newFilterBtn)
        m_newFilterBtn->toggle(false);
    if (m_replacementFilterBtn)
        m_replacementFilterBtn->toggle(true);
    m_queryUsername.clear();
    m_queryHasLevelId = false;
    m_queryLevelId = 0;
    fetchPage(m_currentPage);
}
void PendingThumbnailLayer::onFilterNew(CCObject*) {
    m_filterMode = FilterMode::NewOnly;
    m_currentPage = 1;
    if (m_allFilterBtn)
        m_allFilterBtn->toggle(false);
    if (m_newFilterBtn)
        m_newFilterBtn->toggle(true);
    if (m_replacementFilterBtn)
        m_replacementFilterBtn->toggle(false);
    m_queryUsername.clear();
    m_queryHasLevelId = false;
    m_queryLevelId = 0;
    fetchPage(m_currentPage);
}

void PendingThumbnailLayer::onOpenFilterPopup(CCObject*) {
    auto popup = FilterThumbnailPopup::create(
        [this](std::string username, bool hasLevel, int levelId) {
            this->m_queryUsername = std::move(username);
            this->m_queryHasLevelId = hasLevel;
            this->m_queryLevelId = levelId;
            this->m_currentPage = 1;
            this->fetchPage(this->m_currentPage);
        });
    if (popup)
        popup->show();
}

void PendingThumbnailLayer::fetchPage(int page) {
    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));
    auto url = fmt::format(
        "https://levelthumbs.prevter.me/pending?page={}&per_page=12", page);
    switch (this->m_filterMode) {
        case FilterMode::NewOnly:
            url += "&new_only=true";
            break;
        case FilterMode::ReplacementOnly:
            url += "&replacement_only=true";
            break;
        case FilterMode::All:
        default:
            break;
    }
    // Apply optional query filters (username, level id)
    if (!m_queryUsername.empty()) {
        // naive url encoding for spaces
        auto encoded = m_queryUsername;
        for (auto& c : encoded)
            if (c == ' ')
                c = '+';
        url += fmt::format("&username={}", encoded);
    }
    if (m_queryHasLevelId) {
        url += fmt::format("&level_id={}", m_queryLevelId);
    }
    auto token = Mod::get()->getSavedValue<std::string>("token");
    if (token.empty()) {
        log::warn("Pending API request missing token");
    }
    if (auto spinner = this->getChildByTag(9999))
        spinner->setVisible(true);
    auto task = req.get(url);
    m_listener.spawn(std::move(task), [this, url, token](web::WebResponse res) {
        if (res.code() < 200 || res.code() > 299) {
            auto spinner = this->getChildByTag(9999);
            if (spinner)
                spinner->setVisible(false);
            log::error("Pending API error: {} {}", res.code(), res.string().unwrapOr(""));
            Notification::create(
                fmt::format("Pending API error: {}", res.string().unwrapOr("")),
                NotificationIcon::Error)
                ->show();
            return;
        }
        auto body = res.string().unwrapOrDefault();
        auto jsonResult = res.json();
        if (!jsonResult.isOk()) {
            auto spinner = this->getChildByTag(9999);
            if (spinner)
                spinner->setVisible(false);
            auto maxLen = size_t(1024);
            auto truncated =
                body.size() > maxLen ? body.substr(0, maxLen) + "..." : body;
            log::error("Pending API JSON parse error: {}. Body: {}",
                jsonResult.unwrapErr(),
                truncated);
            Notification::create(
                fmt::format("Pending API error: JSON parse error: {}",
                    jsonResult.unwrapErr()),
                NotificationIcon::Error)
                ->show();
            return;
        }
        auto json = jsonResult.unwrap();
        if (json.isObject() && json["uploads"].isArray()) {
            this->m_serverPaging = true;
            this->m_apiPerPage = json["per_page"].asInt().unwrapOr(ITEMS_PER_PAGE);
            this->m_apiTotal = json["total"].asInt().unwrapOr(0);
            this->m_currentPage = json["page"].asInt().unwrapOr(1);
            auto arr = json["uploads"].asArray().copied().unwrapOrDefault();
            this->m_pendingItems.clear();
            for (auto& item : arr) {
                PendingThumbEntry e;
                e.id = item["id"].asInt().unwrapOrDefault();
                e.user_id = item["user_id"].asInt().unwrapOrDefault();
                e.username = item["username"].asString().unwrapOr("");
                e.level_id = item["level_id"].asInt().unwrapOrDefault();
                e.accepted = item["accepted"].asBool().unwrapOr(false);
                e.upload_time = item["upload_time"].asString().unwrapOr("");
                e.replacement = item["replacement"].asBool().unwrapOr(false);
                this->m_pendingItems.push_back(e);
            }
            log::info("Pending API parsed {} uploads (server paging)",
                this->m_pendingItems.size());
        } else {
            auto spinner = this->getChildByTag(9999);
            if (spinner)
                spinner->setVisible(false);
            auto body = res.string().unwrapOrDefault();
            auto truncated = body.size() > 1024 ? body.substr(0, 1024) + "..." : body;
            log::error(
                "Pending API did not return expected uploads object with "
                "'uploads' array. Response truncated: {}",
                truncated);
            Notification::create("Pending API error: Invalid response format",
                NotificationIcon::Error)
                ->show();
            return;
        }

        this->m_currentPage = this->m_currentPage < 1 ? 1 : this->m_currentPage;
        this->updateUI();
        auto spinner = this->getChildByTag(9999);
        if (spinner)
            spinner->setVisible(false);
    });
    if (auto spinner = this->getChildByTag(9999))
        spinner->setVisible(true);
    if (m_listNode)
        m_listNode->clear();
    if (m_navMenu)
        m_navMenu->setVisible(false);
}

void PendingThumbnailLayer::updateUI() {
    if (!m_listNode)
        return;
    m_listNode->clear();

    std::vector<PendingThumbEntry> filtered;
    filtered.reserve(m_pendingItems.size());
    for (auto& it : m_pendingItems) {
        switch (m_filterMode) {
            case PendingThumbnailLayer::FilterMode::All:
                filtered.push_back(it);
                break;
            case PendingThumbnailLayer::FilterMode::NewOnly:
                if (!it.replacement)
                    filtered.push_back(it);
                break;
            case PendingThumbnailLayer::FilterMode::ReplacementOnly:
                if (it.replacement)
                    filtered.push_back(it);
                break;
        }
    }
    int total = 0;
    int totalPages = 1;
    int start = 0;
    int end = static_cast<int>(filtered.size());
    total = m_apiTotal;
    int perPage = m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE;
    totalPages = (total + perPage - 1) / perPage;
    if (totalPages <= 0)
        totalPages = 1;
    if (m_currentPage < 1)
        m_currentPage = 1;
    if (m_currentPage > totalPages)
        m_currentPage = totalPages;
    start = 0;
    end = static_cast<int>(filtered.size());

    for (int i = start; i < end; ++i) {
        auto& item = filtered[i];
        auto thumbNode = ThumbnailNode::create(
            m_listNode->getContentSize(), item.id, item.user_id, item.username, item.level_id, item.accepted, item.upload_time, item.replacement);
        thumbNode->setAnchorPoint({0.5f, 1.0f});
        m_listNode->addCell(thumbNode);
    }

    m_listNode->updateLayout();
    m_listNode->scrollToTop();

    if (m_infoLabel) {
        int displayStart = 0;
        int displayEnd = 0;
        if (m_serverPaging) {
            displayStart =
                total == 0 ? 0
                           : ((m_currentPage - 1) *
                                     (m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE) +
                                 1);
            displayEnd =
                total == 0 ? 0
                           : ((m_currentPage - 1) *
                                     (m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE) +
                                 end);
        } else {
            displayStart = total == 0 ? 0 : (start + 1);
            displayEnd = total == 0 ? 0 : end;
        }
        m_infoLabel->setString(
            fmt::format("{} to {} of {}", displayStart, displayEnd, total).c_str());
    }

    if (m_navMenu) {
        if (m_serverPaging) {
            m_navMenu->setVisible(total >
                                  (m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE));
            if (m_prevBtn) {
                m_prevBtn->setEnabled(m_currentPage > 1);
                m_prevBtn->setVisible(m_currentPage > 1);
            }
            if (m_nextBtn) {
                m_nextBtn->setEnabled(m_currentPage < totalPages);
                m_nextBtn->setVisible(m_currentPage < totalPages);
            }
        } else {
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
}
