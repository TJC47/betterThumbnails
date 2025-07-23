#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <gdutilsdevs.gdutils/include/RateEvent.hpp>

using namespace geode::prelude;

class ThumbRequest
{
public:
    EventListener<web::WebTask> m_listener;
    EventListener<web::WebTask> e_listener;

    void setupRequests(CCLabelBMFont *info, LazySprite *banner, CCSize screenSize)
    {
        auto meta_url = std::string("");
        e_listener.bind([info, banner, screenSize](web::WebTask::Event *e)
                        {
            if (web::WebResponse* res = e->getValue()) {
                log::info("{}", res->string().unwrapOr("Uh oh!"));
                info->setString(res->string().unwrapOr("Uh oh!").c_str());
                EventData data = {
                    .type=EventType::Announcement,
                    .title=fmt::format("{}", res->string().unwrapOr("Uh oh!"))
                };
                GDUtils::Events::RateEvent::emit(data);
                banner->loadFromUrl("https://tjcsucht.net/static/banner.png");
            } else if (web::WebProgress* p = e->getProgress()) {
                log::info("progress: {}", p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::info("The request was cancelled... So sad :(");
                info->setString("There was an error");
                FLAlertLayer::create("Request cancelled", "It should not be possible to get this error", "Ok")->show();
                CCDirector::get()->pushScene(CCTransitionFade::create(.5f, CreatorLayer::scene()));
            } });

        m_listener.bind([info, meta_url, this](web::WebTask::Event *e)
                        {
            if (web::WebResponse* res = e->getValue()) {
                auto new_meta_url = fmt::format("{}",res->string().unwrapOr("Uh oh!"));
                log::info("{}", res->string().unwrapOr("Uh oh!"));
                info->setString(fmt::format("Meta server: {}",res->string().unwrapOr("Uh oh!").c_str()).c_str());
                EventData data = {
                    .type=EventType::Announcement,
                    .title=fmt::format("{}{} as meta server\n(Images are supplied by vanilla thumbnails)", "Using ", res->string().unwrapOr("Uh oh!"))
                };
                GDUtils::Events::RateEvent::emit(data);
                auto req = web::WebRequest();
                e_listener.setFilter(req.get(fmt::format("{}/motd", new_meta_url)));
            } else if (web::WebProgress* p = e->getProgress()) {
                log::info("progress: {}", p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::info("The request was cancelled... So sad :(");
                info->setString("There was an error");
                FLAlertLayer::create("Request cancelled", "It should not be possible to get this error", "Ok")->show();
                CCDirector::get()->pushScene(CCTransitionFade::create(.5f, CreatorLayer::scene()));
            } });
    }

    void startMetaRequest()
    {
        auto req = web::WebRequest();
        m_listener.setFilter(req.get("https://tjcsucht.net/api/btserver"));
    }
};
