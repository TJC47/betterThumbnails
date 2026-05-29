#include "ThumbnailNode.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/LazySprite.hpp>
#include <Geode/ui/Button.hpp>

#include "../include/BetterThumbnailConstant.hpp"
#include "../layer/ThumbnailInfoLayer.hpp"
#include "Geode/utils/general.hpp"

using namespace geode::prelude;

ThumbnailNode* ThumbnailNode::create(const CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id) {
    auto ret = new ThumbnailNode();
    if (ret && ret->init(size, id, user_id, username, level_id, accepted, upload_time, replacement, submission_note, account_id)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ThumbnailNode::init(const CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id) {
    if (!CCLayer::init())
        return false;

    const float contentHeight = 100.f;  // node height
    const float rightX = 10.f;
    const float baseY = contentHeight / 2.f;
    const float usernameY = baseY + 35.f;
    const float levelIdY = usernameY - 15.f;
    const float replacementY = levelIdY - 15.f;

    auto bgLayer = NineSlice::create("square02_small.png");
    bgLayer->setContentSize({size.width, contentHeight});
    bgLayer->setOpacity(1);
    bgLayer->setAnchorPoint({0.5f, 0.5f});
    bgLayer->setPosition({size.width / 2.f, contentHeight / 2.f});
    this->addChild(bgLayer);

    this->setContentSize({size.width, contentHeight});
    this->ignoreAnchorPointForPosition(false);
    this->setAnchorPoint({0.5f, 1.0f});

    auto clip = CCClippingNode::create(bgLayer);
    clip->setAlphaThreshold(0.1f);
    this->addChild(clip);

    auto lazySprite = LazySprite::create({360.f, 300.f}, true);
    lazySprite->setAutoResize(true);
    lazySprite->setPosition({size.width / 2.f, baseY});  // offset to the right a bit
    clip->addChild(lazySprite);

    // Store data for info view
    m_thumbId = id;
    m_userId = user_id;
    m_username = username;
    m_levelId = level_id;
    m_accepted = accepted;
    m_uploadTime = upload_time;
    m_replacement = replacement;
    m_submissionNote = submission_note;
    m_accountId = account_id;

    this->fetchLevel();

    auto levelNameText = this->m_level ? std::string(this->m_level->m_levelName.c_str()) : std::string("Unknown Level");
    this->m_levelInfoLabel = CCLabelBMFont::create(fmt::format("{} ({})", levelNameText, this->m_levelId).c_str(), "goldFont.fnt");
    this->m_levelInfoLabel->setAnchorPoint({0.f, 0.5f});
    this->m_levelInfoLabel->setPosition({rightX, usernameY});
    this->m_levelInfoLabel->setScale(0.5f);
    this->addChild(this->m_levelInfoLabel, 2);

    auto submitterText = fmt::format("Submitted by {} ({})", username, user_id);
    auto submitterLabel = CCLabelBMFont::create(submitterText.c_str(), "bigFont.fnt");
    submitterLabel->setAnchorPoint({0.f, 0.5f});
    submitterLabel->setPosition({rightX, levelIdY});
    submitterLabel->setScale(0.3f);
    this->addChild(submitterLabel, 2);

    auto viewBtn = geode::Button::createWithNode(ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png"), [this](geode::Button*) {
        CCDirector::get()->pushScene(CCTransitionFade::create(.5f, ThumbnailInfoLayer::scene(m_thumbId, m_userId, m_username, m_levelId, m_accepted, m_uploadTime, m_replacement, m_submissionNote, m_accountId)));
    });

    auto playBtn = geode::Button::createWithNode(ButtonSprite::create("Play Level", "goldFont.fnt", "GJ_button_01.png"), [this](geode::Button*) {
        if (m_level) {
            auto scene = LevelInfoLayer::scene(m_level, false);
            CCDirector::get()->pushScene(
                CCTransitionFade::create(.5f, scene));
        }
    });

    auto actionMenu = CCMenu::create();
    actionMenu->setPosition({rightX, 15.f});
    actionMenu->setAnchorPoint({0.f, 0.5f});
    actionMenu->setContentSize({160.f, 40.f});
    actionMenu->setLayout(RowLayout::create()->setGap(10.f));
    actionMenu->addChild(viewBtn);
    actionMenu->addChild(playBtn);
    actionMenu->updateLayout();
    this->addChild(actionMenu, 2);

    auto uploadTimeLabel = CCLabelBMFont::create(upload_time.c_str(), "chatFont.fnt");
    uploadTimeLabel->setAnchorPoint({1.f, 0.f});
    uploadTimeLabel->setPosition({size.width - 5.f, 5.f});
    uploadTimeLabel->setScale(0.35f);
    this->addChild(uploadTimeLabel, 2);

    // gradient
    auto gradient = CCLayerGradient::create({0, 0, 0, 255}, {0, 0, 0, 100}, {0.5, 1});
    gradient->setContentSize(this->getContentSize());
    this->addChild(gradient, 1);

    this->updateBadges();

    // Fetch image from API and apply to LazySprite
    auto imageReq = web::WebRequest();
    imageReq.header("Authorization", std::string("Bearer ") + Mod::get()->getSavedValue<std::string>("token"));
    auto imageTask = imageReq.get(
        betterThumbnail::makeUrl(fmt::format("/pending/{}/image", id)));
    m_listener.spawn(std::move(imageTask), [this, lazySprite, size](web::WebResponse res) {
        if (res.code() >= 200 && res.code() <= 299) {
            auto data = res.data();
            if (!data.empty()) {
                lazySprite->loadFromData(data);
            }
        } else {
            log::error("Image fetch error: {} {}", res.code(), res.string().unwrapOr(""));
        }
    });

    return true;
}

void ThumbnailNode::fetchLevel() {
    auto glm = GameLevelManager::sharedState();
    auto searchObj = GJSearchObject::create(SearchType::Search, numToString<int>(m_levelId));
    auto key = searchObj->getKey();
    CCArray* levels = nullptr;

    if (key && key[0]) {
        levels = glm->getStoredOnlineLevels(key);
    }

    if (!levels || levels->count() == 0) {
        if (this->m_levelFetchRetries < 10 && key && key[0]) {
            glm->getOnlineLevels(searchObj);
            this->m_levelFetchRetries++;
            auto delay = CCDelayTime::create(1.0f);
            auto callback = CCCallFunc::create(this, callfunc_selector(ThumbnailNode::fetchLevel));
            auto sequence = CCSequence::create(delay, callback, nullptr);
            this->runAction(sequence);
        }
        return;
    }

    for (int i = 0; i < levels->count(); ++i) {
        auto level = typeinfo_cast<GJGameLevel*>(levels->objectAtIndex(i));
        if (!level) {
            continue;
        }
        m_level = level;
        break;
    }

    if (this->m_level && this->m_levelInfoLabel) {
        this->m_levelInfoLabel->setString(fmt::format("{} ({})", this->m_level->m_levelName.c_str(), this->m_levelId).c_str());
        this->updateBadges();
    }
}

void ThumbnailNode::updateBadges() {
    auto contentSize = this->getContentSize();

    if (this->m_level && this->m_level->m_accountID == this->m_accountId && !this->m_creatorNode) {
        this->m_creatorNode = CCNode::create();
        this->m_creatorNode->setScale(0.8f);
        this->m_creatorNode->setPosition({contentSize.width - 10.f, contentSize.height - 10.f});
        this->addChild(this->m_creatorNode, 3);

        auto badgeBg = NineSlice::create("square02_001.png");
        badgeBg->setContentSize({133.f, 24.f});
        badgeBg->setOpacity(150);
        badgeBg->setAnchorPoint({1, 1});
        badgeBg->setPosition({5.f, 5.f});
        this->m_creatorNode->addChild(badgeBg);

        auto hammerIcon = CCSprite::createWithSpriteFrameName("GJ_hammerIcon_001.png");
        if (hammerIcon) {
            hammerIcon->setScale(0.7f);
            hammerIcon->setAnchorPoint({1.f, 1.f});
            hammerIcon->setPosition({0.f, 0.f});
            m_creatorNode->addChild(hammerIcon);

            auto creatorLabel = CCLabelBMFont::create("Level Creator", "bigFont.fnt");
            creatorLabel->setColor({180, 255, 180});
            creatorLabel->setScale(0.4f);
            creatorLabel->setAnchorPoint({1.f, 1.f});
            creatorLabel->setPosition({-hammerIcon->getScaledContentSize().width - 4.f,
                0.f});
            m_creatorNode->addChild(creatorLabel);
        }
    }

    if (this->m_replacement && !this->m_replacementNode) {
        this->m_replacementNode = CCNode::create();
        this->m_replacementNode->setScale(0.8f);
        float y = contentSize.height - 10.f;
        if (this->m_creatorNode) {
            y -= 28.f;
        }
        this->m_replacementNode->setPosition({contentSize.width - 10.f, y});
        this->addChild(this->m_replacementNode, 3);

        auto badgeBg = NineSlice::create("square02_001.png");
        badgeBg->setContentSize({120.f, 24.f});
        badgeBg->setOpacity(150);
        badgeBg->setAnchorPoint({1, 1});
        badgeBg->setPosition({5.f, 5.f});
        this->m_replacementNode->addChild(badgeBg);

        auto replacementLabel = CCLabelBMFont::create("Replacement", "bigFont.fnt");
        replacementLabel->setScale(0.4f);
        replacementLabel->setColor({255, 180, 100});
        replacementLabel->setAnchorPoint({1.f, 1.f});
        replacementLabel->setPosition({-20.f, -0.5f});
        m_replacementNode->addChild(replacementLabel);

        auto sortIcon = CCSprite::createWithSpriteFrameName("GJ_sortIcon_001.png");
        if (sortIcon) {
            sortIcon->setScale(0.6f);
            sortIcon->setAnchorPoint({1.f, 1.f});
            sortIcon->setPosition({0.f, 0.f});
            m_replacementNode->addChild(sortIcon);
        }
    }
}
