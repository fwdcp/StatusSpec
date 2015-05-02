/*
 *  glows.h
 *  StatusSpec project
 *  Derived from the Source SDK
 *
 */

#pragma once

#include <map>

class CViewSetup;
class Vector;

#include "cbase.h"
#include "c_baseentity.h"
#include "ehandle.h"
#include "materialsystem/imaterialsystem.h"

class GlowManager {
public:
	class GlowObject {
	public:
		GlowObject(GlowManager *manager, C_BaseEntity *pEntity, const Vector &vGlowColor = Vector(1.0f, 1.0f, 1.0f), float flGlowAlpha = 1.0f, bool bRenderWhenOccluded = false, bool bRenderWhenUnoccluded = false) {
			m_hManager = manager;
			m_nGlowObjectHandle = m_hManager->RegisterGlowObject(pEntity, vGlowColor, flGlowAlpha, bRenderWhenOccluded, bRenderWhenUnoccluded);
		}

		~GlowObject() {
			m_hManager->UnregisterGlowObject(m_nGlowObjectHandle);
		}

		void SetEntity(C_BaseEntity *pEntity) {
			m_hManager->SetEntity(m_nGlowObjectHandle, pEntity);
		}

		void SetColor(const Vector &vGlowColor) {
			m_hManager->SetColor(m_nGlowObjectHandle, vGlowColor);
		}

		void SetAlpha(float flAlpha) {
			m_hManager->SetAlpha(m_nGlowObjectHandle, flAlpha);
		}

		void SetRenderFlags(bool bRenderWhenOccluded, bool bRenderWhenUnoccluded) {
			m_hManager->SetRenderFlags(m_nGlowObjectHandle, bRenderWhenOccluded, bRenderWhenUnoccluded);
		}

		bool IsRenderingWhenOccluded() const {
			return m_hManager->IsRenderingWhenOccluded(m_nGlowObjectHandle);
		}

		bool IsRenderingWhenUnoccluded() const {
			return m_hManager->IsRenderingWhenUnoccluded(m_nGlowObjectHandle);
		}

		bool IsRendering() const {
			return IsRenderingWhenOccluded() || IsRenderingWhenUnoccluded();
		}

	private:
		GlowManager *m_hManager;
		int m_nGlowObjectHandle;

		GlowObject(const GlowObject &other);
		GlowObject& operator=(const GlowObject &other);
	};

	static bool CheckDependencies();

	void RenderGlowEffects(const CViewSetup *pSetup);

private:
	int RegisterGlowObject(C_BaseEntity *pEntity, const Vector &vGlowColor, float flGlowAlpha, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded) {
		int index = m_nNextIndex++;

		m_GlowObjectDefinitions[index].m_hManager = this;
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
		for (auto iterator : m_GlowObjectDefinitions) {
			if (iterator.second.m_hEntity.Get() == pEntity) {
				return true;
			}
		}

		return false;
	}

	bool IsUsed(int nGlowObjectHandle) {
		return (m_GlowObjectDefinitions.find(nGlowObjectHandle) != m_GlowObjectDefinitions.end());
	}

	void RenderGlowModels(const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext);
	void ApplyEntityGlowEffects(const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext, float flBloomScale, int x, int y, int w, int h);

	struct GlowObjectDefinition_t {
		GlowManager *m_hManager;

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