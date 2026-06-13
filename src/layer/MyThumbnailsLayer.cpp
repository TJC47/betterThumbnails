#include "MyThumbnailsLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>

#include "../include/BetterThumbnailConstant.hpp"
#include "../node/ThumbnailNode.hpp"

using namespace geode::prelude;

MyThumbnailsLayer* MyThumbnailsLayer::create() {
    auto ret = new MyThumbnailsLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MyThumbnailsLayer::init() {
    if (!CCLayer::init()) {
        return false;
    }

    auto bg = createLayerBG();
    if (bg) {
        bg->setColor(ccc3(0, 122, 85));
        this->addChild(bg, -1);
    }
    addSideArt(this, SideArt::BottomLeft, false);
    addSideArt(this, SideArt::BottomRight, false);
    addBackButton(this, BackButtonStyle::Green);

    auto screenSize = CCDirector::sharedDirector()->getWinSize();

    auto listWidth = 356.f;
    auto listHeight = 220.f;
    m_listNode = cue::ListNode::create({listWidth, listHeight}, {191, 114, 62, 255}, cue::ListBorderStyle::SlimLevels);
    m_listNode->setAnchorPoint({0.5f, 0.5f});
    m_listNode->setPosition({screenSize.width / 2.f, screenSize.height / 2.f - 10.f});
    m_listNode->getScrollLayer()->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(0.f));
    this->addChild(m_listNode, 1);

    m_tabMenu = CCMenu::create();
    m_tabMenu->setPosition({m_listNode->getContentSize().width / 2.f, m_listNode->getContentSize().height + 28.f});
    m_tabMenu->setLayout(RowLayout::create()->setGap(20.f));
    m_listNode->addChild(m_tabMenu, -1);

    const float tabY = screenSize.height - 90.f;
    const float tabGap = 120.f;
    auto activeTab = TabButton::create(TabBaseColor::Unselected, TabBaseColor::UnselectedDark, "Active", this, menu_selector(MyThumbnailsLayer::onSelectActive));
    auto pendingTab = TabButton::create(TabBaseColor::Unselected, TabBaseColor::UnselectedDark, "Pending", this, menu_selector(MyThumbnailsLayer::onSelectPending));
    auto rejectedTab = TabButton::create(TabBaseColor::Unselected, TabBaseColor::UnselectedDark, "Rejected", this, menu_selector(MyThumbnailsLayer::onSelectRejected));

    m_activeTab = activeTab;
    m_pendingTab = pendingTab;
    m_rejectedTab = rejectedTab;

    activeTab->setPosition({screenSize.width / 2.f - tabGap, tabY});
    pendingTab->setPosition({screenSize.width / 2.f, tabY});
    rejectedTab->setPosition({screenSize.width / 2.f + tabGap, tabY});

    m_tabMenu->addChild(activeTab);
    m_tabMenu->addChild(pendingTab);
    m_tabMenu->addChild(rejectedTab);
    m_tabMenu->updateLayout();

    m_activeTab->toggle(true);
    m_pendingTab->toggle(false);
    m_rejectedTab->toggle(false);

    m_listNode->setCellHeight(100.f);
    m_loadingCircle = cue::LoadingCircle::create(true);
    m_loadingCircle->addToLayer(m_listNode, 2);

    m_navMenu = CCMenu::create();
    m_navMenu->setPosition({0.f, 0.f});
    this->addChild(m_navMenu, 2);

    CCSprite* prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    m_prevBtn = CCMenuItemSpriteExtra::create(prevSpr, this, menu_selector(MyThumbnailsLayer::onPrevPage));
    m_prevBtn->setPosition({20.f, screenSize.height / 2.f});
    m_navMenu->addChild(m_prevBtn);

    CCSprite* nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    m_nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(MyThumbnailsLayer::onNextPage));
    m_nextBtn->setPosition({screenSize.width - 20.f, screenSize.height / 2.f});
    m_navMenu->addChild(m_nextBtn);

    auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    m_reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr, this, menu_selector(MyThumbnailsLayer::onReload));
    m_reloadBtn->setPosition({screenSize.width - 30.f, 30.f});
    m_navMenu->addChild(m_reloadBtn);

    m_infoLabel = CCLabelBMFont::create("1 to 0 of 0", "goldFont.fnt");
    m_infoLabel->setScale(0.4f);
    m_infoLabel->setAnchorPoint({1.f, 1.f});
    m_infoLabel->setPosition({screenSize.width - 7.f, screenSize.height - 3.f});
    this->addChild(m_infoLabel, 3);

    fetchPage(m_currentPage);
    this->setKeypadEnabled(true);
    return true;
}

void MyThumbnailsLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void MyThumbnailsLayer::onSelectActive(CCObject*) {
    if (m_mode == UploadMode::Active) return;
    m_mode = UploadMode::Active;
    m_activeTab->toggle(true);
    m_pendingTab->toggle(false);
    m_rejectedTab->toggle(false);
    m_currentPage = 1;
    fetchPage(m_currentPage);
}

void MyThumbnailsLayer::onSelectPending(CCObject*) {
    if (m_mode == UploadMode::Pending) return;
    m_mode = UploadMode::Pending;
    m_activeTab->toggle(false);
    m_pendingTab->toggle(true);
    m_rejectedTab->toggle(false);
    m_currentPage = 1;
    fetchPage(m_currentPage);
}

void MyThumbnailsLayer::onSelectRejected(CCObject*) {
    if (m_mode == UploadMode::Rejected) return;
    m_mode = UploadMode::Rejected;
    m_activeTab->toggle(false);
    m_pendingTab->toggle(false);
    m_rejectedTab->toggle(true);
    m_currentPage = 1;
    fetchPage(m_currentPage);
}

void MyThumbnailsLayer::onPrevPage(CCObject*) {
    if (m_currentPage > 1) {
        fetchPage(m_currentPage - 1);
    }
}

void MyThumbnailsLayer::onNextPage(CCObject*) {
    fetchPage(m_currentPage + 1);
}

void MyThumbnailsLayer::onReload(CCObject*) {
    fetchPage(m_currentPage);
}

std::string MyThumbnailsLayer::endpointForMode(MyThumbnailsLayer::UploadMode mode) {
    switch (mode) {
        case MyThumbnailsLayer::UploadMode::Active: return "active";
        case MyThumbnailsLayer::UploadMode::Pending: return "pending";
        case MyThumbnailsLayer::UploadMode::Rejected: return "rejected";
    }
    return "active";
}

void MyThumbnailsLayer::fetchPage(int page) {
    m_currentPage = page;
    auto mode = endpointForMode(m_mode);
    auto req = betterThumbnail::createWebRequest();
    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
    auto url = betterThumbnail::makeUrl(fmt::format("/user/me/uploads/{}?page={}&per_page={}", mode, page, ITEMS_PER_PAGE));

    if (m_loadingCircle)
        m_loadingCircle->fadeIn();
    if (m_listNode)
        m_listNode->clear();
    if (m_navMenu)
        m_navMenu->setVisible(false);

    auto task = req.get(url);
    m_listener.spawn(std::move(task), [this](web::WebResponse res) {
        if (m_loadingCircle)
            m_loadingCircle->fadeOut();
        if (res.code() < 200 || res.code() > 299) {
            Notification::create(fmt::format("My uploads API error: {}", res.string().unwrapOrDefault()), NotificationIcon::Error)->show();
            return;
        }
        auto jsonResult = res.json();
        if (!jsonResult.isOk()) {
            Notification::create(fmt::format("My uploads parse error: {}", jsonResult.unwrapErr()), NotificationIcon::Error)->show();
            return;
        }
        auto json = jsonResult.unwrap();
        if (!json.isObject() || !json["uploads"].isArray()) {
            Notification::create("My uploads invalid response", NotificationIcon::Error)->show();
            return;
        }

        m_apiPerPage = json["per_page"].asInt().unwrapOr(ITEMS_PER_PAGE);
        m_apiTotal = json["total"].asInt().unwrapOr(0);
        m_currentPage = json["page"].asInt().unwrapOr(1);
        auto uploads = json["uploads"].asArray().unwrap();

        m_uploads.clear();
        m_uploads.reserve(uploads.size());
        for (auto const& item : uploads) {
            MyThumbnailEntry entry;
            entry.id = item["id"].asInt().unwrapOrDefault();
            entry.level_id = item["level_id"].asInt().unwrapOrDefault();
            entry.accepted_time = item["accepted_time"].asString().unwrapOr("");
            entry.upload_time = item["upload_time"].asString().unwrapOr("-");
            entry.submission_note = item["submission_note"].asString().unwrapOrDefault();
            m_uploads.push_back(std::move(entry));
        }

        updateUI();
    });
}

void MyThumbnailsLayer::updateUI() {
    if (!m_listNode)
        return;
    m_listNode->clear();

    auto currentUserId = Mod::get()->getSavedValue<int>("user_id");
    auto currentUsername = Mod::get()->getSavedValue<std::string>("username");
    for (auto const& entry : m_uploads) {
        if (entry.level_id <= 0) {
            continue;
        }
        auto thumbNode = ThumbnailNode::create(
            m_listNode->getContentSize(),
            entry.id,
            currentUserId,
            currentUsername,
            entry.level_id,
            m_mode == UploadMode::Active,
            entry.upload_time,
            false,
            entry.submission_note,
            currentUserId,
            entry.accepted_time,
            fmt::format("https://levelthumbs.prevter.me/thumbnail/{}", entry.level_id),
            ThumbnailNode::Mode::MyThumbnail);
        thumbNode->setAnchorPoint({0.5f, 1.0f});
        m_listNode->addCell(thumbNode);
    }

    m_listNode->updateLayout();
    m_listNode->scrollToTop();

    if (m_infoLabel) {
        int displayStart = m_uploads.empty() ? 0 : (m_apiTotal == 0 ? 0 : ((m_currentPage - 1) * (m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE) + 1));
        int displayEnd = m_uploads.empty() ? 0 : (displayStart + static_cast<int>(m_uploads.size()) - 1);
        m_infoLabel->setString(fmt::format("{} to {} of {}", displayStart, displayEnd, m_apiTotal).c_str());
    }

    if (m_navMenu) {
        bool hasPages = m_apiTotal > (m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE);
        m_navMenu->setVisible(hasPages);
        if (m_prevBtn) {
            m_prevBtn->setVisible(m_currentPage > 1);
            m_prevBtn->setEnabled(m_currentPage > 1);
        }
        if (m_nextBtn) {
            int totalPages = (m_apiTotal + (m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE) - 1) / (m_apiPerPage > 0 ? m_apiPerPage : ITEMS_PER_PAGE);
            m_nextBtn->setVisible(m_currentPage < totalPages);
            m_nextBtn->setEnabled(m_currentPage < totalPages);
        }
    }
}
