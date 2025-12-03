#pragma once
#include <Geode/Geode.hpp>
#include <argon/argon.hpp>
#include "BetterThumbnailLayer.hpp"
#include "NotificationUI.hpp"

using namespace geode::prelude;

class AuthLayer : public CCBlockLayer {
public:
    CCLabelBMFont *loadingLabel = nullptr;
    CCSprite *loadingSpinner = nullptr;
    bool argonResponded = false;
    bool apiResponded = false;
    EventListener<web::WebTask> m_listener;

    static AuthLayer *create();
    virtual bool init();
    void removeLoading();
    void startAuthProcess();
    void onArgonTimeout(float);
    void onApiTimeout(float);
};
