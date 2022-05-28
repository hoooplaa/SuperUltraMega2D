#pragma once

#include "Debug.h"
#include "Math.h"

namespace Mega
{
	enum class eSystemState
	{
		Created = 0,
		Initialized = 1,
		Destroyed = -1,
	};

	// A system guard used to ensure systems are properly initialized and destroyed. Used
	// in things like the Mega::Scene and Mega::Renderer classes
	class SystemGuard
	{
	public:
		SystemGuard() {}
		~SystemGuard()
		{
			MEGA_ASSERT(IsDestroyed(), "Deleting system that has not been properly destoyed");
		}
		virtual void Initialize() = 0
		{
			SetStateInitialized();
		}
		virtual void Destroy() = 0
		{
			SetStateDestroyed();
		}

		bool IsCreated()     const { return m_state == eSystemState::Created; }
		bool IsInitialized() const { return m_state == eSystemState::Initialized; }
		bool IsDestroyed()   const { return m_state == eSystemState::Destroyed; }

		eSystemState GetState() const { return m_state; }

	private:
		void SetStateInitialized() {
			MEGA_ASSERT(IsCreated(), "Initialiazing from state other than created prob doin something weird");
			m_state = eSystemState::Initialized;
		}
		void SetStateDestroyed() {
			MEGA_ASSERT(IsInitialized(), "Destroying unitialized object");
			m_state = eSystemState::Destroyed;
		}

		eSystemState m_state = eSystemState::Created;
	};
}