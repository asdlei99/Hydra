#include "ShaderImporter.h"

#include <iostream>

#include "Hydra/Core/File.h"

#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

#include "Hydra/Engine.h"

namespace Hydra
{
	HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
	{
		if (!entryPoint || !profile || !blob)
			return E_INVALIDARG;

		*blob = nullptr;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		//const D3D_SHADER_MACRO defines[0];

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile(srcFile, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				std::cout << (char*)errorBlob->GetBufferPointer() << std::endl;
				errorBlob->Release();
			}

			if (shaderBlob)
				shaderBlob->Release();

			return hr;
		}

		*blob = shaderBlob;

		return hr;
	}

	HRESULT CompileShaderFromString(_In_ const String& shaderSource, _In_ const String& name, _In_ ID3DInclude* include, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
	{
		if (!entryPoint || !profile || !blob)
			return E_INVALIDARG;

		*blob = nullptr;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG ) || true
		flags |= D3DCOMPILE_DEBUG;
#endif

		//const D3D_SHADER_MACRO defines[0];

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.length(), name.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				std::cout << (char*)errorBlob->GetBufferPointer() << std::endl;
				errorBlob->Release();
			}

			if (shaderBlob)
				shaderBlob->Release();

			return hr;
		}

		*blob = shaderBlob;

		return hr;
	}

	String GetFeatureLevelForShaderType(const NVRHI::ShaderType::Enum& type)
	{
		switch (type)
		{
		case NVRHI::ShaderType::SHADER_VERTEX:
			return "vs_5_0";
			break;
		case NVRHI::ShaderType::SHADER_HULL:
			return "hs_5_0";
			break;
		case NVRHI::ShaderType::SHADER_DOMAIN:
			return "ds_5_0";
			break;
		case NVRHI::ShaderType::SHADER_GEOMETRY:
			return "gs_5_0";
			break;
		case NVRHI::ShaderType::SHADER_PIXEL:
			return "ps_5_0";
			break;
		case NVRHI::ShaderType::SHADER_COMPUTE:
			return "cs_5_0";
			break;
		default:
			return String();
		}
	}

	/**class ShaderInclude : public ID3DInclude
	{
		HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
		{

		}
		HRESULT __stdcall Close(LPCVOID pData)
		{

		}
	};*/

	ShaderPtr ShaderImporter::Import(const File& file)
	{
		Log("ShaderImporter::Import", file.GetPath(), "Loading...");

		String shaderSource;
		Map<NVRHI::ShaderType::Enum, String> shaderTypes;
		String renderStage;

		List<String> shaderLines = file.ReadLines();
		for (String& line : shaderLines)
		{
			if (StartsWith(line, "#pragma hydra"))
			{
				String pragmaData = line.substr(13);
				if (pragmaData.length() > 0)
				{
					if (pragmaData[0] == ' ')
					{
						pragmaData = pragmaData.substr(1);
					}

					List<String> pragmaParams = SplitString(pragmaData, ' ');

					for (String pragmaParam : pragmaParams)
					{
						if (StringContains(pragmaParam, ':'))
						{
							List<String> paramSplit = SplitString(pragmaParam, ':');
							
							if (paramSplit.size() == 2)
							{
								String paramName = paramSplit[0];
								String paramValue = paramSplit[1];

								if (paramName == "vert")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_VERTEX] = paramValue;
								}

								if (paramName == "hull")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_HULL] = paramValue;
								}

								if (paramName == "dom")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_DOMAIN] = paramValue;
								}

								if (paramName == "geom")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_GEOMETRY] = paramValue;
								}

								if (paramName == "pixel")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_PIXEL] = paramValue;
								}

								if (paramName == "cmp")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_COMPUTE] = paramValue;
								}

								if (paramName == "rs")
								{
									renderStage = paramValue;

									Log("ShaderImporter::Import", file.GetPath(), "Found render stage: " + renderStage);
								}
							}
						}
						else
						{
							// Do other paraments
						}
					}
				}
			}
			else
			{
				shaderSource += line + "\r\n";
			}
		}

		IRendererInterface renderer = Engine::GetRenderInterface();

		if (shaderTypes.size() == 0)
		{
			Log("ShaderImporter::Import", file.GetPath(), "ERROR: Has 0 defined shaders !");

			return nullptr;
		}

		ShaderPtr shader = New(Shader);
		shader->SetSource(file.GetPath());
		shader->SetRenderStage(renderStage);
		
		ITER(shaderTypes, it)
		{
			NVRHI::ShaderType::Enum type = it->first;
			String entryPoint = it->second;

			Log("ShaderImporter::Import", file.GetPath(), "Found entry point: " + entryPoint);

			ID3DBlob* shaderBlob = nullptr;
			HRESULT hr = CompileShaderFromString(shaderSource, file.GetName(), NULL, entryPoint.c_str(), GetFeatureLevelForShaderType(type).c_str(), &shaderBlob);

			if (FAILED(hr))
			{
				printf("Failed compiling vertex shader (%s) %08X\n", file.GetPath().c_str(), hr);
				continue;
			}

			ID3D11ShaderReflection* refl;
			D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&refl);

			D3D11_SHADER_DESC shaderDesc;
			refl->GetDesc(&shaderDesc);

			for (int d = 0; d < shaderDesc.DefCount; d++)
			{
				
			}

			for (UINT i = 0; i < shaderDesc.ConstantBuffers; i++)
			{
				D3D11_SHADER_BUFFER_DESC Description;
				ID3D11ShaderReflectionConstantBuffer* pConstBuffer = refl->GetConstantBufferByIndex(i);
				pConstBuffer->GetDesc(&Description);

				for (UINT j = 0; j < Description.Variables; j++)
				{
					ID3D11ShaderReflectionVariable* pVariable = pConstBuffer->GetVariableByIndex(j);
					D3D11_SHADER_VARIABLE_DESC var_desc;
					pVariable->GetDesc(&var_desc);
					std::cout << " Name: " << var_desc.Name;
					std::cout << " Size: " << var_desc.Size;
					std::cout << " Offset: " << var_desc.StartOffset << "\n";
				}
			}

			Log("ShaderImporter::Import", file.GetPath(), ToString(shaderDesc.ConstantBuffers) + " cbuffers.");

			refl->Release();

			NVRHI::ShaderHandle shaderHandle = renderer->createShader(NVRHI::ShaderDesc(type), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());

			if (shaderHandle != nullptr)
			{
				shader->SetShader(type, shaderHandle, shaderBlob);
			}
		}

		Log("ShaderImporter::Import", file.GetPath(), "Loaded.");

		return shader;
	}
}
