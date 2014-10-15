/*
 *  glows.h
 *  StatusSpec project
 *  Derived from the Source SDK
 *
 */

#pragma once

#define CLIENT_DLL

#include <map>

#include "ifaces.h"

#include "cbase.h"
#include "mathlib/vector.h"

class C_BaseEntity;
class CViewSetup;
class CMatRenderContextPtr;

class CGlowObjectManager {
public:
	int RegisterGlowObject(C_BaseEntity *pEntity, const Vector &vGlowColor, float flGlowAlpha, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded) {
		int index = m_nNextIndex++;

		m_GlowObjectDefinitions[index].m_hEntity = pEntity;
		m_GlowObjectDefinitions[index].m_vGlowColor = vGlowColor;
		m_GlowObjectDefinitions[index].m_flGlowAlpha = flGlowAlpha;
		m_GlowObjectDefinitions[index].m_bRenderWhenOccluded = bRenderWhenOccluded;
		m_GlowObjectDefinitions[index].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;

		return index;
	}

	void UnregisterGlowObject(int nGlowObjectHandle) {
		m_GlowObjectDefinitions.erase(nGlowObjectHandle);
	}

	void SetEntity(int nGlowObjectHandle, C_BaseEntity *pEntity) {
		if (IsUsed(nGlowObjectHandle)) {
			m_GlowObjectDefinitions[nGlowObjectHandle].m_hEntity = pEntity;
		}
	}

	void SetColor(int nGlowObjectHandle, const Vector &vGlowColor) {
		if (IsUsed(nGlowObjectHandle)) {
			m_GlowObjectDefinitions[nGlowObjectHandle].m_vGlowColor = vGlowColor;
		}
	}

	void SetAlpha(int nGlowObjectHandle, float flAlpha) {
		if (IsUsed(nGlowObjectHandle)) {
			m_GlowObjectDefinitions[nGlowObjectHandle].m_flGlowAlpha = flAlpha;
		}
	}

	void SetRenderFlags(int nGlowObjectHandle, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded) {
		if (IsUsed(nGlowObjectHandle)) {
			m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded = bRenderWhenOccluded;
			m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
		}
	}

	bool IsRenderingWhenOccluded(int nGlowObjectHandle) {
		if (IsUsed(nGlowObjectHandle)) {
			return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded;
		}
		else {
			return false;
		}
	}

	bool IsRenderingWhenUnoccluded(int nGlowObjectHandle) {
		if (IsUsed(nGlowObjectHandle)) {
			return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded;
		}
		else {
			return false;
		}
	}

	bool HasGlowEffect(C_BaseEntity *pEntity) {
		for (auto iterator = m_GlowObjectDefinitions.begin(); iterator != m_GlowObjectDefinitions.end(); ++iterator) {
			if (iterator->second.m_hEntity.Get() == pEntity) {
				return true;
			}
		}

		return false;
	}

	bool IsUsed(int nGlowObjectHandle) {
		return (m_GlowObjectDefinitions.find(nGlowObjectHandle) != m_GlowObjectDefinitions.end());
	}

	void RenderGlowEffects(const CViewSetup *pSetup);

private:
	void RenderGlowModels(const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext);
	void ApplyEntityGlowEffects(const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext, float flBloomScale, int x, int y, int w, int h);

	struct GlowObjectDefinition_t {
		bool ShouldDraw() const {
			return m_hEntity.Get() &&
				(m_bRenderWhenOccluded || m_bRenderWhenUnoccluded) &&
				m_hEntity->ShouldDraw() &&
				!m_hEntity->IsDormant();
		}

		void DrawModel();

		CHandle<C_BaseEntity> m_hEntity;
		Vector m_vGlowColor;
		float m_flGlowAlpha;

		bool m_bRenderWhenOccluded;
		bool m_bRenderWhenUnoccluded;
	};

	int m_nNextIndex;
	std::map<int, GlowObjectDefinition_t> m_GlowObjectDefinitions;
};

extern CGlowObjectManager g_GlowObjectManager;

class CGlowObject {
public:
	CGlowObject(C_BaseEntity *pEntity, const Vector &vGlowColor = Vector(1.0f, 1.0f, 1.0f), float flGlowAlpha = 1.0f, bool bRenderWhenOccluded = false, bool bRenderWhenUnoccluded = false) {
		m_nGlowObjectHandle = g_GlowObjectManager.RegisterGlowObject(pEntity, vGlowColor, flGlowAlpha, bRenderWhenOccluded, bRenderWhenUnoccluded);
	}

	~CGlowObject() {
		g_GlowObjectManager.UnregisterGlowObject(m_nGlowObjectHandle);
	}

	void SetEntity(C_BaseEntity *pEntity) {
		g_GlowObjectManager.SetEntity(m_nGlowObjectHandle, pEntity);
	}

	void SetColor(const Vector &vGlowColor) {
		g_GlowObjectManager.SetColor(m_nGlowObjectHandle, vGlowColor);
	}

	void SetAlpha(float flAlpha) {
		g_GlowObjectManager.SetAlpha(m_nGlowObjectHandle, flAlpha);
	}

	void SetRenderFlags(bool bRenderWhenOccluded, bool bRenderWhenUnoccluded) {
		g_GlowObjectManager.SetRenderFlags(m_nGlowObjectHandle, bRenderWhenOccluded, bRenderWhenUnoccluded);
	}

	bool IsRenderingWhenOccluded() const {
		return g_GlowObjectManager.IsRenderingWhenOccluded(m_nGlowObjectHandle);
	}

	bool IsRenderingWhenUnoccluded() const {
		return g_GlowObjectManager.IsRenderingWhenUnoccluded(m_nGlowObjectHandle);
	}

	bool IsRendering() const {
		return IsRenderingWhenOccluded() || IsRenderingWhenUnoccluded();
	}

private:
	int m_nGlowObjectHandle;

	CGlowObject(const CGlowObject &other);
	CGlowObject& operator=(const CGlowObject &other);
};