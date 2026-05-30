#include <Geode/Geode.hpp>

#include <cue/LoadingCircle.hpp>
#include <Geode/ui/Button.hpp>

#include "../include/BetterThumbnailConstant.hpp"
#include "ThumbnailInfoLayer.hpp"
#include "../popup/RejectReasonPopup.hpp"
#include <cmath>
#include <map>
#include <sstream>

using namespace geode::prelude;

CCScene* ThumbnailInfoLayer::scene(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id) {
    auto scene = CCScene::create();
    scene->addChild(ThumbnailInfoLayer::create(
        id, user_id, username, level_id, accepted, upload_time, replacement, submission_note, account_id));
    return scene;
}

ThumbnailInfoLayer* ThumbnailInfoLayer::create(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id) {
    auto ret = new ThumbnailInfoLayer;
    if (ret && ret->init(id, user_id, username, level_id, accepted, upload_time, replacement, submission_note, account_id)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ThumbnailInfoLayer::init(int id, int user_id, const std::string& username, int level_id, bool accepted, const std::string& upload_time, bool replacement, const std::string& submission_note, int account_id) {
    if (!CCLayer::init())
        return false;

    addSideArt(this, SideArt::All, false);

    // store fields for later actions
    m_id = id;
    m_userId = user_id;
    m_accountId = account_id;
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

    // Submitter
    auto submitter = CCLabelBMFont::create(
        fmt::format("Submitted by {} ({})", username, user_id).c_str(),
        "goldFont.fnt");
    submitter->setPosition({screenSize.width / 2.f, screenSize.height - 20.f});
    submitter->setScale(0.8f);
    submitter->setAlignment(kCCTextAlignmentCenter);
    this->addChild(submitter, 1);

    this->fetchLevel();

    // thumb bg
    auto thumbBg = NineSlice::create("GJ_square05.png");
    thumbBg->setPosition(
        {screenSize.width / 2.f, screenSize.height / 2.f});
    thumbBg->setContentSize({300.f, 170.f});
    thumbBg->setScale(1.3f);
    this->addChild(thumbBg);
    m_thumbBg = thumbBg;

    auto thumbBorder = NineSlice::create("GJ_square07.png");
    thumbBorder->setContentSize(thumbBg->getContentSize());
    thumbBorder->setPosition(thumbBg->getContentSize() / 2);
    thumbBg->addChild(thumbBorder, 5);

    auto infoMenu = CCMenu::create();
    infoMenu->setPosition({0, 0});
    infoMenu->setContentSize(thumbBg->getContentSize());
    thumbBg->addChild(infoMenu, 10);

    auto infoOffSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    auto infoOnSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    if (infoOnSpr)
        infoOnSpr->setColor({120, 120, 120});

    m_infoToggle = CCMenuItemToggler::create(
        infoOffSpr,
        infoOnSpr,
        this,
        menu_selector(ThumbnailInfoLayer::onInfoToggle));
    m_infoToggle->setPosition({thumbBg->getContentSize().width,
        thumbBg->getContentSize().height});
    m_infoToggle->toggle(false);
    infoMenu->addChild(m_infoToggle);

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

    auto stencilClip = CCClippingNode::create(stencil);
    stencilClip->setAnchorPoint(thumbBg->getAnchorPoint());
    stencilClip->setPosition(thumbBg->getContentSize() / 2);
    thumbBg->addChild(stencilClip);
    stencilClip->setAlphaThreshold(0.1f);
    stencilClip->addChild(thumbReplacement);
    stencilClip->addChild(thumbOriginal);

    if (m_replacementFlag) {
        m_toggleButton = geode::Button::createWithNode(
            CircleButtonSprite::createWithSpriteFrameName("GJ_sortIcon_001.png", 1.f, CircleBaseColor::Gray, CircleBaseSize::Small),
            [this](geode::Button* btn) {
                this->onShowOriginal(nullptr);
            });
        m_toggleButton->setScale(0.7f);
        m_toggleButton->setPosition({thumbBg->getContentSize().width, 0.f});
        thumbBg->addChild(m_toggleButton, 10);
        updateToggleButtonColor();
    }

    auto spinner = cue::LoadingCircle::create(true);
    spinner->addToLayer(thumbBg, 2);
    m_thumbSpinner = spinner;

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

        std::string level_name = fields["ln"];

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
        // progressions and attempts label
        auto levelLabel = CCLabelBMFont::create(
            fmt::format("Screenshot Details: {}% ({}s)", prStr, tmStr).c_str(), "bigFont.fnt");
        levelLabel->setPosition({screenSize.width / 2.f, screenSize.height - 40.f});
        levelLabel->setScale(0.4f);
        levelLabel->setAlignment(kCCTextAlignmentCenter);
        this->addChild(levelLabel, 1);

        // std::string noteText = fmt::format(
        //     "### <cb>Note</c>\n\n{}",
        //     fields["m"]);
        // auto noteTextArea = MDTextArea::create(noteText, {180.f, 80.f});
        // noteTextArea->setScale(0.8f);
        // this->addChildAtPosition(noteTextArea, Anchor::Right, {-115.f, -120.f}, false);

        std::string infoText = fmt::format(
            "##### <cg>Level ID:</c> {}\n"
            "##### <ca>Level Name:</c> {}\n"
            "##### <cs>Uploaded by:</c> {} ({})\n"
            "##### <cc>Accepted:</c> {}\n"
            "##### <cl>Replacement:</c> {}\n"
            "##### <co>Thumbnail ID:</c> {}\n"
            "##### <cb>Note:</c> {}\n\n",
            level_id,
            level_name,
            username,
            user_id,
            accepted ? "Yes" : "No",
            replacement ? "Yes" : "No",
            id,
            fields["m"]);

        m_infoTextArea = MDTextArea::create(infoText, thumbBg->getContentSize() - CCSize(90.f, 70.f));
        m_infoTextArea->setVisible(false);
        thumbBg->addChildAtPosition(m_infoTextArea, Anchor::BottomLeft, {9.f, 0.f}, {0, 0}, false);
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

        auto playBtn = geode::Button::createWithNode(
            CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png"), [this](geode::Button* btn) {
                ThumbnailInfoLayer::onPlayLevelButton(btn);
            });
        playBtn->setScale(0.3f);
        infoMenu->addChildAtPosition(playBtn, Anchor::TopLeft, {0.f, 0.f}, false);

        auto actionMenu = CCMenu::create();
        actionMenu->setPosition({screenSize.width / 2.f, 25.f});
        actionMenu->setContentSize({screenSize.width - 40.f, 40.f});
        actionMenu->setLayout(RowLayout::create()->setGap(5.f));
        actionMenu->addChild(acceptBtn);
        actionMenu->addChild(rejectBtn);
        actionMenu->updateLayout();

        this->addChild(actionMenu);
    }

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);
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
        return;
    }
}

void ThumbnailInfoLayer::onInfoToggle(CCObject* sender) {
    m_infoVisible = !m_infoVisible;
    if (m_infoTextArea) {
        m_infoTextArea->setVisible(m_infoVisible);
    }
}

float clip(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
}

bool ThumbnailInfoLayer::ccTouchBegan(CCTouch* pTouch, CCEvent* event) {
    LazySprite* thumbnail = m_showingOriginal ? m_thumbOriginal : m_thumbReplacement;
    if (!thumbnail) {
        return false;
    }

    auto touchLocation = pTouch->getLocation();
    if (!thumbnail->boundingBox().containsPoint(touchLocation)) {
        return false;
    }

    if (m_touches.size() == 1) {
        auto firstTouch = *m_touches.begin();
        auto firstLoc = firstTouch->getLocation();
        auto secondLoc = pTouch->getLocation();

        this->m_touchMidPoint = (firstLoc + secondLoc) / 2.f;
        this->m_initialScale = thumbnail->getScale();
        this->m_initialDistance = firstLoc.getDistance(secondLoc);
        auto oldAnchor = thumbnail->getAnchorPoint();
        auto worldPos = thumbnail->convertToWorldSpace({0, 0});
        auto newAnchorX = (m_touchMidPoint.x - worldPos.x) / thumbnail->getScaledContentWidth();
        auto newAnchorY = (m_touchMidPoint.y - worldPos.y) / thumbnail->getScaledContentHeight();
        thumbnail->setAnchorPoint({clip(newAnchorX, 0, 1), clip(newAnchorY, 0, 1)});
        thumbnail->setPosition({thumbnail->getPositionX() + thumbnail->getScaledContentWidth() * -(oldAnchor.x - clip(newAnchorX, 0, 1)),
            thumbnail->getPositionY() + thumbnail->getScaledContentHeight() * -(oldAnchor.y - clip(newAnchorY, 0, 1))});
    }

    m_touches.insert(pTouch);
    return true;
}

void ThumbnailInfoLayer::ccTouchMoved(CCTouch* pTouch, CCEvent* event) {
    LazySprite* thumbnail = m_showingOriginal ? m_thumbOriginal : m_thumbReplacement;
    if (!thumbnail) {
        return;
    }

    if (m_touches.size() == 1) {
        thumbnail->setPosition(thumbnail->getPosition() + pTouch->getDelta());
        return;
    }

    if (m_touches.size() == 2) {
        this->m_wasZooming = true;
        auto it = m_touches.begin();
        auto firstTouch = *it;
        ++it;
        auto secondTouch = *it;

        auto firstLoc = firstTouch->getLocation();
        auto secondLoc = secondTouch->getLocation();
        auto center = (firstLoc + secondLoc) / 2;
        auto distNow = firstLoc.getDistance(secondLoc);

        auto const mult = this->m_initialDistance / distNow;
        auto zoom = clip(this->m_initialScale / mult, 0.2f, 6.5f);
        thumbnail->setScale(zoom);

        auto centerDiff = this->m_touchMidPoint - center;
        thumbnail->setPosition(thumbnail->getPosition() - centerDiff);
        this->m_touchMidPoint = center;
    }
}

void ThumbnailInfoLayer::ccTouchEnded(CCTouch* pTouch, CCEvent* event) {
    m_touches.erase(pTouch);
    LazySprite* thumbnail = m_showingOriginal ? m_thumbOriginal : m_thumbReplacement;
    if (this->m_wasZooming && m_touches.size() == 1 && thumbnail) {
        auto scale = thumbnail->getScale();
        if (scale < 0.25f) {
            thumbnail->runAction(
                CCEaseSineInOut::create(
                    CCScaleTo::create(0.5f, 0.25f)));
        }
        if (scale > 4.0f) {
            thumbnail->runAction(
                CCEaseSineInOut::create(
                    CCScaleTo::create(0.5f, 4.0f)));
        }
        this->m_wasZooming = false;
    }
}

void ThumbnailInfoLayer::scrollWheel(float y, float x) {
    LazySprite* thumbnail = m_showingOriginal ? m_thumbOriginal : m_thumbReplacement;
    if (!thumbnail) {
        return;
    }

    constexpr float zoomSpeed = 0.01f;
    float oldScale = thumbnail->getScale();
    float newScale = oldScale * std::pow(1.f + zoomSpeed, -y);
    newScale = clip(newScale, 0.25f, 6.f);

    if (std::abs(newScale - oldScale) < 0.0001f)
        return;

    CCPoint mouseWorld = getMousePos();
    CCPoint localBefore = thumbnail->convertToNodeSpace(mouseWorld);
    thumbnail->setScale(newScale);

    CCPoint worldAfter = thumbnail->convertToWorldSpace(localBefore);
    CCPoint diff = mouseWorld - worldAfter;
    thumbnail->setPosition(thumbnail->getPosition() + diff);
}

void ThumbnailInfoLayer::fetchLevel() {
    if (m_levelId <= 0) {
        return;
    }

    auto glm = GameLevelManager::sharedState();
    GJGameLevel* level = nullptr;

    if (auto saved = glm->getSavedLevel(m_levelId)) {
        level = saved;
    } else {
        auto searchObj = GJSearchObject::create(SearchType::Search, numToString<int>(m_levelId));
        auto key = searchObj->getKey();
        if (!key || !key[0]) {
            return;
        }

        auto levels = glm->getStoredOnlineLevels(key);
        if (!levels || levels->count() == 0) {
            if (m_levelFetchRetries > 10) {
                return;
            }

            glm->getOnlineLevels(searchObj);
            m_levelFetchRetries++;
            auto delay = CCDelayTime::create(1.0f);
            auto callback = CCCallFunc::create(this, callfunc_selector(ThumbnailInfoLayer::fetchLevel));
            auto sequence = CCSequence::create(delay, callback, nullptr);
            this->runAction(sequence);
            return;
        }

        for (int i = 0; i < levels->count(); ++i) {
            auto l = typeinfo_cast<GJGameLevel*>(levels->objectAtIndex(i));
            if (!l) {
                continue;
            }

            level = l;
            break;
        }
    }

    if (level) {
        m_level = level;
        auto screenSize = CCDirector::sharedDirector()->getWinSize();
        if (m_level->m_accountID == m_accountId) {
            m_creatorNode = CCNode::create();
            m_creatorNode->setScale(0.8f);
            m_creatorNode->setPosition({screenSize.width - 10.f, screenSize.height - 10.f});
            this->addChild(m_creatorNode, 10);

            auto nodeBg = NineSlice::create("square02_001.png");
            nodeBg->setContentSize({133.f, 25.f});
            nodeBg->setOpacity(150);
            nodeBg->setAnchorPoint({1, 1});
            nodeBg->setPosition({5.f, 5.f});
            m_creatorNode->addChild(nodeBg);

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

        if (m_replacementFlag) {
            m_replacementNode = CCNode::create();
            m_replacementNode->setScale(0.8f);
            float replacementY = screenSize.height - 40.f;
            if (!m_creatorNode) {
                replacementY = screenSize.height - 10.f;
            }
            m_replacementNode->setPosition({screenSize.width - 10.f, replacementY});
            this->addChild(m_replacementNode, 10);

            auto replacementBg = NineSlice::create("square02_001.png");
            replacementBg->setContentSize({120.f, 25.f});
            replacementBg->setOpacity(150);
            replacementBg->setAnchorPoint({1, 1});
            replacementBg->setPosition({5.f, 5.f});
            m_replacementNode->addChild(replacementBg);

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
}

void ThumbnailInfoLayer::updateToggleButtonColor() {
    if (!m_toggleButton) {
        return;
    }

    CircleBaseColor color = m_showingOriginal ? CircleBaseColor::Cyan : CircleBaseColor::Gray;
    auto sprite = CircleButtonSprite::createWithSpriteFrameName("GJ_sortIcon_001.png", 1.f, color, CircleBaseSize::Small);
    if (!sprite) {
        return;
    }

    auto oldNode = m_toggleButton->getDisplayNode();
    if (oldNode) {
        oldNode->removeFromParent();
    }

    sprite->setAnchorPoint({0.5f, 0.5f});
    sprite->setPosition({0.f, 0.f});
    m_toggleButton->addChild(sprite);
}

void ThumbnailInfoLayer::onShowOriginal(CCObject*) {
    // Toggle back to the replacement
    if (m_showingOriginal) {
        if (m_thumbOriginal)
            m_thumbOriginal->setVisible(false);
        if (m_thumbReplacement)
            m_thumbReplacement->setVisible(true);
        m_showingOriginal = false;
        if (m_toggleButton) {
            updateToggleButtonColor();
        }
        return;
    }

    // If original already loaded, just show it and hide replacement
    if (m_originalLoaded) {
        if (m_thumbOriginal)
            m_thumbOriginal->setVisible(true);
        if (m_thumbReplacement)
            m_thumbReplacement->setVisible(false);
        m_showingOriginal = true;
        if (m_toggleButton) {
            updateToggleButtonColor();
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
                }
                if (m_thumbReplacement) {
                    m_thumbReplacement->setVisible(false);
                }
                m_originalLoaded = true;
                m_showingOriginal = true;
                if (m_thumbSpinner)
                    m_thumbSpinner->setVisible(false);
                if (m_toggleButton) {
                    updateToggleButtonColor();
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
