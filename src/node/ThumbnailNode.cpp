#include "ThumbnailNode.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/LazySprite.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

ThumbnailNode *ThumbnailNode::create(const CCSize &size, int id, int user_id, const std::string &username, int level_id, bool accepted, const std::string &upload_time, bool replacement)
{
    auto ret = new ThumbnailNode();
    if (ret && ret->init(size, id, user_id, username, level_id, accepted, upload_time, replacement))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ThumbnailNode::init(const CCSize &size, int id, int user_id, const std::string &username, int level_id, bool accepted, const std::string &upload_time, bool replacement)
{
    if (!CCNode::init())
        return false;

    auto thumbnailBg = CCScale9Sprite::create("GJ_square05.png");
    thumbnailBg->setContentSize({size.width, 100.f});
    thumbnailBg->setScale(0.95f);
    thumbnailBg->setAnchorPoint({0.5f, 0.5f});
    thumbnailBg->setPosition({size.width / 2.f, 0.f});
    this->addChild(thumbnailBg);

    auto lazySprite = LazySprite::create({1920.f, 1080.f}, true);
    lazySprite->setPosition({90.f, 0.f});
    lazySprite->setScale(0.325f);
    this->addChild(lazySprite);

    float rightX = size.width - 185.f;
    float baseY = 0.f;
    float usernameY = baseY + 35.f;
    float levelIdY = usernameY - 15.f;
    float replacementY = levelIdY - 15.f;

    auto userInfo = fmt::format("{} ({})", username, user_id);
    auto usernameLabel = CCLabelBMFont::create(userInfo.c_str(), "goldFont.fnt");
    usernameLabel->setAnchorPoint({0.f, 0.5f});
    usernameLabel->setPosition({rightX, usernameY});
    usernameLabel->setScale(0.5f);
    this->addChild(usernameLabel);

    auto levelIdLabel = CCLabelBMFont::create(fmt::format("Level ID: {}", level_id).c_str(), "bigFont.fnt");
    levelIdLabel->setAnchorPoint({0.f, 0.5f});
    levelIdLabel->setPosition({rightX, levelIdY});
    levelIdLabel->setScale(0.3f);
    this->addChild(levelIdLabel);

    auto replacementLabel = CCLabelBMFont::create(fmt::format("Replacement: {}", replacement ? "Yes" : "No").c_str(), "bigFont.fnt");
    replacementLabel->setAnchorPoint({0.f, 0.5f});
    replacementLabel->setPosition({rightX, replacementY});
    replacementLabel->setScale(0.3f);
    this->addChild(replacementLabel);

    // Store thumb id for use in callback
    m_thumbId = id;

    auto idLabel = CCLabelBMFont::create(fmt::format("ThumbID: {}", id).c_str(), "bigFont.fnt");
    idLabel->setAnchorPoint({0.f, 0.5f});
    float idLabelY = replacementY - 15.f;
    idLabel->setPosition({rightX, idLabelY});
    idLabel->setScale(0.3f);
    this->addChild(idLabel);

    auto buttonSprite = ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    buttonSprite->setScale(0.65f);
    auto viewBtn = CCMenuItemSpriteExtra::create(
        buttonSprite,
        nullptr,
        this,
        menu_selector(ThumbnailNode::onViewButton));
    viewBtn->setAnchorPoint({0.f, 0.5f});
    viewBtn->setPosition({rightX, idLabelY - 20.f});

    auto menu = CCMenu::create();
    menu->addChild(viewBtn);
    menu->setPosition({0.f, 0.f});
    this->addChild(menu);

    float bgWidth = thumbnailBg->getContentSize().width;
    auto uploadTimeLabel = CCLabelBMFont::create(fmt::format("{}", upload_time).c_str(), "chatFont.fnt");
    uploadTimeLabel->setAnchorPoint({1.f, 0.f});
    uploadTimeLabel->setPosition({bgWidth - 15.f, -40.f});
    uploadTimeLabel->setScale(0.35f);
    this->addChild(uploadTimeLabel);

    // Fetch image from API and apply to LazySprite
    auto req = web::WebRequest();
    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
    auto imageTask = req.get(fmt::format("https://levelthumbs.prevter.me/pending/{}/image", id));
    m_listener.bind([lazySprite](web::WebTask::Event *e)
                    {
        if (auto res = e->getValue()) {
            if (res->code() >= 200 && res->code() <= 299) {
                auto data = res->data();
                if (!data.empty()) {
                    lazySprite->loadFromData(data);
                }
            } else {
                log::error("Image fetch error: {} {}", res->code(), res->string().unwrapOr(""));
            }
        } });
    m_listener.setFilter(imageTask);

    /*
    auto info = fmt::format(
        "id: {}, user_id: {}, username: {}, level_id: {}\naccepted: {}\nupload_time: {}\nreplacement: {}",
        id, user_id, username, level_id, accepted ? "true" : "false", upload_time, replacement ? "true" : "false"
    );
    auto label = cocos2d::CCLabelBMFont::create(info.c_str(), "goldFont.fnt");
    label->setAnchorPoint({0.5f, 0.5f});
    label->setPosition({size.width / 2.f, 0.f});
    label->setScale(0.35f);
    this->addChild(label);
    */

    return true;
}

void ThumbnailNode::onViewButton(CCObject *)
{
    geode::utils::web::openLinkInBrowser(fmt::format("https://levelthumbs.prevter.me/pending/{}/image", m_thumbId));
}
