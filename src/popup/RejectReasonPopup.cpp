#include "RejectReasonPopup.hpp"
#include <Geode/ui/Button.hpp>

using namespace geode::prelude;

static const std::vector<std::string> rejectionReasons = {
    "Original was better",
    "Original was made by the creator",
    "Screenshot bug",
    "Dead/About to die",
    "Noclip",
    "Obvious Speedhack",
    "Visible overlays",
    "Low Quality",
    "No gameplay in thumbnail",
    "Too close to start",
    "Texture Pack",
    "High Quality mobile texture bug",
    "Title Card",
};

RejectReasonPopup* RejectReasonPopup::create(int thumbId, geode::Function<void(std::string)> onSend) {
    auto ret = new RejectReasonPopup;
    if (ret && ret->init(thumbId, std::move(onSend))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool RejectReasonPopup::init(int thumbId, geode::Function<void(std::string)> onSend) {
    if (!Popup::init(280.f, 280.f, "GJ_square02.png"))
        return false;

    m_id = thumbId;
    m_callback = std::move(onSend);
    setTitle("Reject Reason");

    m_input = TextInput::create(240.f, "Reason for rejection");
    m_input->setLabel("Rejection Reason");
    m_input->setPosition({m_mainLayer->getContentSize().width / 2.f, m_mainLayer->getContentSize().height - 60.f});
    m_mainLayer->addChild(m_input);

    m_reasonList = cue::ListNode::create({m_mainLayer->getContentSize().width - 20.f, 145.f}, {0, 0, 0, 100}, cue::ListBorderStyle::CommentsBlue);
    m_reasonList->setAnchorPoint({0.5f, 1.0f});
    m_reasonList->setPosition({m_mainLayer->getContentSize().width / 2.f, m_input->getPositionY() - 25.f});
    m_reasonList->getScrollLayer()->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(0.f));
    m_mainLayer->addChild(m_reasonList);

    m_reasonList->clear();
    m_reasonList->setCellHeight(34.f);
    for (auto const& reason : rejectionReasons) {
        auto rowNode = CCLayer::create();
        rowNode->setContentSize({m_reasonList->getContentSize().width, 34.f});

        auto reasonLabel = CCLabelBMFont::create(reason.c_str(), "bigFont.fnt");
        reasonLabel->setAnchorPoint({0.f, 0.5f});
        reasonLabel->setPosition({10.f, 17.f});
        reasonLabel->setScale(0.35f);
        rowNode->addChild(reasonLabel);

        auto useButton = geode::Button::createWithSpriteFrameName(
            "GJ_selectSongBtn_001.png",
            [this, reason](geode::Button*) {
                if (m_input) {
                    m_input->setString(reason);
                }
            });
        useButton->setPosition({rowNode->getContentSize().width - 25.f, 17.f});
        useButton->setScale(0.7f);
        rowNode->addChild(useButton);

        m_reasonList->addCell(rowNode);
    }
    m_reasonList->updateLayout();
    m_reasonList->scrollToTop();

    auto sendSpr = ButtonSprite::create("Reject", "goldFont.fnt", "GJ_button_06.png", 1.f);
    auto sendBtn = CCMenuItemSpriteExtra::create(sendSpr, this, menu_selector(RejectReasonPopup::onSend));
    sendBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 25.f});
    m_buttonMenu->addChild(sendBtn);

    return true;
}

void RejectReasonPopup::onSend(CCObject*) {
    std::string reason = m_input ? m_input->getString() : "";
    if (m_callback)
        m_callback(reason);
    this->onClose(nullptr);
}