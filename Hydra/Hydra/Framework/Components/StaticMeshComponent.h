#pragma once

#include "MeshComponent.h"
#include "Hydra/Framework/StaticMesh.h"

namespace Hydra
{
	class HYDRA_API HStaticMeshComponent : public HMeshComponent
	{
		HCLASS_BODY(HStaticMeshComponent)
	public:
		HStaticMesh* StaticMesh;

	public:
		HStaticMeshComponent();
		~HStaticMeshComponent();
	};
}