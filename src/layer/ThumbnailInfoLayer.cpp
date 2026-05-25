#include <Geode/Geode.hpp>

#include <cue/LoadingCircle.hpp>
#include <Geode/ui/Button.hpp>

#include "../include/BetterThumbnailConstant.hpp"
#include "ThumbnailInfoLayer.hpp"
#include "../popup/RejectReasonPopup.hpp"
#include <map>
#include <sstream>

using namespace geode::prelude;

CCScene* ThumbnailInfoLayer::scene(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note) {
    auto scene = CCScene::create();
    scene->addChild(ThumbnailInfoLayer::create(
        id, user_id, username, level_id, accepted, upload_time, replacement, submission_note));
    return scene;
}

ThumbnailInfoLayer* ThumbnailInfoLayer::create(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note) {
    auto ret = new ThumbnailInfoLayer;
    if (ret && ret->init(id, user_id, username, level_id, accepted, upload_time, replacement, submission_note)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ThumbnailInfoLayer::init(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note) {
    if (!CCLayer::init())
        return false;

    addSideArt(this, SideArt::All, false);

    // store fields for later actions
    m_id = id;
    m_userId = user_id;
    m_username = username;
    m_levelId = level_id;
    m_acceptedFlag = accepted;
    m_uploadTime = upload_time;
    m_replacementFlag = replacement;
    m_submissionNote = submission_note;

    auto bg = createLayerBG();
    if (bg != nullptr)
        this->addChild(bg, -1);

    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    addBackButton(this, BackButtonStyle::Green);

    // // Submitter
    // auto submitter = CCLabelBMFont::create(
    //     fmt::format("Submitter: {} ({})", username, user_id).c_str(),
    //     "goldFont.fnt");
    // submitter->setPosition({screenSize.width / 2.f, screenSize.height - 25.f});
    // submitter->setScale(0.8f);
    // submitter->setAlignment(kCCTextAlignmentCenter);
    // this->addChild(submitter, 1);

    // // Timestamp
    // auto timestamp = CCLabelBMFont::create(
    //     fmt::format("Uploaded: {}", upload_time).c_str(), "chatFont.fnt");
    // timestamp->setPosition({screenSize.width / 2.f, screenSize.height - 50.f});
    // timestamp->setScale(0.75f);
    // timestamp->setAlignment(kCCTextAlignmentCenter);
    // this->addChild(timestamp, 1);

    m_levelCell = LevelCell::create(356, 50);
    m_levelCell->setPosition({screenSize.width / 2.f - 170.f, screenSize.height - 55.f});
    m_levelCell->setContentSize({356, 50});
    m_levelCell->m_compactView = true;
    this->fetchLevel();
    this->addChild(m_levelCell);

    // thumb bg
    auto thumbBg = NineSlice::create("GJ_square06.png");
    thumbBg->setPosition(
        {screenSize.width / 2.f - 80.f, screenSize.height / 2.f + 10.f});
    thumbBg->setContentSize({300.f, 170.f});
    this->addChild(thumbBg);

    // Thumbnail (replacement + original)
    auto thumbReplacement = LazySprite::create({300.f, 170.f}, false);
    thumbReplacement->setVisible(true);  // show replacement first by default
    thumbReplacement->setAutoResize(true);
    thumbReplacement->setAnchorPoint({0.5f, 0.5f});
    m_thumbReplacement = thumbReplacement;

    auto thumbOriginal = LazySprite::create({300.f, 170.f}, false);
    thumbOriginal->setVisible(false);  // original starts hidden by default
    thumbOriginal->setAutoResize(true);
    thumbOriginal->setAnchorPoint({0.5f, 0.5f});
    m_thumbOriginal = thumbOriginal;

    auto stencil = NineSlice::create("GJ_square06.png");
    stencil->setContentSize(thumbBg->getContentSize());
    stencil->setPosition({0, 0});

    auto clip = CCClippingNode::create(stencil);
    clip->setAnchorPoint(thumbBg->getAnchorPoint());
    clip->setPosition(thumbBg->getContentSize() / 2);
    thumbBg->addChild(clip);

    // add both sprites to the clipped area
    clip->addChild(thumbReplacement);
    clip->addChild(thumbOriginal);

    auto spinner = cue::LoadingCircle::create(true);
    spinner->setScale(0.45f);
    spinner->addToLayer(clip);
    m_thumbSpinner = spinner;

    // Label above thumbnail indicating which image is shown
    auto thumbLabel = CCLabelBMFont::create(
        m_replacementFlag ? "New Thumbnail" : "Original Thumbnail", "bigFont.fnt");
    thumbLabel->setScale(0.3f);
    thumbLabel->setAnchorPoint({0.5f, 0.5f});
    thumbLabel->setPosition({thumbBg->getPositionX(),
        thumbBg->getPositionY() +
            thumbBg->getContentSize().height / 2.f + 5.f});
    this->addChild(thumbLabel, 2);
    m_thumbLabel = thumbLabel;

    if (!m_replacementFlag)
        thumbLabel->setVisible(false);  // hide label if no replacement

    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));
    auto imageTask = req.get(
        betterThumbnail::makeUrl(fmt::format("/pending/{}/image", id)));
    m_listener.spawn(std::move(imageTask),
        [this, thumbReplacement, thumbOriginal, thumbBg, id, spinner](web::WebResponse res) {
            if (res.code() >= 200 && res.code() <= 299) {
                auto data = res.data();
                if (!data.empty()) {
                    thumbReplacement->loadFromData(data);
                    if (spinner)
                        spinner->fadeOut();
                }
            } else {
                log::error("Image fetch error: {} {}", res.code(), res.string().unwrapOr(""));
                if (spinner)
                    spinner->fadeOut();
            }
        });

    m_bottomMenu = CCMenu::create();
    m_bottomMenu->setPosition({screenSize.width / 2.f - 80.f, 25.f});
    m_bottomMenu->setContentSize({screenSize.width - 300.f, 40.f});
    m_bottomMenu->setLayout(RowLayout::create()->setGap(10.f));
    this->addChild(m_bottomMenu, 2);

    // If this thumbnail is a replacement, add a Show original button under the thumb
    if (m_replacementFlag) {
        auto showSpr =
            ButtonSprite::create("Show original", 140, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 1.f);
        m_showOriginalBtn = CCMenuItemSpriteExtra::create(
            showSpr, this, menu_selector(ThumbnailInfoLayer::onShowOriginal));
        m_bottomMenu->addChild(m_showOriginalBtn);
    }

    std::string infoText = fmt::format(
        "### <cg>Level ID:</c> {}\n"
        "### <cc>Accepted:</c> {}\n"
        "### <cl>Replacement:</c> {}\n"
        "### <co>Thumbnail ID:</c> {}",
        level_id,
        accepted ? "Yes" : "No",
        replacement ? "Yes" : "No",
        id);

    auto infoTextArea = MDTextArea::create(infoText, {180.f, 100.f});
    infoTextArea->setScale(0.8f);
    this->addChildAtPosition(infoTextArea, Anchor::Right, {-115.f, 60.f}, false);

    if (!m_submissionNote.empty()) {
        std::map<std::string, std::string> fields;
        std::stringstream ss(m_submissionNote);
        std::string token;
        while (std::getline(ss, token, ';')) {
            size_t pos = token.find('=');
            if (pos != std::string::npos) {
                fields[token.substr(0, pos)] = token.substr(pos + 1);
            }
        }

        std::string prStr = fields["pr"];
        if (!prStr.empty()) {
            char* end;
            float prVal = std::strtof(prStr.c_str(), &end);
            if (end != prStr.c_str()) {
                prStr = fmt::format("{:.2f}", prVal);
            }
        }
        std::string tmStr = fields["tm"];
        if (!tmStr.empty()) {
            char* end;
            float tmVal = std::strtof(tmStr.c_str(), &end);
            if (end != tmStr.c_str()) {
                tmStr = fmt::format("{:.2f}", tmVal);
            }
        }

        std::string infoText = fmt::format(
            "<cg>Version:</c> {}\n\n"
            "<cg>Submitted By:</c> {}\n\n"
            "<cg>Submitter Account ID:</c> {}\n\n"
            "<cg>Progression:</c> {}%\n\n"
            "<cg>Attempt Time:</c> {}s",
            fields["v"],
            fields["cn"],
            fields["ci"],
            prStr,
            tmStr);
        auto infoTextArea = MDTextArea::create(infoText, {180.f, 120.f});
        infoTextArea->setScale(0.8f);
        this->addChildAtPosition(infoTextArea, Anchor::Right, {-115.f, -30.f}, false);

        std::string noteText = fmt::format(
            "### <cb>Note</c>\n\n{}",
            fields["m"]);
        auto noteTextArea = MDTextArea::create(noteText, {180.f, 80.f});
        noteTextArea->setScale(0.8f);
        this->addChildAtPosition(noteTextArea, Anchor::Right, {-115.f, -120.f}, false);
    }

    // check if user role is a moderator/admin, show the button
    if (betterThumbnail::hasRoleAtLeast(betterThumbnail::RoleNum::Moderator)) {
        auto acceptBtn = geode::Button::createWithNode(
            ButtonSprite::create(
                "Accept", "goldFont.fnt", "GJ_button_01.png"),
            [this](geode::Button* btn) {
                ThumbnailInfoLayer::onAccept(btn);
            });

        auto rejectBtn = geode::Button::createWithNode(
            ButtonSprite::create(
                "Reject", "goldFont.fnt", "GJ_button_06.png"),
            [this](geode::Button* btn) {
                ThumbnailInfoLayer::onReject(btn);
            });

        auto playBtnSprite = ButtonSprite::create(
            "Play Level", "goldFont.fnt", "GJ_button_01.png");
        auto playBtn = geode::Button::createWithNode(
            playBtnSprite, [this](geode::Button* btn) {
                ThumbnailInfoLayer::onPlayLevelButton(btn);
            });
        m_bottomMenu->addChild(playBtn);
        m_bottomMenu->updateLayout();

        auto actionMenu = CCMenu::create();
        actionMenu->setPosition({screenSize.width / 2.f - 80.f, 60.f});
        actionMenu->setContentSize({screenSize.width - 300.f, 40.f});
        actionMenu->setLayout(RowLayout::create()->setGap(5.f));
        actionMenu->addChild(acceptBtn);
        actionMenu->addChild(rejectBtn);
        actionMenu->updateLayout();

        this->addChild(actionMenu);
    }

    this->setKeypadEnabled(true);
    return true;
}

void ThumbnailInfoLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f,
        PopTransition::kPopTransitionFade);
}

void ThumbnailInfoLayer::onAccept(CCObject*) {
    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));
    req.bodyJSON(matjson::makeObject({{"accepted", true}}));
    auto task =
        req.post(betterThumbnail::makeUrl(fmt::format("/pending/{}", m_id)));

    m_listener.spawn(std::move(task), [this](web::WebResponse res) {
        if (res.code() >= 200 && res.code() <= 299) {
            CCDirector::get()->popSceneWithTransition(
                0.5f, PopTransition::kPopTransitionFade);
            Notification::create("Thumbnail accepted.", NotificationIcon::Success)
                ->show();
        } else {
            Notification::create("Failed to accept thumbnail.",
                NotificationIcon::Error)
                ->show();
        }
    });
}

void ThumbnailInfoLayer::onReject(CCObject*) {
    auto popup = RejectReasonPopup::create(m_id, [this](std::string reason) {
        auto req = web::WebRequest();
        req.header("Authorization",
            fmt::format("Bearer {}",
                Mod::get()->getSavedValue<std::string>("token")));
        req.bodyJSON(
            matjson::makeObject({{"accepted", false}, {"reason", reason}}));
        auto task = req.post(
            betterThumbnail::makeUrl(fmt::format("/pending/{}", m_id)));

        m_listener.spawn(std::move(task), [this](web::WebResponse res) {
            if (res.code() >= 200 && res.code() <= 299) {
                CCDirector::get()->popSceneWithTransition(
                    0.5f, PopTransition::kPopTransitionFade);
                Notification::create("Thumbnail rejected.", NotificationIcon::Success)
                    ->show();
            } else {
                Notification::create("Failed to reject thumbnail.",
                    NotificationIcon::Error)
                    ->show();
            }
        });
    });
    if (popup)
        popup->show();
}

void ThumbnailInfoLayer::onPlayLevelButton(CCObject*) {
    if (m_level) {
        auto scene = LevelInfoLayer::scene(m_level, false);
        CCDirector::get()->pushScene(
            CCTransitionFade::create(.5f, scene));
    } else {
        auto search =
            GJSearchObject::create(SearchType::Type19, numToString<int>(m_levelId));
        CCDirector::get()->pushScene(
            CCTransitionFade::create(.5f, LevelBrowserLayer::scene(search)));
    }
}

void ThumbnailInfoLayer::onShowOriginal(CCObject*) {
    // Toggle back to the replacement
    if (m_showingOriginal) {
        if (m_thumbOriginal)
            m_thumbOriginal->setVisible(false);
        if (m_thumbReplacement)
            m_thumbReplacement->setVisible(true);
        if (m_thumbLabel)
            m_thumbLabel->setString("New Thumbnail");
        m_showingOriginal = false;
        // update the button label back to 'Show original'
        if (m_showOriginalBtn) {
            auto newSprite =
                ButtonSprite::create("Show original", "goldFont.fnt", "GJ_button_01.png");
            m_showOriginalBtn->setNormalImage(newSprite);
            m_bottomMenu->updateLayout();
        }
        return;
    }

    // If original already loaded, just show it and hide replacement
    if (m_originalLoaded) {
        if (m_thumbOriginal)
            m_thumbOriginal->setVisible(true);
        if (m_thumbReplacement)
            m_thumbReplacement->setVisible(false);
        if (m_thumbLabel)
            m_thumbLabel->setString("Original Thumbnail");
        m_showingOriginal = true;
        if (m_showOriginalBtn) {
            auto newSprite =
                ButtonSprite::create("Show replacement", "goldFont.fnt", "GJ_button_01.png");
            m_showOriginalBtn->setNormalImage(newSprite);
            m_bottomMenu->updateLayout();
        }
        return;
    }

    // Otherwise, fetch original image
    if (!m_thumbOriginal || !m_thumbSpinner)
        return;
    m_thumbSpinner->setVisible(true);
    auto req = web::WebRequest();
    req.header("Authorization",
        fmt::format("Bearer {}",
            Mod::get()->getSavedValue<std::string>("token")));
    auto task = req.get(
        betterThumbnail::makeUrl(fmt::format("/thumbnail/{}", m_levelId)));

    m_listener.spawn(std::move(task), [this](web::WebResponse res) {
        if (res.code() >= 200 && res.code() <= 299) {
            auto data = res.data();
            if (!data.empty()) {
                if (m_thumbOriginal) {
                    m_thumbOriginal->loadFromData(data);
                    m_thumbOriginal->setVisible(true);
                    if (m_thumbLabel)
                        m_thumbLabel->setString("Original Thumbnail");
                }
                if (m_thumbReplacement) {
                    m_thumbReplacement->setVisible(false);
                }
                m_originalLoaded = true;
                m_showingOriginal = true;
                if (m_thumbSpinner)
                    m_thumbSpinner->setVisible(false);
                if (m_showOriginalBtn) {
                    auto newSprite =
                        ButtonSprite::create("Show replacement", "goldFont.fnt", "GJ_button_01.png");
                    m_showOriginalBtn->setNormalImage(newSprite);
                    m_bottomMenu->updateLayout();
                }
            }
        } else {
            log::error("Original image fetch error: {} {}", res.code(), res.string().unwrapOr(""));
            if (m_thumbSpinner)
                m_thumbSpinner->setVisible(false);
            Notification::create("Error loading original image.",
                NotificationIcon::Error)
                ->show();
        }
    });
}

void ThumbnailInfoLayer::fetchLevel() {
    auto glm = GameLevelManager::sharedState();
    auto searchObj = GJSearchObject::create(SearchType::Search, numToString<int>(m_levelId));
    auto key = searchObj->getKey();
    CCArray* levels = nullptr;

    if (key && key[0]) {
        levels = glm->getStoredOnlineLevels(key);
    }

    if (!levels || levels->count() == 0) {
        if (m_levelFetchRetries > 10) return;

        if (key && key[0]) {
            glm->getOnlineLevels(searchObj);
        }

        m_levelFetchRetries++;
        auto delay = CCDelayTime::create(1.0f);
        auto callback = CCCallFunc::create(this, callfunc_selector(ThumbnailInfoLayer::fetchLevel));
        auto sequence = CCSequence::create(delay, callback, nullptr);
        this->runAction(sequence);
        return;
    }

    GJGameLevel* level = nullptr;
    for (int i = 0; i < levels->count(); ++i) {
        auto l = typeinfo_cast<GJGameLevel*>(levels->objectAtIndex(i));
        if (!l) {
            continue;
        }

        level = l;
        break;
    }

    if (level && m_levelCell) {
        m_level = level;
        m_levelCell->loadFromLevel(level);
        if (m_levelCell->m_mainMenu) {
            m_levelCell->m_mainMenu->setPosition({400, 425});
        }
    }
}
