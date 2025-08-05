#include "PendingThumbnailLayer.hpp"
#include "BetterThumbnailLayer.hpp"
#include "../node/ThumbnailNode.hpp"
#include <Geode/Geode.hpp>

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

            // Find the ScrollLayer and its content layer
            auto listLayer = this->getChildren()->count() > 0 ? dynamic_cast<GJListLayer*>(this->getChildren()->objectAtIndex(1)) : nullptr;
            auto scrollLayer = listLayer ? dynamic_cast<ScrollLayer*>(listLayer->getChildren()->objectAtIndex(listLayer->getChildrenCount() - 1)) : nullptr;
            CCLayer* contentLayer = nullptr;
            if (scrollLayer && scrollLayer->getChildrenCount() > 0) {
                contentLayer = dynamic_cast<CCLayer*>(scrollLayer->getChildren()->objectAtIndex(0));
            }
            if (!contentLayer) {
                log::error("ScrollLayer's content layer is nullptr");
                return;
            }

            float y = contentLayer->getContentSize().height;
            float nodeHeight = 100.f; // match ThumbnailNode height
            for (auto& item : json) {
                auto id = item["id"].asInt().unwrapOrDefault();
                auto user_id = item["user_id"].asInt().unwrapOrDefault();
                auto username = item["username"].asString().unwrapOr("");
                auto level_id = item["level_id"].asInt().unwrapOrDefault();
                auto accepted = item["accepted"].asBool().unwrapOr(false);
                auto upload_time = item["upload_time"].asString().unwrapOr("");
                auto replacement = item["replacement"].asBool().unwrapOr(false);
                auto thumbNode = ThumbnailNode::create(contentLayer->getContentSize(), id, user_id, username, level_id, accepted, upload_time, replacement);
                if (thumbNode != nullptr) {
                    thumbNode->setAnchorPoint(CCPoint(0, 1));
                    thumbNode->setPosition(CCPoint(0, y));
                    contentLayer->addChild(thumbNode);
                    y -= nodeHeight;
                } else {
                    log::error("ThumbnailNode::create returned nullptr");
                }
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
    listLayer->setAnchorPoint(CCPoint(0.5f, 0.5f));
    listLayer->setPosition(CCPoint(screenSize / 2 - listLayer->getScaledContentSize() / 2)); // dont change this, its a werid way to center it

    // scrollable content using ScrollLayer
    auto scrollLayer = ScrollLayer::create(listLayer->getContentSize(), true, true);
    listLayer->addChild(scrollLayer);

    // Add ThumbnailNode to the scrollLayer
    if (scrollLayer != nullptr)
    {
        int id = 0;
        int user_id = 0;
        std::string username = "";
        int level_id = 0;
        bool accepted = false;
        std::string upload_time = "";
        bool replacement = false;
        auto thumbNode = ThumbnailNode::create(listLayer->getContentSize(), id, user_id, username, level_id, accepted, upload_time, replacement);
        if (thumbNode != nullptr)
        {
            // Position the node at the top left for demonstration, adjust as needed
            thumbNode->setAnchorPoint(CCPoint(0, 1));
            if (scrollLayer->getChildrenCount() > 0)
            {
                auto contentLayer = dynamic_cast<CCLayer *>(scrollLayer->getChildren()->objectAtIndex(0));
                if (contentLayer != nullptr)
                {
                    thumbNode->setPosition(CCPoint(0, contentLayer->getContentSize().height));
                    contentLayer->addChild(thumbNode);
                }
                else
                {
                    log::error("ScrollLayer's content layer is nullptr");
                }
            }
            else
            {
                log::error("ScrollLayer has no children (content layer)");
            }
        }
        else
        {
            log::error("ThumbnailNode::create returned nullptr");
        }
    }
    else
    {
        log::error("ScrollLayer is nullptr");
    }

    // Back button at top left
    auto backButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this,
        menu_selector(PendingThumbnailLayer::onBackButton));
    backButton->setPosition({25.f, screenSize.height - 25.f});
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
