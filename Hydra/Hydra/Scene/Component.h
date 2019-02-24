#pragma once

#include "Hydra/Core/SmartPointer.h"

namespace Hydra
{
	class Spatial;

	class Component
	{
	public:
		Spatial* Parent;
		bool Enabled;

		Component();

		virtual void Start() = 0;
		virtual void Update() = 0;
	};

	typedef SharedPtr<Component> ComponentPtr;
	typedef WeakPtr<Component> ComponentWeakPtr;
}