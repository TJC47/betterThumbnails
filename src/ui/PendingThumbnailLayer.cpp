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

    auto items = CCArray::create();
    auto testLabel = CCLabelBMFont::create("No pending thumbnails loaded yet", "goldFont.fnt");
    items->addObject(testLabel);

    auto listLayer = GJListLayer::create(
        nullptr,              // BoomListView*
        "Pending Thumbnails", // title
        {0, 0, 0, 100},       
        356.f,                // width
        220.f,                // height
        0                     // list type (default)
    );
    this->addChild(listLayer, 1);
    listLayer->setAnchorPoint(CCPoint(0.5f, 0.5f));
    listLayer->setPosition(CCPoint(screenSize / 2 - listLayer->getScaledContentSize() / 2)); // dont change this, its a werid way to center it

    auto scrollLayer = ScrollLayer::create(listLayer->getContentSize(), true, true);
    scrollLayer->setContentSize(listLayer->getContentSize());
    scrollLayer->setAnchorPoint({0.0f, 0.0f});
    listLayer->addChild(scrollLayer);
    scrollLayer->setContentSize(listLayer->getContentSize());

    auto spinner = LoadingSpinner::create(100.f);
    spinner->setPosition(listLayer->getContentSize().width / 2.f, listLayer->getContentSize().height / 2.f);
    spinner->setTag(9999);
    listLayer->addChild(spinner, 10);

    auto columnLayout = ColumnLayout::create();
    columnLayout->setAxisReverse(true);
    columnLayout->setAxisAlignment(AxisAlignment::Even);
    columnLayout->setCrossAxisAlignment(AxisAlignment::Center);
    columnLayout->setGap(120.f);
    columnLayout->setAutoGrowAxis(true);

    auto contentLayer = scrollLayer->m_contentLayer;
    contentLayer->setContentSize(scrollLayer->getContentSize());
    contentLayer->setLayout(columnLayout);

    // Fetch pending thumbnails from APIF
    auto req = web::WebRequest();
    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token")));
    auto task = req.get("https://levelthumbs.prevter.me/pending");
    m_listener.bind([this, scrollLayer, contentLayer, listLayer](web::WebTask::Event *e) {
        // Remove spinner when fetch completes
        auto spinner = listLayer->getChildByTag(9999);
        if (spinner) spinner->setVisible(false);

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

            int pendingCount = 0;
            // Add nodes in order, first at top
            for (auto& item : json) {
                auto id = item["id"].asInt().unwrapOrDefault();
                auto user_id = item["user_id"].asInt().unwrapOrDefault();
                auto username = item["username"].asString().unwrapOr("");
                auto level_id = item["level_id"].asInt().unwrapOrDefault();
                auto accepted = item["accepted"].asBool().unwrapOr(false);
                auto upload_time = item["upload_time"].asString().unwrapOr("");
                auto replacement = item["replacement"].asBool().unwrapOr(false);
                auto thumbNode = ThumbnailNode::create(scrollLayer->getContentSize(), id, user_id, username, level_id, accepted, upload_time, replacement);
                if (thumbNode) {
                    // Anchor to top-center
                    thumbNode->setAnchorPoint({0.5f, 1.0f});
                    contentLayer->addChild(thumbNode);
                    pendingCount++;
                }
            }

            contentLayer->updateLayout();

            // Show pending count
            auto countLabel = CCLabelBMFont::create(fmt::format("Pending thumbnails: {}", pendingCount).c_str(), "bigFont.fnt");                                                                           
            countLabel->setPosition(listLayer->getContentSize().width / 2.f, -5.f);
            countLabel->setScale(0.5f);
            listLayer->addChild(countLabel, 11);
        }
    });
    m_listener.setFilter(task);

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
