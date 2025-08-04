#include "PendingThumbnailLayer.hpp"
#include "BetterThumbnailLayer.hpp"

CCScene *PendingThumbnailLayer::scene()
{
    auto scene = CCScene::create();
    scene->addChild(PendingThumbnailLayer::create());
    return scene;
}

bool PendingThumbnailLayer::init()
{
    if (!CCLayer::init())
        return false;

    auto bg = createLayerBG();
    if (bg != nullptr)
    {
        this->addChild(bg, -1);
    }
    else
    {
        log::error("createLayerBG returned nullptr");
        return false;
    }

    auto screenSize = CCDirector::sharedDirector()->getWinSize();

    auto menu = CCMenu::create();
    this->addChild(menu, 2);
    menu->setPosition(CCPointZero);

    // Fetch pending thumbnails from API
    auto req = web::WebRequest();
    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
    auto task = req.get("https://levelthumbs.prevter.me/pending");
    m_listener.bind([this](web::WebTask::Event *e)
                    {
        if (auto res = e->getValue()) {
            if (res->code() < 200 || res->code() > 299) {
                log::error("Pending API error: {} {}", res->code(), res->string().unwrapOr(""));
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
            }
        } });
    m_listener.setFilter(task);

    // Create CCArray of items for GJListLayer
    auto items = CCArray::create();
    // Example: add a label for now, replace with real items later
    auto testLabel = CCLabelBMFont::create("No pending thumbnails loaded yet", "goldFont.fnt");
    items->addObject(testLabel);

    auto listLayer = GJListLayer::create(
        nullptr,              // BoomListView*
        "Pending Thumbnails", // title
        {0, 0, 0, 100},       // ccColor4B (background color RGBA)
        356.f,                // width
        220.f,                // height
        0                     // list type (default)
    );
    this->addChild(listLayer, 1);
    listLayer->setAnchorPoint({0.5f, 0.5f});
    listLayer->setPosition(CCPoint(screenSize / 2 - listLayer->getScaledContentSize() / 2));

    // Back button at top left
    auto backButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this,
        menu_selector(PendingThumbnailLayer::onBackButton));
    backButton->setPosition(CCPoint(25.f, screenSize.height - 25.f));
    menu->addChild(backButton);

    this->setKeypadEnabled(true);

    return true;
}

PendingThumbnailLayer *PendingThumbnailLayer::create()
{
    auto ret = new PendingThumbnailLayer;
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void PendingThumbnailLayer::keyBackClicked()
{
    onBackButton(nullptr);
}

void PendingThumbnailLayer::onBackButton(CCObject *)
{
    CCDirector::get()->pushScene(CCTransitionFade::create(.5f, BetterThumbnailLayer::scene()));
}
