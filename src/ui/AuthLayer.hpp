#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <argon/argon.hpp>

using namespace geode::prelude;

class AuthLayer : public CCBlockLayer {
public:
    CCLabelBMFont *loadingLabel = nullptr;
    CCSprite *loadingSpinner = nullptr;
    bool argonResponded = false;
    bool apiResponded = false;
    async::TaskHolder<web::WebResponse> m_listener;

    static AuthLayer *create();
    virtual bool init();
    void removeLoading();
    void startAuthProcess();
    void onArgonTimeout(float);
    void onApiTimeout(float);
};
