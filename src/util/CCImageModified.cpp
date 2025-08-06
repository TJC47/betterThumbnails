#include <Geode/modify/CCImage.hpp>
#include <Geode/Geode.hpp>
#include "webp/decode.h"

using namespace geode::prelude;
// thanks prevter for the webp code

class $modify(CCImage)
{
    bool initWithImageData(void *data, int dataLen, EImageFormat eFmt, int width, int height, int bitsPercComponent, int whoKnows)
    {
        if (eFmt == kFmtWebp || (eFmt == kFmtUnKnown && dataLen > 12 && memcmp(data, "RIFF", 4) == 0 && memcmp((char *)data + dataLen - 8, "WEBP", 4) == 0))
        {
            return this->initWithWebPData(data, dataLen);
        }
        return CCImage::initWithImageData(data, dataLen, eFmt, width, height, bitsPercComponent, whoKnows);
    }

    bool initWithWebPData(void *data, int dataLen)
    {
        WebPBitstreamFeatures features;
        if (WebPGetFeatures(static_cast<uint8_t *>(data), dataLen, &features) != VP8_STATUS_OK)
        {
            log::error("Failed to get WebP features");
            return false;
        }

        m_nWidth = static_cast<uint16_t>(features.width);
        m_nHeight = static_cast<uint16_t>(features.height);
        m_bHasAlpha = features.has_alpha;
        m_nBitsPerComponent = features.has_alpha ? 32 : 24;
        m_bPreMulti = false;

        if (features.has_alpha)
        {
            m_pData = WebPDecodeRGBA(static_cast<uint8_t *>(data), dataLen, nullptr, nullptr);
        }
        else
        {
            m_pData = WebPDecodeRGB(static_cast<uint8_t *>(data), dataLen, nullptr, nullptr);
        }

        return m_pData != nullptr;
    }
};