/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrRRectBlurEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrRRectBlurEffect.h"

std::unique_ptr<GrFragmentProcessor> GrRRectBlurEffect::Make(GrRecordingContext* context,
                                                             float sigma,
                                                             float xformedSigma,
                                                             const SkRRect& srcRRect,
                                                             const SkRRect& devRRect) {
    SkASSERT(!SkRRectPriv::IsCircle(devRRect) &&
             !devRRect.isRect());  // Should've been caught up-stream

    // TODO: loosen this up
    if (!SkRRectPriv::IsSimpleCircular(devRRect)) {
        return nullptr;
    }

    // Make sure we can successfully ninepatch this rrect -- the blur sigma has to be
    // sufficiently small relative to both the size of the corner radius and the
    // width (and height) of the rrect.
    SkRRect rrectToDraw;
    SkISize dimensions;
    SkScalar ignored[kSkBlurRRectMaxDivisions];
    int ignoredSize;
    uint32_t ignored32;

    bool ninePatchable = SkComputeBlurredRRectParams(
            srcRRect, devRRect, SkRect::MakeEmpty(), sigma, xformedSigma, &rrectToDraw, &dimensions,
            ignored, ignored, ignored, ignored, &ignoredSize, &ignoredSize, &ignored32);
    if (!ninePatchable) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> mask(
            find_or_create_rrect_blur_mask(context, rrectToDraw, dimensions, xformedSigma));
    if (!mask) {
        return nullptr;
    }

    return std::unique_ptr<GrFragmentProcessor>(
            new GrRRectBlurEffect(xformedSigma, devRRect.getBounds(),
                                  SkRRectPriv::GetSimpleRadii(devRRect).fX, std::move(mask)));
}
#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLRRectBlurEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLRRectBlurEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrRRectBlurEffect& _outer = args.fFp.cast<GrRRectBlurEffect>();
        (void)_outer;
        auto sigma = _outer.sigma;
        (void)sigma;
        auto rect = _outer.rect;
        (void)rect;
        auto cornerRadius = _outer.cornerRadius;
        (void)cornerRadius;
        cornerRadiusVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                           "cornerRadius");
        proxyRectVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
                                                        "proxyRect");
        blurRadiusVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                         "blurRadius");
        fragBuilder->codeAppendf(
                "\nhalf2 translatedFragPos = half2(sk_FragCoord.xy - %s.xy);\nhalf threshold = %s "
                "+ 2.0 * %s;\nhalf2 middle = half2((%s.zw - %s.xy) - float(2.0 * threshold));\nif "
                "(translatedFragPos.x >= threshold && translatedFragPos.x < middle.x + threshold) "
                "{\n    translatedFragPos.x = threshold;\n} else if (translatedFragPos.x >= "
                "middle.x + threshold) {\n    translatedFragPos.x -= middle.x - 1.0;\n}\nif "
                "(translatedFragPos.y > threshold && translatedFragPos.y < middle.y + threshold) "
                "{\n    translatedFragPos.y = threshold;",
                args.fUniformHandler->getUniformCStr(proxyRectVar),
                args.fUniformHandler->getUniformCStr(cornerRadiusVar),
                args.fUniformHandler->getUniformCStr(blurRadiusVar),
                args.fUniformHandler->getUniformCStr(proxyRectVar),
                args.fUniformHandler->getUniformCStr(proxyRectVar));
        fragBuilder->codeAppendf(
                "\n} else if (translatedFragPos.y >= middle.y + threshold) {\n    "
                "translatedFragPos.y -= middle.y - 1.0;\n}\nhalf2 proxyDims = half2(2.0 * "
                "threshold + 1.0);\nhalf2 texCoord = translatedFragPos / proxyDims;\n%s = %s * "
                "sample(%s, float2(texCoord)).%s;\n",
                args.fOutputColor, args.fInputColor,
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str());
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrRRectBlurEffect& _outer = _proc.cast<GrRRectBlurEffect>();
        { pdman.set1f(cornerRadiusVar, (_outer.cornerRadius)); }
        auto sigma = _outer.sigma;
        (void)sigma;
        auto rect = _outer.rect;
        (void)rect;
        UniformHandle& cornerRadius = cornerRadiusVar;
        (void)cornerRadius;
        GrSurfaceProxy& ninePatchSamplerProxy = *_outer.textureSampler(0).proxy();
        GrTexture& ninePatchSampler = *ninePatchSamplerProxy.peekTexture();
        (void)ninePatchSampler;
        UniformHandle& proxyRect = proxyRectVar;
        (void)proxyRect;
        UniformHandle& blurRadius = blurRadiusVar;
        (void)blurRadius;

        float blurRadiusValue = 3.f * SkScalarCeilToScalar(sigma - 1 / 6.0f);
        pdman.set1f(blurRadius, blurRadiusValue);

        SkRect outset = rect;
        outset.outset(blurRadiusValue, blurRadiusValue);
        pdman.set4f(proxyRect, outset.fLeft, outset.fTop, outset.fRight, outset.fBottom);
    }
    UniformHandle proxyRectVar;
    UniformHandle blurRadiusVar;
    UniformHandle cornerRadiusVar;
};
GrGLSLFragmentProcessor* GrRRectBlurEffect::onCreateGLSLInstance() const {
    return new GrGLSLRRectBlurEffect();
}
void GrRRectBlurEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                              GrProcessorKeyBuilder* b) const {}
bool GrRRectBlurEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrRRectBlurEffect& that = other.cast<GrRRectBlurEffect>();
    (void)that;
    if (sigma != that.sigma) return false;
    if (rect != that.rect) return false;
    if (cornerRadius != that.cornerRadius) return false;
    if (ninePatchSampler != that.ninePatchSampler) return false;
    return true;
}
GrRRectBlurEffect::GrRRectBlurEffect(const GrRRectBlurEffect& src)
        : INHERITED(kGrRRectBlurEffect_ClassID, src.optimizationFlags())
        , sigma(src.sigma)
        , rect(src.rect)
        , cornerRadius(src.cornerRadius)
        , ninePatchSampler(src.ninePatchSampler) {
    this->setTextureSamplerCnt(1);
}
std::unique_ptr<GrFragmentProcessor> GrRRectBlurEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrRRectBlurEffect(*this));
}
const GrFragmentProcessor::TextureSampler& GrRRectBlurEffect::onTextureSampler(int index) const {
    return IthTextureSampler(index, ninePatchSampler);
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrRRectBlurEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrRRectBlurEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar w = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar h = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar r = d->fRandom->nextRangeF(1.f, 9.f);
    SkScalar sigma = d->fRandom->nextRangeF(1.f, 10.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    return GrRRectBlurEffect::Make(d->context(), sigma, sigma, rrect, rrect);
}
#endif
