#include "NotificationMenuPopup.hpp"
#include <Geode/Geode.hpp>
#include <cue/ListNode.hpp>

using namespace geode::prelude;

NotificationMenuPopup* NotificationMenuPopup::create() {
    auto ret = new NotificationMenuPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NotificationMenuPopup::init() {
    if (!Popup::init(440.f, 290.f))
        return false;

    setTitle("All Notifications");

    m_listNode = cue::ListNode::create({m_mainLayer->getContentWidth() - 40.f, m_mainLayer->getContentHeight() - 100.f}, {0, 0, 0, 0}, cue::ListBorderStyle::Comments);
    m_listNode->setPosition({m_mainLayer->getContentSize().width / 2.f - 5.f, m_mainLayer->getContentSize().height / 2.f});
    m_listNode->getScrollLayer()->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(0.f));
    m_mainLayer->addChild(m_listNode);

    auto clearBtnSprite = ButtonSprite::create("Clear All", "goldFont.fnt", "GJ_button_06.png");
    auto clearBtn = CCMenuItemSpriteExtra::create(clearBtnSprite, this, menu_selector(NotificationMenuPopup::onClearAll));
    clearBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 25.f});
    m_buttonMenu->addChild(clearBtn);

    return true;
}

void NotificationMenuPopup::setNotifications(const std::vector<NotificationEntry>& notifications, int userId) {
    m_notifications = notifications;
    m_userId = userId;
    populateList();
}

void NotificationMenuPopup::populateList() {
    if (!m_listNode) return;

    m_listNode->clear();

    if (m_notifications.empty()) {
        auto emptyLabel = CCLabelBMFont::create("No notifications", "goldFont.fnt");
        emptyLabel->setPosition({m_listNode->getContentSize().width / 2.f, m_listNode->getContentSize().height / 2.f});
        emptyLabel->setScale(0.6f);
        m_listNode->addChild(emptyLabel);
        return;
    }

    const float rowHeight = 80.f;
    m_listNode->setCellHeight(rowHeight);

    for (auto const& entry : m_notifications) {
        auto rowNode = CCLayer::create();
        rowNode->setContentSize({m_listNode->getContentSize().width, rowHeight});

        auto icon = CCSprite::createWithSpriteFrameName(
            entry.type == "success" ? "GJ_completesIcon_001.png" :
                                    // @geode-ignore(unknown-resource)
                entry.type == "warn" ? "geode.loader/info-warning.png"
            : entry.type == "error"  ? "GJ_deleteIcon_001.png"
                                     :
                                    // @geode-ignore(unknown-resource)
                entry.type == "critical" ? "geode.loader/info-alert.png"
                                         : nullptr);

        float titleOffset = 10.f;
        if (icon) {
            icon->setAnchorPoint({0.f, 1.f});
            icon->setPosition({10.f, rowHeight - 5.f});
            icon->setScale(0.7f);
            rowNode->addChild(icon);
            titleOffset += 26.f;
        }

        auto titleLabel = CCLabelBMFont::create(entry.title.c_str(), "goldFont.fnt");
        titleLabel->setAnchorPoint({0.f, 1.f});
        titleLabel->setPosition({titleOffset, rowHeight - 5.f});
        titleLabel->limitLabelWidth(rowNode->getContentSize().width - titleOffset - 10.f, .8f, 0.5f);
        rowNode->addChild(titleLabel);

        auto contentLabel = SimpleTextArea::create(entry.body.c_str(), "bigFont.fnt", 0.5f, rowNode->getContentSize().width - 20.f);
        contentLabel->setAnchorPoint({0.f, 1.f});
        contentLabel->setScale(0.3f);
        contentLabel->setMaxLines(3);
        contentLabel->setPosition({10.f, rowHeight - 30.f});
        rowNode->addChild(contentLabel);

        auto timestampLabel = CCLabelBMFont::create(entry.timestamp.c_str(), "chatFont.fnt");
        timestampLabel->setAnchorPoint({1.f, .0f});
        timestampLabel->setColor({0, 0, 0});
        timestampLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        timestampLabel->setOpacity(100);
        timestampLabel->setPosition({rowNode->getContentSize().width - 6, 5});
        timestampLabel->setScale(0.6f);
        rowNode->addChild(timestampLabel);

        m_listNode->addCell(rowNode);
    }

    m_listNode->updateLayout();
    m_listNode->scrollToTop();

    auto scrollBar = Scrollbar::create(m_listNode->getScrollLayer());
    scrollBar->setPosition({m_mainLayer->getContentSize().width - 15.f, m_mainLayer->getContentSize().height / 2.f});
    m_mainLayer->addChild(scrollBar);
}

void NotificationMenuPopup::onClearAll(CCObject*) {
    if (m_userId <= 0) {
        Notification::create("Clear notifications failed", NotificationIcon::Error)->show();
        return;
    }

    geode::createQuickPopup(
        "Clear notifications",
        "Are you sure you want to clear all notifications?",
        "No",
        "Clear",
        [this](FLAlertLayer* alert, bool btn2) {
            if (!btn2) {
                return;
            }

            auto upopup = UploadActionPopup::create(nullptr, "Clearing notifications...");
            upopup->show();

            auto req = web::WebRequest();
            req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
            auto url = fmt::format("https://tjcsucht.net/api/bt/clearnotif/{}", m_userId);
            auto task = req.post(url);

            async::spawn(std::move(task), [this, upopup](web::WebResponse res) {
                if (res.code() >= 200 && res.code() <= 299) {
                    m_notifications.clear();
                    populateList();
                    upopup->showSuccessMessage("Notifications cleared");
                    this->onClose(nullptr);
                } else {
                    upopup->showFailMessage(fmt::format("Failed to clear notifications: {}", res.string().unwrapOr("Unknown error")));
                }
            });
        });
}