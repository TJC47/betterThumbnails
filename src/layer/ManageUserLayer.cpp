#include "ManageUserLayer.hpp"
#include <fmt/format.h>
#include "../popup/BanReasonPopup.hpp"
#include "../popup/FilterUsersPopup.hpp"

#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/UploadActionPopup.hpp>

#include <cue/ListNode.hpp>
#include <cue/LoadingCircle.hpp>

#include "Geode/ui/BasedButtonSprite.hpp"
#include "Geode/ui/General.hpp"
#include <Geode/ui/Button.hpp>
#include <Geode/ui/MDPopup.hpp>
#include "../include/BetterThumbnailConstant.hpp"

using namespace geode::prelude;

namespace {
    constexpr std::array<std::string_view, 6> ROLE_OPTIONS = {
        "Any",
        "user",
        "verified",
        "moderator",
        "admin",
        "owner",
    };
}

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

static int roleRank(const std::string& role) {
    auto it = std::find(ROLE_OPTIONS.begin(), ROLE_OPTIONS.end(), std::string_view(role));
    if (it == ROLE_OPTIONS.end())
        return 0;
    return static_cast<int>(std::distance(ROLE_OPTIONS.begin(), it));
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

    addSideArt(this, SideArt::BottomLeft, false);
    addSideArt(this, SideArt::BottomRight, false);

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

    this->m_loadingCircle = cue::LoadingCircle::create(true);
    this->m_loadingCircle->addToLayer(this->m_listNode, 2);

    this->m_infoLabel = CCLabelBMFont::create("0 users", "goldFont.fnt");
    this->m_infoLabel->setScale(0.48f);
    this->m_infoLabel->setAnchorPoint({1.f, 1.f});
    this->m_infoLabel->setPosition({screenSize.width - 7.f, screenSize.height - 14.f});
    this->addChild(this->m_infoLabel, 2);

    this->m_pageLabel = CCLabelBMFont::create("1 to 0 of 0", "goldFont.fnt");
    this->m_pageLabel->setScale(0.48f);
    this->m_pageLabel->setAnchorPoint({1.f, 1.f});
    this->m_pageLabel->setPosition({screenSize.width - 7.f, screenSize.height - 3.f});
    this->addChild(this->m_pageLabel, 2);

    this->m_navMenu = CCMenu::create();
    this->m_navMenu->setPosition({0.f, 0.f});
    this->addChild(this->m_navMenu, 2);

    this->m_filterMenu = CCMenu::create();
    this->m_filterMenu->setPosition({0.f, 0.f});
    this->addChild(this->m_filterMenu, 2);
    this->m_showBannedOnly = false;

    // @geode-ignore(unknown-resource)
    auto filterBtn = geode::Button::createWithNode(
        EditorButtonSprite::createWithSpriteFrameName(
            // @geode-ignore(unknown-resource)
            "geode.loader/search.png",
            1.f,
            EditorBaseColor::Gray,
            EditorBaseSize::Normal),
        [this](geode::Button* btn) {
            ManageUserLayer::onOpenFilterUsersPopup(btn);
        });
    filterBtn->setPosition({22.f, 60.f});
    this->addChild(filterBtn);

    auto bannedOff = EditorButtonSprite::createWithSprite(
        "BT_banIcon.png"_spr,
        1.f,
        EditorBaseColor::Gray,
        EditorBaseSize::Normal);
    auto bannedOn = EditorButtonSprite::createWithSprite(
        "BT_banIcon.png"_spr,
        1.f,
        EditorBaseColor::Salmon,
        EditorBaseSize::Normal);
    this->m_bannedToggle = CCMenuItemToggler::create(
        bannedOff,
        bannedOn,
        this,
        menu_selector(ManageUserLayer::onToggleBanned));
    this->m_bannedToggle->setPosition({22.f, 22.f});
    this->m_filterMenu->addChild(this->m_bannedToggle);
    this->m_bannedToggle->toggle(this->m_showBannedOnly);

    this->m_roleDropdown = cue::DropdownNode::create({0, 0, 0, 90}, 200.f, 20.f, 120.f);
    this->m_roleDropdown->setPosition({screenSize.width / 2.f, screenSize.height / 2.f + 132.f});
    this->m_roleDropdown->setAnchorPoint({0.5f, 1.f});
    this->m_roleDropdown->setCallback([this](size_t index, CCNode*) {
        if (index >= ROLE_OPTIONS.size())
            return;

        this->m_selectedRole = std::string(ROLE_OPTIONS[index]);
        this->m_currentPage = 1;
        fetchPage(this->m_currentPage);
    });

    auto makeRoleCell = [](std::string_view role) {
        auto cell = CCLayer::create();
        cell->setContentSize({200.f, 20.f});

        auto label = CCLabelBMFont::create(role.data(), "bigFont.fnt");
        auto badge = badgeForRole(std::string(role));

        if (badge) {
            badge->setScale(0.45f);
            label->limitLabelWidth(75.f, 0.5f, 0.35f);

            float badgeWidth = badge->getContentSize().width * badge->getScale();
            float labelWidth = label->getContentSize().width * label->getScale();
            float gap = 4.f;
            float totalWidth = badgeWidth + gap + labelWidth;

            float startX = (cell->getContentSize().width - totalWidth) / 2.f;

            badge->setAnchorPoint({0.f, 0.5f});
            badge->setPosition({
                startX,
                cell->getContentSize().height / 2.f,
            });
            cell->addChild(badge);

            label->setAnchorPoint({0.f, 0.5f});
            label->setPosition({
                startX + badgeWidth + gap,
                cell->getContentSize().height / 2.f,
            });
            cell->addChild(label);
        } else {
            label->limitLabelWidth(100.f, 0.5f, 0.35f);
            label->setAnchorPoint({0.5f, 0.5f});
            label->setPosition({
                cell->getContentSize().width / 2.f,
                cell->getContentSize().height / 2.f,
            });
            cell->addChild(label);
        }

        return cell;
    };

    for (auto role : ROLE_OPTIONS) {
        this->m_roleDropdown->addCell(makeRoleCell(role));
    }
    this->addChild(this->m_roleDropdown, 2);

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
void ManageUserLayer::onOpenFilterUsersPopup(CCObject*) {
    auto popup = FilterUsersPopup::create([this](std::string username, std::string userId, std::string accountId, std::string discordId) {
        this->onApplyUserFilters(std::move(username), std::move(userId), std::move(accountId), std::move(discordId));
    });
    if (popup) {
        popup->show();
    }
}

void ManageUserLayer::onApplyUserFilters(std::string username, std::string userId, std::string accountId, std::string discordId) {
    this->m_usernameFilter = std::move(username);
    this->m_userIdFilter = std::move(userId);
    this->m_accountIdFilter = std::move(accountId);
    this->m_discordIdFilter = std::move(discordId);
    this->m_currentPage = 1;
    fetchPage(this->m_currentPage);
}
void ManageUserLayer::onToggleBanned(CCObject* sender) {
    auto toggle = typeinfo_cast<CCMenuItemToggler*>(sender);
    if (!toggle)
        return;

    this->m_showBannedOnly = !this->m_showBannedOnly;
    fetchPage(this->m_currentPage);
}

void ManageUserLayer::banUser(int id) {
    auto banPopup = BanReasonPopup::create(id);
    if (banPopup) banPopup->show();
}

void ManageUserLayer::unbanUser(int id) {
    createQuickPopup("Unban user?", fmt::format("Are you sure you want to unban <cc>{}</c>?", id), "Cancel", "Unban", [this, id](auto, bool yes) {
        if (!yes) return;
        auto popup = UploadActionPopup::create(nullptr, fmt::format("Unbanning user {}...", id));
        if (popup)
            popup->show();

        auto req = web::WebRequest();
        req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
        auto url = fmt::format("https://levelthumbs.prevter.me/admin/ban/{}", id);
        auto task = req.send("DELETE", url);
        this->m_listener.spawn(std::move(task), [this, popup](web::WebResponse res) {
            if (res.code() == 200) {
                if (popup)
                    popup->showSuccessMessage("User unbanned successfully");
                this->fetchPage(this->m_currentPage);
            } else {
                if (popup)
                    popup->showFailMessage(fmt::format("Unban failed: {}", res.string().unwrapOr("Unknown error")));
            }
        });
    });
}

void ManageUserLayer::fetchPage(int page) {
    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));

    req.param("page", page);
    req.param("per_page", ITEMS_PER_PAGE);
    req.param("sort_by", "id");
    req.param("sort_dir", "asc");
    if (this->m_showBannedOnly)
        req.param("banned", "true");
    if (this->m_selectedRole != "Any")
        req.param("role", this->m_selectedRole);
    if (!this->m_usernameFilter.empty())
        req.param("username", this->m_usernameFilter);
    if (!this->m_userIdFilter.empty())
        req.param("id", this->m_userIdFilter);
    if (!this->m_accountIdFilter.empty())
        req.param("account_id", this->m_accountIdFilter);
    if (!this->m_discordIdFilter.empty())
        req.param("discord_id", this->m_discordIdFilter);

    auto url = betterThumbnail::makeUrl("/admin/users");

    if (this->m_loadingCircle)
        this->m_loadingCircle->fadeIn();

    auto task = req.get(url);
    this->m_listener.spawn(std::move(task), [this, page](web::WebResponse res) {
        if (res.code() < 200 || res.code() > 299) {
            if (this->m_loadingCircle)
                this->m_loadingCircle->fadeOut();
            Notification::create(
                fmt::format("Failed to load users: {}", res.string().unwrapOrDefault()),
                NotificationIcon::Error)
                ->show();
            return;
        }

        auto jsonResult = res.json();
        if (!jsonResult.isOk()) {
            if (this->m_loadingCircle)
                this->m_loadingCircle->fadeOut();
            Notification::create("Failed to parse users response", NotificationIcon::Error)->show();
            return;
        }

        auto json = jsonResult.unwrap();
        if (!json.isObject() || !json["users"].isArray()) {
            if (this->m_loadingCircle)
                this->m_loadingCircle->fadeOut();
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
            entry.banned = item["banned"].asBool().unwrapOrDefault();
            entry.banReason = item["ban_reason"].asString().unwrapOrDefault();
            entry.bannedBy = item["banned_by_username"].asString().unwrapOrDefault();
            this->m_users.push_back(std::move(entry));
        }

        populateList();
        if (this->m_loadingCircle)
            this->m_loadingCircle->fadeOut();
    });
}

void ManageUserLayer::populateList() {
    if (!this->m_listNode)
        return;

    if (this->m_emptyLabel) {
        this->m_emptyLabel->removeFromParent();
        this->m_emptyLabel = nullptr;
    }

    this->m_listNode->clear();
    this->m_listNode->setCellHeight(40.f);

    auto infoText = fmt::format("{} users", this->m_totalUsers);
    if (this->m_infoLabel)
        this->m_infoLabel->setString(infoText.c_str());
    if (this->m_pageLabel)
        this->m_pageLabel->setString(fmt::format("{} to {} of {}",
            (this->m_currentPage - 1) * ITEMS_PER_PAGE + 1,
            std::min(this->m_currentPage * ITEMS_PER_PAGE, this->m_totalUsers),
            this->m_totalUsers)
                .c_str());

    if (this->m_prevBtn)
        this->m_prevBtn->setVisible(this->m_currentPage > 1);
    if (this->m_nextBtn)
        this->m_nextBtn->setVisible(this->m_currentPage < this->m_totalPages);

    if (this->m_users.empty()) {
        this->m_emptyLabel = CCLabelBMFont::create("No users found", "goldFont.fnt");
        this->m_emptyLabel->setScale(0.55f);
        this->m_emptyLabel->setPosition({this->m_listNode->getContentSize().width / 2.f,
            this->m_listNode->getContentSize().height / 2.f});
        this->m_listNode->addChild(this->m_emptyLabel);
        this->m_listNode->updateLayout();
        return;
    }

    auto currentRole = Mod::get()->getSavedValue<std::string>("role");
    auto currentAccountId = Mod::get()->getSavedValue<int>("user_id");
    auto currentRoleRank = roleRank(currentRole);

    for (auto const& entry : this->m_users) {
        auto row = CCLayer::create();
        row->setContentSize({this->m_listNode->getContentSize().width - 20.f, 40.f});

        auto usernameLabel = geode::Button::createWithLabel(entry.username.c_str(), "goldFont.fnt", [entry](geode::Button* btn) {
            if (entry.accountId == -1)
                return;
            ProfilePage::create(entry.accountId, false)->show();
        });
        usernameLabel->setAnchorPoint({0.f, .5f});
        usernameLabel->setScale(0.55f);
        usernameLabel->setPosition({12.f, row->getContentSize().height - 12.f});
        row->addChild(usernameLabel);

        if (entry.accountId == -1) {
            usernameLabel->setColor({0, 255, 0});
            usernameLabel->setEnabled(false);
        }

        auto badge = this->spriteForRole(entry.role);
        float nextX = usernameLabel->getPositionX() + usernameLabel->getContentSize().width * usernameLabel->getScale() + 8.f;
        if (badge) {
            badge->setScale(0.45f);
            badge->setAnchorPoint({0.f, .5f});
            badge->setPosition({nextX, row->getContentSize().height - 12.f});
            row->addChild(badge);
            nextX += badge->getContentSize().width * badge->getScale() + 8.f;
        }

        if (entry.banned) {
            auto infoBtn = geode::Button::createWithNode(CircleButtonSprite::createWithSprite("BT_unbanIcon.png"_spr, .9f, CircleBaseColor::Green, CircleBaseSize::Small), [entry](geode::Button* btn) {
                geode::MDPopup::create(
                    fmt::format("Banned by {}", entry.bannedBy),
                    entry.banReason,
                    "OK")
                    ->show();
            });
            infoBtn->setScale(0.6f);
            infoBtn->setAnchorPoint({0.f, .5f});
            infoBtn->setPosition({nextX, row->getContentSize().height - 12.f});
            row->addChild(infoBtn);

            usernameLabel->setColor({255, 0, 0});
        }

        auto canBan = currentRoleRank > roleRank(entry.role) && entry.accountId != currentAccountId;
        if (canBan) {
            auto banBtn = geode::Button::createWithNode(
                CircleButtonSprite::createWithSprite(
                    entry.banned ? "BT_unbanIcon.png"_spr : "BT_banIcon.png"_spr,
                    .9f,
                    CircleBaseColor::Red,
                    CircleBaseSize::Small),
                [this, entry](geode::Button* btn) {
                    if (entry.banned) {
                        this->unbanUser(entry.id);
                    } else {
                        this->banUser(entry.id);
                    }
                });
            banBtn->setAnchorPoint({.5f, .5f});
            banBtn->setPosition({row->getContentSize().width - 10.f, row->getContentSize().height / 2.f});
            row->addChild(banBtn);
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