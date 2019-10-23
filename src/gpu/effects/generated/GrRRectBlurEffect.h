/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrRRectBlurEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrRRectBlurEffect_DEFINED
#define GrRRectBlurEffect_DEFINED
#include "include/core/SkTypes.h"

#include "include/effects/SkBlurMaskFilter.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkBlurPriv.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrStyle.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
class GrRRectBlurEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrTextureProxy> find_or_create_rrect_blur_mask(GrRecordingContext* context,
                                                                const SkRRect& rrectToDraw,
                                                                const SkISize& dimensions,
                                                                float xformedSigma) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 9, "RoundRect Blur Mask");
        builder[0] = SkScalarCeilToInt(xformedSigma - 1 / 6.0f);

        int index = 1;
        for (auto c : {SkRRect::kUpperLeft_Corner, SkRRect::kUpperRight_Corner,
                       SkRRect::kLowerRight_Corner, SkRRect::kLowerLeft_Corner}) {
            SkASSERT(SkScalarIsInt(rrectToDraw.radii(c).fX) &&
                     SkScalarIsInt(rrectToDraw.radii(c).fY));
            builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fX);
            builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fY);
        }
        builder.finish();

        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

        sk_sp<GrTextureProxy> mask(proxyProvider->findOrCreateProxyByUniqueKey(
                key, GrColorType::kAlpha_8, kBottomLeft_GrSurfaceOrigin));
        if (!mask) {
            // TODO: this could be SkBackingFit::kApprox, but:
            //   1) The texture coords would need to be updated.
            //   2) We would have to use GrTextureDomain::kClamp_Mode for the GaussianBlur.
            auto rtc = context->priv().makeDeferredRenderTargetContextWithFallback(
                    SkBackingFit::kExact, dimensions.fWidth, dimensions.fHeight,
                    GrColorType::kAlpha_8, nullptr);
            if (!rtc) {
                return nullptr;
            }

            GrPaint paint;

            rtc->clear(nullptr, SK_PMColor4fTRANSPARENT,
                       GrRenderTargetContext::CanClearFullscreen::kYes);
            rtc->drawRRect(GrNoClip(), std::move(paint), GrAA::kYes, SkMatrix::I(), rrectToDraw,
                           GrStyle::SimpleFill());

            sk_sp<GrTextureProxy> srcProxy(rtc->asTextureProxyRef());
            if (!srcProxy) {
                return nullptr;
            }
            auto rtc2 = SkGpuBlurUtils::GaussianBlur(context,
                                                     std::move(srcProxy),
                                                     rtc->colorInfo().colorType(),
                                                     rtc->colorInfo().alphaType(),
                                                     SkIPoint::Make(0, 0),
                                                     nullptr,
                                                     SkIRect::MakeSize(dimensions),
                                                     SkIRect::EmptyIRect(),
                                                     xformedSigma,
                                                     xformedSigma,
                                                     GrTextureDomain::kIgnore_Mode,
                                                     SkBackingFit::kExact);
            if (!rtc2) {
                return nullptr;
            }

            mask = rtc2->asTextureProxyRef();
            if (!mask) {
                return nullptr;
            }
            SkASSERT(mask->origin() == kBottomLeft_GrSurfaceOrigin);
            proxyProvider->assignUniqueKeyToProxy(key, mask.get());
        }

        return mask;
    }

    static std::unique_ptr<GrFragmentProcessor> Make(GrRecordingContext* context,
                                                     float sigma,
                                                     float xformedSigma,
                                                     const SkRRect& srcRRect,
                                                     const SkRRect& devRRect);
    GrRRectBlurEffect(const GrRRectBlurEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "RRectBlurEffect"; }
    float sigma;
    SkRect rect;
    float cornerRadius;
    TextureSampler ninePatchSampler;

private:
    GrRRectBlurEffect(float sigma, SkRect rect, float cornerRadius,
                      sk_sp<GrTextureProxy> ninePatchSampler)
            : INHERITED(kGrRRectBlurEffect_ClassID,
                        (OptimizationFlags)kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , sigma(sigma)
            , rect(rect)
            , cornerRadius(cornerRadius)
            , ninePatchSampler(std::move(ninePatchSampler)) {
        this->setTextureSamplerCnt(1);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    const TextureSampler& onTextureSampler(int) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    typedef GrFragmentProcessor INHERITED;
};
#endif
