#include "ManageUserLayer.hpp"

#include <Geode/binding/ProfilePage.hpp>

#include <cue/ListNode.hpp>

#include "Geode/ui/General.hpp"
#include "../include/BetterThumbnailConstant.hpp"

using namespace geode::prelude;

namespace {
    CCSprite* badgeForRole(const std::string& role) {
        if (role == "owner")
            return CCSprite::create("BT_ownerBadge.png"_spr);
        if (role == "admin")
            return CCSprite::create("BT_adminBadge.png"_spr);
        if (role == "moderator")
            return CCSprite::create("BT_modBadge.png"_spr);
        if (role == "verified")
            return CCSprite::create("BT_verifiedBadge.png"_spr);
        return nullptr;
    }
}

CCSprite* ManageUserLayer::spriteForRole(const std::string& role) {
    return badgeForRole(role);
}

ManageUserLayer* ManageUserLayer::create() {
    auto ret = new ManageUserLayer;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ManageUserLayer::init() {
    if (!CCLayer::init())
        return false;

    auto bg = createLayerBG();
    if (bg)
        this->addChild(bg, -1);

    addSideArt(this, SideArt::All, false);

    auto screenSize = CCDirector::sharedDirector()->getWinSize();

    auto listWidth = 356.f;
    auto listHeight = 220.f;
    this->m_listNode = cue::ListNode::create(
        {listWidth, listHeight}, {0, 0, 0, 90}, cue::ListBorderStyle::Levels);
    this->m_listNode->setAnchorPoint({0.5f, 0.5f});
    this->m_listNode->setPosition({screenSize.width / 2.f, screenSize.height / 2.f - 5.f});
    this->m_listNode->getScrollLayer()->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(0.f));
    this->m_listNode->setCellHeight(40.f);
    this->addChild(this->m_listNode, 1);

    this->m_infoLabel = CCLabelBMFont::create("0 users", "goldFont.fnt");
    this->m_infoLabel->setScale(0.35f);
    this->m_infoLabel->setAnchorPoint({0.5f, .0f});
    this->m_infoLabel->setPosition({screenSize.width / 2.f, 2.f});
    this->addChild(this->m_infoLabel, 2);

    this->m_pageLabel = CCLabelBMFont::create("Page 1 / 1", "goldFont.fnt");
    this->m_pageLabel->setScale(0.5f);
    this->m_pageLabel->setAnchorPoint({1.f, 1.f});
    this->m_pageLabel->setPosition({screenSize.width - 5.f, screenSize.height - 5.f});
    this->addChild(this->m_pageLabel, 2);

    this->m_navMenu = CCMenu::create();
    this->m_navMenu->setPosition({0.f, 0.f});
    this->addChild(this->m_navMenu, 2);

    auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    this->m_prevBtn = CCMenuItemSpriteExtra::create(
        prevSpr, this, menu_selector(ManageUserLayer::onPrevPage));
    this->m_prevBtn->setPosition({20.f, screenSize.height / 2.f});
    this->m_navMenu->addChild(this->m_prevBtn);

    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    this->m_nextBtn = CCMenuItemSpriteExtra::create(
        nextSpr, this, menu_selector(ManageUserLayer::onNextPage));
    this->m_nextBtn->setPosition({screenSize.width - 20.f, screenSize.height / 2.f});
    this->m_navMenu->addChild(this->m_nextBtn);

    addBackButton(this, BackButtonStyle::Green);
    this->setKeypadEnabled(true);

    fetchPage(m_currentPage);
    return true;
}

void ManageUserLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(
        0.5f, PopTransition::kPopTransitionFade);
}

void ManageUserLayer::onPrevPage(CCObject*) {
    if (this->m_currentPage > 1)
        fetchPage(m_currentPage - 1);
}

void ManageUserLayer::onNextPage(CCObject*) {
    if (this->m_currentPage < this->m_totalPages)
        fetchPage(m_currentPage + 1);
}

void ManageUserLayer::fetchPage(int page) {
    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));

    auto url = betterThumbnail::makeUrl(
        fmt::format("/admin/users?page={}&per_page=10&sort_by=id&sort_dir=asc", page));

    auto task = req.get(url);
    this->m_listener.spawn(std::move(task), [this, page](web::WebResponse res) {
        if (res.code() < 200 || res.code() > 299) {
            Notification::create(
                fmt::format("Failed to load users: {}", res.string().unwrapOrDefault()),
                NotificationIcon::Error)
                ->show();
            return;
        }

        auto jsonResult = res.json();
        if (!jsonResult.isOk()) {
            Notification::create("Failed to parse users response", NotificationIcon::Error)->show();
            return;
        }

        auto json = jsonResult.unwrap();
        if (!json.isObject() || !json["users"].isArray()) {
            Notification::create("Invalid users response", NotificationIcon::Error)->show();
            return;
        }

        this->m_currentPage = json["page"].asInt().unwrapOr(page);
        this->m_totalPages = json["total_pages"].asInt().unwrapOr(1);
        this->m_totalUsers = json["total"].asInt().unwrapOr(0);

        this->m_users.clear();
        for (auto& item : json["users"].asArray().copied().unwrapOrDefault()) {
            ManageUserEntry entry;
            entry.id = item["id"].asInt().unwrapOrDefault();
            entry.accountId = item["account_id"].asInt().unwrapOrDefault();
            entry.username = item["username"].asString().unwrapOr("Unknown");
            entry.role = item["role"].asString().unwrapOr("user");
            entry.accepted = item["accepted"].asInt().unwrapOrDefault();
            entry.activeThumbnails = item["active_thumbnails"].asInt().unwrapOrDefault();
            entry.pending = item["pending"].asInt().unwrapOrDefault();
            entry.rejected = item["rejected"].asInt().unwrapOrDefault();
            entry.totalUploads = item["total_uploads"].asInt().unwrapOrDefault();
            this->m_users.push_back(std::move(entry));
        }

        populateList();
    });
}

void ManageUserLayer::populateList() {
    if (!this->m_listNode)
        return;

    this->m_listNode->clear();
    this->m_listNode->setCellHeight(40.f);

    auto infoText = fmt::format("{} users", this->m_totalUsers);
    if (this->m_infoLabel)
        this->m_infoLabel->setString(infoText.c_str());
    if (this->m_pageLabel)
        this->m_pageLabel->setString(fmt::format("Page {} / {}", this->m_currentPage, this->m_totalPages).c_str());

    if (this->m_users.empty()) {
        auto emptyLabel = CCLabelBMFont::create("No users found", "goldFont.fnt");
        emptyLabel->setScale(0.55f);
        emptyLabel->setPosition({this->m_listNode->getContentSize().width / 2.f,
            this->m_listNode->getContentSize().height / 2.f});
        this->m_listNode->addChild(emptyLabel);
        this->m_listNode->updateLayout();
        return;
    }

    for (auto const& entry : this->m_users) {
        auto row = CCLayer::create();
        row->setContentSize({this->m_listNode->getContentSize().width - 20.f, 40.f});

        auto usernameText = CCLabelBMFont::create(entry.username.c_str(), "goldFont.fnt");
        usernameText->setAnchorPoint({0.f, 1.f});
        usernameText->setScale(0.55f);

        auto usernameLabel = CCMenuItemSpriteExtra::create(
            usernameText, this, menu_selector(ManageUserLayer::onOpenProfile));
        usernameLabel->setTag(entry.accountId);
        usernameLabel->setAnchorPoint({0.f, 1.f});
        usernameLabel->setPosition({12.f, row->getContentSize().height - 5.f});

        if (entry.accountId == -1) {
            usernameText->setColor({0, 255, 0});
            usernameLabel->setEnabled(false);
        }

        auto usernameMenu = CCMenu::create();
        usernameMenu->setPosition({0.f, 0.f});
        usernameMenu->addChild(usernameLabel);
        row->addChild(usernameMenu, 5);

        auto badge = this->spriteForRole(entry.role);
        if (badge) {
            badge->setScale(0.45f);
            badge->setAnchorPoint({0.f, 1.f});
            badge->setPosition({usernameLabel->getPositionX() + usernameLabel->getContentSize().width * usernameLabel->getScale() + 8.f,
                row->getContentSize().height - 5.f});
            row->addChild(badge);
        }

        auto statsLabel = CCLabelBMFont::create(
            fmt::format("ID {} | Active {} | Pending {} | Rejected {} | Uploads {}",
                entry.id,
                entry.activeThumbnails,
                entry.pending,
                entry.rejected,
                entry.totalUploads)
                .c_str(),
            "bigFont.fnt");
        statsLabel->setAnchorPoint({0.f, 0.f});
        statsLabel->setPosition({12.f, 8.f});
        statsLabel->setScale(0.28f);
        row->addChild(statsLabel);

        this->m_listNode->addCell(row);
    }

    this->m_listNode->updateLayout();
    this->m_listNode->scrollToTop();
}

void ManageUserLayer::onOpenProfile(CCObject* sender) {
    auto node = typeinfo_cast<CCNode*>(sender);
    if (!node)
        return;

    auto accountId = node->getTag();
    if (accountId < 0)
        return;

    auto profile = ProfilePage::create(accountId, false);
    if (profile)
        profile->show();
}