#include "PendingThumbnailNode.hpp"

PendingThumbnailNode* PendingThumbnailNode::create() {
    auto ret = new PendingThumbnailNode();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PendingThumbnailNode::init() {
    if (!CCNode::init())
        return false;
    // Optionally set up node visuals here
    fetchPendingThumbnails();
    return true;
}

void PendingThumbnailNode::fetchPendingThumbnails() {
    auto req = web::WebRequest();
    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
    auto task = req.get("https://levelthumbs.prevter.me/pending");
    m_listener.bind([this](web::WebTask::Event *e) {
        if (auto res = e->getValue()) {
            if (res->code() < 200 || res->code() > 299) {
                log::error("Pending API error: {} {}", res->code(), res->string().unwrapOr("") );
                return;
            }
            auto json = res->json().unwrapOrDefault();
            if (!json.isArray()) {
                log::error("Pending API did not return an array");
                return;
            }
            for (auto& item : json) {
                auto id = item["id"].asInt().unwrapOrDefault();
                auto user_id = item["user_id"].asInt().unwrapOrDefault();
                auto username = item["username"].asString().unwrapOr("");
                auto level_id = item["level_id"].asInt().unwrapOrDefault();
                auto accepted = item["accepted"].asBool().unwrapOr(false);
                auto upload_time = item["upload_time"].asString().unwrapOr("");
                auto replacement = item["replacement"].asBool().unwrapOr(false);
                log::debug("id: {}, user_id: {}, username: {}, level_id: {}, accepted: {}, upload_time: {}, replacement: {}", id, user_id, username, level_id, accepted, upload_time, replacement);
                addThumbnail(id, user_id, username, level_id, accepted, upload_time, replacement);
            }
        }
    });
    m_listener.setFilter(task);
}

void PendingThumbnailNode::addThumbnail(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement) {
    // Create and add thumbnail node logic here
    // Add thumbnail container as nodeBg
    auto nodeBg = CCScale9Sprite::create("GJ_square05.png");
    nodeBg->setContentSize({150.f, 100.f});
    nodeBg->setAnchorPoint({0.5f, 0.5f});
    nodeBg->setPosition(CCPoint(listLayer->getContentSize().width / 2.f, listLayer->getContentSize().height / 2.f));
    listLayer->addChild(nodeBg, 2);
    
    // Example: create a label for now
    auto label = CCLabelBMFont::create(fmt::format("{} - {}", username, upload_time).c_str(), "goldFont.fnt");
    nodeBg->addChild(label);
}
