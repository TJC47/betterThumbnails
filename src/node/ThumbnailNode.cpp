#include "ThumbnailNode.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/LazySprite.hpp>

#include "../layer/ThumbnailInfoLayer.hpp"

using namespace geode::prelude;

ThumbnailNode* ThumbnailNode::create(const CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
    auto ret = new ThumbnailNode();
    if (ret && ret->init(size, id, user_id, username, level_id, accepted, upload_time, replacement)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ThumbnailNode::init(const CCSize& size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
    if (!CCLayer::init())
        return false;

    const float contentHeight = 100.f;  // node height
    const float rightX = 15.f;
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

    auto lazySprite = LazySprite::create({180.f, 101.25f}, true);
    lazySprite->setAutoResize(true);
    lazySprite->setPosition({size.width / 2.f + 87.f, baseY});  // offset to the right a bit
    clip->addChild(lazySprite);

    auto userInfo = username + std::string(" (") + std::to_string(user_id) + ")";
    auto usernameLabel = CCLabelBMFont::create(userInfo.c_str(), "goldFont.fnt");
    usernameLabel->setAnchorPoint({0.f, 0.5f});
    usernameLabel->setPosition({rightX, usernameY});
    usernameLabel->setScale(0.5f);
    this->addChild(usernameLabel);

    auto levelIdText = std::string("Level ID: ") + std::to_string(level_id);
    auto levelIdLabel = CCLabelBMFont::create(levelIdText.c_str(), "bigFont.fnt");
    levelIdLabel->setAnchorPoint({0.f, 0.5f});
    levelIdLabel->setPosition({rightX, levelIdY});
    levelIdLabel->setScale(0.3f);
    this->addChild(levelIdLabel);

    auto replacementText = std::string("Replacement: ") + (replacement ? "Yes" : "No");
    auto replacementLabel = CCLabelBMFont::create(replacementText.c_str(), "bigFont.fnt");
    replacementLabel->setAnchorPoint({0.f, 0.5f});
    replacementLabel->setPosition({rightX, replacementY});
    replacementLabel->setScale(0.3f);
    this->addChild(replacementLabel);

    // Store data for info view
    m_thumbId = id;
    m_userId = user_id;
    m_username = username;
    m_levelId = level_id;
    m_accepted = accepted;
    m_uploadTime = upload_time;
    m_replacement = replacement;

    auto idText = std::string("ThumbID: ") + std::to_string(id);
    auto idLabel = CCLabelBMFont::create(idText.c_str(), "bigFont.fnt");
    idLabel->setAnchorPoint({0.f, 0.5f});
    float idLabelY = replacementY - 15.f;
    idLabel->setPosition({rightX, idLabelY});
    idLabel->setScale(0.3f);
    this->addChild(idLabel);

    auto buttonSprite = ButtonSprite::create("View", "bigFont.fnt", "GJ_button_01.png", 0.6f);
    buttonSprite->setScale(0.65f);
    auto viewBtn = CCMenuItemSpriteExtra::create(
        buttonSprite,
        nullptr,
        this,
        menu_selector(ThumbnailNode::onViewButton));
    viewBtn->setPosition({rightX + 20.f, idLabelY - 20.f});

    auto playBtnSprite = ButtonSprite::create("Play Level", "bigFont.fnt", "GJ_button_01.png", 0.6f);
    playBtnSprite->setScale(0.65f);
    auto playBtn = CCMenuItemSpriteExtra::create(playBtnSprite, nullptr, this, menu_selector(ThumbnailNode::onPlayLevelButton));
    playBtn->setPosition({rightX + 90.f, idLabelY - 20.f});

    auto menu = CCMenu::create();
    menu->addChild(viewBtn);
    menu->addChild(playBtn);
    menu->setPosition({0.f, 0.f});
    this->addChild(menu);

    auto uploadTimeLabel = CCLabelBMFont::create(upload_time.c_str(), "chatFont.fnt");
    uploadTimeLabel->setAnchorPoint({1.f, 0.f});
    uploadTimeLabel->setPosition({size.width - 15.f, 10.f});
    uploadTimeLabel->setScale(0.35f);
    this->addChild(uploadTimeLabel);

    // Fetch image from API and apply to LazySprite
    auto imageReq = web::WebRequest();
    imageReq.header("Authorization", std::string("Bearer ") + Mod::get()->getSavedValue<std::string>("token"));
    auto imageTask = imageReq.get(std::string("https://levelthumbs.prevter.me/pending/") + std::to_string(id) + "/image");
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

void ThumbnailNode::onViewButton(CCObject*) {
    CCDirector::get()->pushScene(
        CCTransitionFade::create(
            .5f,
            ThumbnailInfoLayer::scene(m_thumbId, m_userId, m_username, m_levelId, m_accepted, m_uploadTime, m_replacement)));
}

void ThumbnailNode::onPlayLevelButton(CCObject*) {
    auto search = GJSearchObject::create(SearchType::Type19, std::to_string(m_levelId));
    CCDirector::get()->pushScene(CCTransitionFade::create(.5f, LevelBrowserLayer::scene(search)));
}
