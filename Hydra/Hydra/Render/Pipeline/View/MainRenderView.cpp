#include "MainRenderView.h"
#include "Hydra/HydraEngine.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Components/MeshComponent.h"

#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Framework/StaticMeshResources.h"

#include "Hydra/Framework/Components/StaticMeshComponent.h"

#include "Hydra/Render/Material.h"
#include "Hydra/Render/Shader.h"

void MainRenderView::OnCreated()
{
	Engine->InitializeAssetManager(Context->GetAssetManager());
	Engine->SceneInit();

	SharedPtr<Technique> tech = MakeShared<Technique>(Context, File("Assets/Shaders/DefaultDeffered.hlsl"), true);

	MaterialInterface* mat = new MaterialInterface("yo", tech);

	mat->GetShader(NVRHI::ShaderType::SHADER_VERTEX)->CreateInputLayout();

	delete mat;
}

void MainRenderView::OnDestroy()
{

}

void MainRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
{
	List<HPrimitiveComponent*>& components = Engine->GetWorld()->GetPrimitiveComponents();

	for (HPrimitiveComponent* cmp : components)
	{
		if (HStaticMeshComponent* staticMeshComponent = cmp->SafeCast<HStaticMeshComponent>())
		{
			HStaticMesh* mesh = staticMeshComponent->StaticMesh;

			if (mesh)
			{

			}
		}
	}
}

void MainRenderView::OnTick(float Delta)
{

}

void MainRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
{
	Context->ScreenSize.x = width;
	Context->ScreenSize.y = height;
}