#include "ThumbnailNode.hpp"
#include <Geode/Geode.hpp>

ThumbnailNode *ThumbnailNode::create(const cocos2d::CCSize &size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement)
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

bool ThumbnailNode::init(const cocos2d::CCSize &size, int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement)
{
    if (!cocos2d::CCNode::init())
        return false;
    auto thumbnailBg = cocos2d::extension::CCScale9Sprite::create("GJ_square05.png");
    thumbnailBg->setContentSize({size.width, 100.f});
    thumbnailBg->setScale(0.95f);
    thumbnailBg->setAnchorPoint({0.5f, 0.5f});
    thumbnailBg->setPosition({size.width / 2.f, size.height / 2.f});
    this->addChild(thumbnailBg);

    // Add label with all thumbnail info
    auto info = fmt::format(
        "id: {}, user_id: {}, username: {}, level_id: {}\naccepted: {}\nupload_time: {}\nreplacement: {}",
        id, user_id, username, level_id, accepted ? "true" : "false", upload_time, replacement ? "true" : "false"
    );
    auto label = cocos2d::CCLabelBMFont::create(info.c_str(), "goldFont.fnt");
    label->setAnchorPoint({0.5f, 0.5f});
    label->setPosition({size.width / 2.f, size.height / 2.f});
    this->addChild(label);

    return true;
}
