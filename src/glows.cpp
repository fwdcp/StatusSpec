/*
 *  glows.cpp
 *  StatusSpec project
 *  Derived from the Source SDK
 *
 */

#include "glows.h"

#include "model_types.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "view_shared.h"
#include "viewpostprocess.h"

#include "common.h"
#include "ifaces.h"

#define FULL_FRAME_TEXTURE "_rt_FullFrameFB"

struct ShaderStencilState_t {
	bool m_bEnable;
	StencilOperation_t m_FailOp;
	StencilOperation_t m_ZFailOp;
	StencilOperation_t m_PassOp;
	StencilComparisonFunction_t m_CompareFunc;
	int m_nReferenceValue;
	uint32 m_nTestMask;
	uint32 m_nWriteMask;

	ShaderStencilState_t() {
		m_bEnable = false;
		m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
		m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		m_nReferenceValue = 0;
		m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
	}

	void SetStencilState(CMatRenderContextPtr &pRenderContext) {
		pRenderContext->SetStencilEnable(m_bEnable);
		pRenderContext->SetStencilFailOperation(m_FailOp);
		pRenderContext->SetStencilZFailOperation(m_ZFailOp);
		pRenderContext->SetStencilPassOperation(m_PassOp);
		pRenderContext->SetStencilCompareFunction(m_CompareFunc);
		pRenderContext->SetStencilReferenceValue(m_nReferenceValue);
		pRenderContext->SetStencilTestMask(m_nTestMask);
		pRenderContext->SetStencilWriteMask(m_nWriteMask);
	}
};

bool GlowManager::CheckDependencies() {
	bool ready = true;

	if (!g_pMaterialSystemHardwareConfig) {
		PRINT_TAG();
		Warning("Required interface IMaterialSystemHardwareConfig for glow manager not available!\n");

		ready = false;
	}

	if (!g_pMaterialSystem) {
		PRINT_TAG();
		Warning("Required interface IMaterialSystem for glow manager not available!\n");

		ready = false;
	}

	if (!g_pStudioRender) {
		PRINT_TAG();
		Warning("Required interface IStudioRender for glow manager not available!\n");

		ready = false;
	}

	if (!Interfaces::pRenderView) {
		PRINT_TAG();
		Warning("Required interface IVRenderView for glow manager not available!\n");

		ready = false;
	}

	return ready;
}

void GlowManager::RenderGlowEffects(const CViewSetup *pSetup) {
	if (g_pMaterialSystemHardwareConfig->SupportsPixelShaders_2_0()) {
		CMatRenderContextPtr pRenderContext(g_pMaterialSystem);

		int nX, nY, nWidth, nHeight;
		pRenderContext->GetViewport(nX, nY, nWidth, nHeight);

		PIXEvent _pixEvent(pRenderContext, "EntityGlowEffects");
		ApplyEntityGlowEffects(pSetup, pRenderContext, 10.0f, nX, nY, nWidth, nHeight);
	}
}

static void SetRenderTargetAndViewPort(ITexture *rt, int w, int h) {
	CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
	pRenderContext->SetRenderTarget(rt);
	pRenderContext->Viewport(0, 0, w, h);
}

void GlowManager::RenderGlowModels(const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext) {
	pRenderContext->PushRenderTargetAndViewport();

	Vector vOrigColor;
	Interfaces::pRenderView->GetColorModulation(vOrigColor.Base());
	float flOrigBlend = Interfaces::pRenderView->GetBlend();

	ITexture *pRtFullFrame = NULL;
	pRtFullFrame = g_pMaterialSystem->FindTexture(FULL_FRAME_TEXTURE, TEXTURE_GROUP_RENDER_TARGET);

	SetRenderTargetAndViewPort(pRtFullFrame, pSetup->width, pSetup->height);

	pRenderContext->ClearColor3ub(0, 0, 0);
	pRenderContext->ClearBuffers(true, false, false);

	IMaterial *pMatGlowColor = NULL;

	pMatGlowColor = g_pMaterialSystem->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER, true);
	g_pStudioRender->ForcedMaterialOverride(pMatGlowColor);

	ShaderStencilState_t stencilState;
	stencilState.m_bEnable = false;
	stencilState.m_nReferenceValue = 0;
	stencilState.m_nTestMask = 0xFF;
	stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
	stencilState.m_PassOp = STENCILOPERATION_KEEP;
	stencilState.m_FailOp = STENCILOPERATION_KEEP;
	stencilState.m_ZFailOp = STENCILOPERATION_KEEP;

	stencilState.SetStencilState(pRenderContext);

	for (auto iterator : m_GlowObjectDefinitions) {
		if (!iterator.second.ShouldDraw()) {
			continue;
		}

		Interfaces::pRenderView->SetBlend(iterator.second.m_flGlowAlpha);
		Vector vGlowColor = iterator.second.m_vGlowColor * iterator.second.m_flGlowAlpha;
		Interfaces::pRenderView->SetColorModulation(&vGlowColor[0]);

		iterator.second.DrawModel();
	}

	g_pStudioRender->ForcedMaterialOverride(NULL);
	Interfaces::pRenderView->SetColorModulation(vOrigColor.Base());
	Interfaces::pRenderView->SetBlend(flOrigBlend);

	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	stencilStateDisable.SetStencilState(pRenderContext);

	pRenderContext->PopRenderTargetAndViewport();
}

void GlowManager::ApplyEntityGlowEffects(const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext, float flBloomScale, int x, int y, int w, int h)
{
	IMaterial *pMatGlowColor = g_pMaterialSystem->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER, true);
	g_pStudioRender->ForcedMaterialOverride(pMatGlowColor);

	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	float flSavedBlend = Interfaces::pRenderView->GetBlend();

	Interfaces::pRenderView->SetBlend(0.0f);
	pRenderContext->OverrideDepthEnable(true, false);

	int iNumGlowObjects = 0;

	for (auto iterator : m_GlowObjectDefinitions) {
		if (!iterator.second.ShouldDraw()) {
			continue;
		}

		if (iterator.second.m_bRenderWhenOccluded || iterator.second.m_bRenderWhenUnoccluded) {
			if (iterator.second.m_bRenderWhenOccluded && iterator.second.m_bRenderWhenUnoccluded) {
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 1;
				stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
				stencilState.m_PassOp = STENCILOPERATION_REPLACE;
				stencilState.m_FailOp = STENCILOPERATION_KEEP;
				stencilState.m_ZFailOp = STENCILOPERATION_REPLACE;

				stencilState.SetStencilState(pRenderContext);

				iterator.second.DrawModel();
			}
			else if (iterator.second.m_bRenderWhenOccluded) {
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 1;
				stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
				stencilState.m_PassOp = STENCILOPERATION_KEEP;
				stencilState.m_FailOp = STENCILOPERATION_KEEP;
				stencilState.m_ZFailOp = STENCILOPERATION_REPLACE;

				stencilState.SetStencilState(pRenderContext);

				iterator.second.DrawModel();
			}
			else if (iterator.second.m_bRenderWhenUnoccluded) {
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 2;
				stencilState.m_nTestMask = 0x1;
				stencilState.m_nWriteMask = 0x3;
				stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
				stencilState.m_PassOp = STENCILOPERATION_INCRSAT;
				stencilState.m_FailOp = STENCILOPERATION_KEEP;
				stencilState.m_ZFailOp = STENCILOPERATION_REPLACE;

				stencilState.SetStencilState(pRenderContext);

				iterator.second.DrawModel();
			}
		}

		iNumGlowObjects++;
	}

	for (auto iterator : m_GlowObjectDefinitions) {
		if (!iterator.second.ShouldDraw()) {
			continue;
		}

		if (iterator.second.m_bRenderWhenOccluded && !iterator.second.m_bRenderWhenUnoccluded) {
			ShaderStencilState_t stencilState;
			stencilState.m_bEnable = true;
			stencilState.m_nReferenceValue = 2;
			stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
			stencilState.m_PassOp = STENCILOPERATION_REPLACE;
			stencilState.m_FailOp = STENCILOPERATION_KEEP;
			stencilState.m_ZFailOp = STENCILOPERATION_KEEP;
			stencilState.SetStencilState(pRenderContext);

			iterator.second.DrawModel();
		}
	}

	pRenderContext->OverrideDepthEnable(false, false);
	Interfaces::pRenderView->SetBlend(flSavedBlend);
	stencilStateDisable.SetStencilState(pRenderContext);
	g_pStudioRender->ForcedMaterialOverride(NULL);

	if (iNumGlowObjects <= 0) {
		return;
	}

	PIXEvent pixEvent(pRenderContext, "RenderGlowModels");
	RenderGlowModels(pSetup, pRenderContext);

	int nSrcWidth = pSetup->width;
	int nSrcHeight = pSetup->height;
	int nViewportX, nViewportY, nViewportWidth, nViewportHeight;
	pRenderContext->GetViewport(nViewportX, nViewportY, nViewportWidth, nViewportHeight);

	ITexture *pRtQuarterSize1 = g_pMaterialSystem->FindTexture("_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET);

	IMaterial *pMatHaloAddToScreen = g_pMaterialSystem->FindMaterial("dev/halo_add_to_screen", TEXTURE_GROUP_OTHER, true);

	IMaterialVar *pDimVar = pMatHaloAddToScreen->FindVar("$C0_X", NULL);
	pDimVar->SetFloatValue(1.0f);

	ShaderStencilState_t stencilState;
	stencilState.m_bEnable = true;
	stencilState.m_nWriteMask = 0x0;
	stencilState.m_nTestMask = 0xFF;
	stencilState.m_nReferenceValue = 0x0;
	stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
	stencilState.m_PassOp = STENCILOPERATION_KEEP;
	stencilState.m_FailOp = STENCILOPERATION_KEEP;
	stencilState.m_ZFailOp = STENCILOPERATION_KEEP;
	stencilState.SetStencilState(pRenderContext);

	pRenderContext->DrawScreenSpaceRectangle(pMatHaloAddToScreen, 0, 0, nViewportWidth, nViewportHeight,
		0.0f, -0.5f, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
		pRtQuarterSize1->GetActualWidth(),
		pRtQuarterSize1->GetActualHeight());

	stencilStateDisable.SetStencilState(pRenderContext);
}

void GlowManager::GlowObjectDefinition_t::DrawModel() {
	if (m_hEntity.Get()) {
		m_hEntity->DrawModel(STUDIO_RENDER);
		C_BaseEntity *pAttachment = m_hEntity->FirstMoveChild();

		while (pAttachment != NULL) {
			if (!m_hManager->HasGlowEffect(pAttachment) && pAttachment->ShouldDraw()) {
				pAttachment->DrawModel(STUDIO_RENDER);
			}

			pAttachment = pAttachment->NextMovePeer();
		}
	}
}