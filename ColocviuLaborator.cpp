#include <Windows.h>
#include <d3dx9.h>
#include <dshow.h>
#include "Camera.h" 
#include <dinput.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

IGraphBuilder* graphBuilder = NULL;
IMediaControl* mediaControl = NULL;

LPDIRECTINPUT8			    g_pDin;
LPDIRECTINPUTDEVICE8		g_pDinKeyboard;
LPDIRECTINPUTDEVICE8		g_pDinmouse;
BYTE				        g_Keystate[256];
DIMOUSESTATE			    g_pMousestate;

LPDIRECT3D9					directD3D = NULL;
LPDIRECT3DDEVICE9			direct3Device9 = NULL;
LPDIRECT3DVERTEXBUFFER9		cubVertexBuffer = NULL;
LPDIRECT3DTEXTURE9			Textures[6];
CXCamera* camera;


LPD3DXMESH              Mesh_Nava = NULL;
D3DMATERIAL9*			MeshMaterials_Nava = NULL;
LPDIRECT3DTEXTURE9*		MeshTextures_Nava = NULL;
DWORD                   NumMaterials_Nava = 0L;

LPD3DXMESH              Mesh_Earth = NULL;
D3DMATERIAL9*			MeshMaterials_Earth = NULL;
LPDIRECT3DTEXTURE9*		MeshTextures_Earth = NULL;
DWORD                   NumMaterials_Earth = 0;

LPD3DXMESH              Mesh_Moon = NULL;
D3DMATERIAL9*			MeshMaterials_Moon = NULL;
LPDIRECT3DTEXTURE9*		MeshTextures_Moon = NULL;
DWORD                   NumMaterials_Moon = 0;

LPD3DXMESH              Mesh_Mars = NULL;
D3DMATERIAL9*			MeshMaterials_Mars = NULL;
LPDIRECT3DTEXTURE9*		MeshTextures_Mars = NULL;
DWORD                   NumMaterials_Mars = 0;

LPD3DXMESH              Mesh_Sun = NULL;
D3DMATERIAL9*			MeshMaterials_Sun = NULL;
LPDIRECT3DTEXTURE9*		MeshTextures_Sun = NULL;
DWORD                   NumMaterials_Sun = 0;

LPD3DXMESH              Mesh_Neptun = NULL;
D3DMATERIAL9*			MeshMaterials_Neptun = NULL;
LPDIRECT3DTEXTURE9*		MeshTextures_Neptun = NULL;
DWORD					NumMaterials_Neptun = 0;

LPD3DXMESH              Mesh_Uranus = NULL;
D3DMATERIAL9*			MeshMaterials_Uranus = NULL;
LPDIRECT3DTEXTURE9*		MeshTextures_Uranus = NULL;
DWORD					NumMaterials_Uranus = 0;
D3DXMATRIXA16			worldMatrix;

float distance;
int direction = 1;
int maximumStepDirection = 10;
int currentStep = 0;

D3DXMATRIX rotate_mesh;
D3DXMATRIX translate_mesh;
D3DXMATRIX scaling_mesh;

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	FLOAT       tu, tv;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

HRESULT InitD3D(HWND hWnd)
{
	if (NULL == (directD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(directD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &direct3Device9)))
	{
		if (FAILED(directD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp, &direct3Device9)))
			return E_FAIL;
	}

	direct3Device9->SetRenderState(D3DRS_ZENABLE, TRUE);
	direct3Device9->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

	direct3Device9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	direct3Device9->SetRenderState(D3DRS_LIGHTING, FALSE);

	return S_OK;
}
HRESULT InitDInput(HINSTANCE hInstance, HWND hWnd)
{
	DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDin, NULL);
	g_pDin->CreateDevice(GUID_SysKeyboard, &g_pDinKeyboard, NULL);
	g_pDin->CreateDevice(GUID_SysMouse, &g_pDinmouse, NULL);

	g_pDinKeyboard->SetDataFormat(&c_dfDIKeyboard);
	g_pDinmouse->SetDataFormat(&c_dfDIMouse);

	g_pDinKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	g_pDinmouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	return S_OK;
}
HRESULT InitDirectShow(HWND hWnd)
{
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&graphBuilder);
	hr = graphBuilder->QueryInterface(IID_IMediaControl, (void**)&mediaControl);

	hr = graphBuilder->RenderFile(L"sound.mp3", NULL);

	mediaControl->Run();

	return S_OK;
}
VOID DetectInput()
{
	g_pDinKeyboard->Acquire();
	g_pDinmouse->Acquire();

	g_pDinKeyboard->GetDeviceState(256, (LPVOID)g_Keystate);
	g_pDinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&g_pMousestate);
}
VOID InitiateCamera()
{
	camera = new CXCamera(direct3Device9);

	D3DXVECTOR3 vEyePt(10.0f, 0.0f, -5.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);

	camera->LookAtPos(&vEyePt, &vLookatPt, &vUpVec);
}

HRESULT createSkyBox() {
	FLOAT size = 10.0f;
	CUSTOMVERTEX cub[] =
	{

		{  D3DXVECTOR3(size, -size, size),0.0f, 1.0f },
		{  D3DXVECTOR3(size, size, size), 0.0f, 0.0f },
		{  D3DXVECTOR3(size, -size, -size), 1.0f, 1.0f },
		{  D3DXVECTOR3(size, size, -size),1.0f, 0.0f },

		{  D3DXVECTOR3(-size, -size, -size), 0.0f, 1.0f },
		{  D3DXVECTOR3(-size, size, -size), 0.0f, 0.0f },
		{  D3DXVECTOR3(-size, -size, size), 1.0f, 1.0f},
		{  D3DXVECTOR3(-size, size, size), 1.0f, 0.0f },

		{  D3DXVECTOR3(-size, size, size), 0.0f, 1.0f },
		{  D3DXVECTOR3(-size, size, -size), 0.0f,0.0f  },
		{  D3DXVECTOR3(size, size, size),1.0f,1.0f},
		{  D3DXVECTOR3(size, size, -size), 1.0f, 0.0f },

		{  D3DXVECTOR3(-size, -size, -size), 0.0f, 1.0f },
		{  D3DXVECTOR3(-size, -size, size), 0.0f, 0.0f  },
		{  D3DXVECTOR3(size, -size, -size), 1.0f,1.0f  },
		{  D3DXVECTOR3(size, -size, size), 1.0f,0.0f   },

		{  D3DXVECTOR3(-size, -size, size), 0.0f, 1.0f },
		{  D3DXVECTOR3(-size, size, size), 0.0f, 0.0f },
		{  D3DXVECTOR3(size, -size, size), 1.0f, 1.0f },
		{  D3DXVECTOR3(size, size, size),1.0f, 0.0f },

		{  D3DXVECTOR3(size, -size, -size), 0.0f, 1.0f },
		{  D3DXVECTOR3(size, size, -size), 0.0f, 0.0f },
		{  D3DXVECTOR3(-size, -size, -size),1.0f, 1.0f },
		{  D3DXVECTOR3(-size, size, -size),1.0f, 0.0f }
	};

	if (FAILED(direct3Device9->CreateVertexBuffer(sizeof(cub),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &cubVertexBuffer, NULL)))
	{
		return E_FAIL;
	}

	CUSTOMVERTEX* pVertices;
	if (FAILED(cubVertexBuffer->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;

	memcpy(pVertices, cub, sizeof(cub));
	cubVertexBuffer->Unlock();
}
HRESULT loadTexture()
{
	if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "posx.png", &Textures[0])))
	{
		if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "..\\posx.png", &Textures[0])))
		{
			MessageBox(NULL, "Could not find posx.png", "Textures.exe", MB_OK);
			return E_FAIL;
		}
	}
	if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "posy.png", &Textures[2])))
	{
		if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "..\\posy.png", &Textures[2])))
		{
			MessageBox(NULL, "Could not find posy.png", "Textures.exe", MB_OK);
			return E_FAIL;
		}
	}
	if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "posz.png", &Textures[4])))
	{
		if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "..\\posz.png", &Textures[4])))
		{
			MessageBox(NULL, "Could not find posz.png", "Textures.exe", MB_OK);
			return E_FAIL;
		}
	}
	if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "negx.png", &Textures[1])))
	{
		if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "..\\negx.png", &Textures[1])))
		{
			MessageBox(NULL, "Could not find negx.png", "Textures.exe", MB_OK);
			return E_FAIL;
		}
	}
	if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "negy.png", &Textures[3])))
	{
		if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "..\\negy.png", &Textures[3])))
		{
			MessageBox(NULL, "Could not find negy.png", "Textures.exe", MB_OK);
			return E_FAIL;
		}
	}
	if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "negz.png", &Textures[5])))
	{
		if (FAILED(D3DXCreateTextureFromFile(direct3Device9, "..\\negz.png", &Textures[5])))
		{
			MessageBox(NULL, "Could not find negz.png", "Textures.exe", MB_OK);
			return E_FAIL;
		}
	}
}
HRESULT loadMesh()
{
	LPD3DXBUFFER pD3DXMtrlBuffer_Nava;
	LPD3DXBUFFER pD3DXMtrlBuffer_Earth;
	LPD3DXBUFFER pD3DXMtrlBuffer_Sun;
	LPD3DXBUFFER pD3DXMtrlBuffer_Mars;
	LPD3DXBUFFER pD3DXMtrlBuffer_Moon;
	LPD3DXBUFFER pD3DXMtrlBuffer_Neptun;
	LPD3DXBUFFER pD3DXMtrlBuffer_Uranus;

	if (FAILED(D3DXLoadMeshFromX("SciFi_Fighter.x", D3DXMESH_SYSTEMMEM, direct3Device9, NULL, &pD3DXMtrlBuffer_Nava, NULL,
		&NumMaterials_Nava,
		&Mesh_Nava)))
	{
		MessageBox(NULL, "Could not find SciFi_Fighter.x", "Meshes.exe", MB_OK);
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials_Nava = (D3DXMATERIAL*)pD3DXMtrlBuffer_Nava->GetBufferPointer();
	MeshMaterials_Nava = new D3DMATERIAL9[NumMaterials_Nava];
	MeshTextures_Nava = new LPDIRECT3DTEXTURE9[NumMaterials_Nava];

	for (DWORD i = 0; i < NumMaterials_Nava; i++)
	{
		MeshMaterials_Nava[i] = d3dxMaterials_Nava[i].MatD3D;
		MeshTextures_Nava[i] = NULL;

		if (d3dxMaterials_Nava[i].pTextureFilename != NULL && lstrlen(d3dxMaterials_Nava[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(direct3Device9,
				d3dxMaterials_Nava[i].pTextureFilename,
				&MeshTextures_Nava[i])))
			{
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
			}
		}
	}

	if (FAILED(D3DXLoadMeshFromX("Earth.x", D3DXMESH_SYSTEMMEM,
		direct3Device9, NULL,
		&pD3DXMtrlBuffer_Earth, NULL, &NumMaterials_Earth,
		&Mesh_Earth)))
	{
		MessageBox(NULL, "Could not find Earth.x", "Meshes.exe", MB_OK);
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials_Earth = (D3DXMATERIAL*)pD3DXMtrlBuffer_Earth->GetBufferPointer();
	MeshMaterials_Earth = new D3DMATERIAL9[NumMaterials_Earth];
	MeshTextures_Earth = new LPDIRECT3DTEXTURE9[NumMaterials_Earth];

	for (DWORD i = 0; i < NumMaterials_Earth; i++)
	{
		MeshMaterials_Earth[i] = d3dxMaterials_Earth[i].MatD3D;
		MeshTextures_Earth[i] = NULL;

		if (d3dxMaterials_Earth[i].pTextureFilename != NULL && lstrlen(d3dxMaterials_Earth[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(direct3Device9,
				d3dxMaterials_Earth[i].pTextureFilename,
				&MeshTextures_Earth[i])))
			{
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
			}
		}
	}

	if (FAILED(D3DXLoadMeshFromX("Sun.x", D3DXMESH_SYSTEMMEM,
		direct3Device9, NULL,
		&pD3DXMtrlBuffer_Sun, NULL, &NumMaterials_Sun,
		&Mesh_Sun)))
	{
		MessageBox(NULL, "Could not find Sun.x", "Meshes.exe", MB_OK);
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials_Sun = (D3DXMATERIAL*)pD3DXMtrlBuffer_Sun->GetBufferPointer();
	MeshMaterials_Sun = new D3DMATERIAL9[NumMaterials_Sun];
	MeshTextures_Sun = new LPDIRECT3DTEXTURE9[NumMaterials_Sun];

	for (DWORD i = 0; i < NumMaterials_Sun; i++)
	{
		MeshMaterials_Sun[i] = d3dxMaterials_Sun[i].MatD3D;
		MeshTextures_Sun[i] = NULL;

		if (d3dxMaterials_Sun[i].pTextureFilename != NULL && lstrlen(d3dxMaterials_Sun[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(direct3Device9,
				d3dxMaterials_Sun[i].pTextureFilename,
				&MeshTextures_Sun[i])))
			{
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
			}
		}
	}

	if (FAILED(D3DXLoadMeshFromX("Mars.x", D3DXMESH_SYSTEMMEM,
		direct3Device9, NULL,
		&pD3DXMtrlBuffer_Mars, NULL, &NumMaterials_Mars,
		&Mesh_Mars)))
	{
		MessageBox(NULL, "Could not find Mars.x", "Meshes.exe", MB_OK);
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials_Mars = (D3DXMATERIAL*)pD3DXMtrlBuffer_Mars->GetBufferPointer();
	MeshMaterials_Mars = new D3DMATERIAL9[NumMaterials_Mars];
	MeshTextures_Mars = new LPDIRECT3DTEXTURE9[NumMaterials_Mars];

	for (DWORD i = 0; i < NumMaterials_Mars; i++)
	{
		MeshMaterials_Mars[i] = d3dxMaterials_Mars[i].MatD3D;
		MeshTextures_Mars[i] = NULL;

		if (d3dxMaterials_Mars[i].pTextureFilename != NULL && lstrlen(d3dxMaterials_Mars[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(direct3Device9,
				d3dxMaterials_Mars[i].pTextureFilename,
				&MeshTextures_Mars[i])))
			{
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
			}
		}
	}

	if (FAILED(D3DXLoadMeshFromX("Moon.x", D3DXMESH_SYSTEMMEM,
		direct3Device9, NULL,
		&pD3DXMtrlBuffer_Moon, NULL, &NumMaterials_Moon,
		&Mesh_Moon)))
	{
		MessageBox(NULL, "Could not find Moon.x", "Meshes.exe", MB_OK);
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials_Moon = (D3DXMATERIAL*)pD3DXMtrlBuffer_Moon->GetBufferPointer();
	MeshMaterials_Moon = new D3DMATERIAL9[NumMaterials_Moon];
	MeshTextures_Moon = new LPDIRECT3DTEXTURE9[NumMaterials_Moon];

	for (DWORD i = 0; i < NumMaterials_Moon; i++)
	{
		MeshMaterials_Moon[i] = d3dxMaterials_Moon[i].MatD3D;
		MeshTextures_Moon[i] = NULL;

		if (d3dxMaterials_Moon[i].pTextureFilename != NULL && lstrlen(d3dxMaterials_Moon[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(direct3Device9,
				d3dxMaterials_Moon[i].pTextureFilename,
				&MeshTextures_Moon[i])))
			{
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
			}
		}
	}

	if (FAILED(D3DXLoadMeshFromX("Neptun.x", D3DXMESH_SYSTEMMEM,
		direct3Device9, NULL,
		&pD3DXMtrlBuffer_Neptun, NULL, &NumMaterials_Neptun,
		&Mesh_Neptun)))
	{
		MessageBox(NULL, "Could not find Neptun.x", "Meshes.exe", MB_OK);
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials_Neptun = (D3DXMATERIAL*)pD3DXMtrlBuffer_Neptun->GetBufferPointer();
	MeshMaterials_Neptun = new D3DMATERIAL9[NumMaterials_Neptun];
	MeshTextures_Neptun = new LPDIRECT3DTEXTURE9[NumMaterials_Neptun];

	for (DWORD i = 0; i < NumMaterials_Neptun; i++)
	{
		MeshMaterials_Neptun[i] = d3dxMaterials_Neptun[i].MatD3D;
		MeshTextures_Neptun[i] = NULL;

		if (d3dxMaterials_Neptun[i].pTextureFilename != NULL && lstrlen(d3dxMaterials_Neptun[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(direct3Device9,
				d3dxMaterials_Neptun[i].pTextureFilename,
				&MeshTextures_Neptun[i])))
			{
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
			}
		}
	}

	if (FAILED(D3DXLoadMeshFromX("Uranus.x", D3DXMESH_SYSTEMMEM,
		direct3Device9, NULL,
		&pD3DXMtrlBuffer_Uranus, NULL, &NumMaterials_Uranus,
		&Mesh_Uranus)))
	{
		MessageBox(NULL, "Could not find Uranus.x", "Meshes.exe", MB_OK);
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials_Uranus = (D3DXMATERIAL*)pD3DXMtrlBuffer_Uranus->GetBufferPointer();
	MeshMaterials_Uranus = new D3DMATERIAL9[NumMaterials_Uranus];
	MeshTextures_Uranus = new LPDIRECT3DTEXTURE9[NumMaterials_Uranus];

	for (DWORD i = 0; i < NumMaterials_Uranus; i++)
	{
		MeshMaterials_Uranus[i] = d3dxMaterials_Uranus[i].MatD3D;
		MeshTextures_Uranus[i] = NULL;

		if (d3dxMaterials_Uranus[i].pTextureFilename != NULL && lstrlen(d3dxMaterials_Uranus[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFile(direct3Device9,
				d3dxMaterials_Uranus[i].pTextureFilename,
				&MeshTextures_Uranus[i])))
			{
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
			}
		}
	}

	pD3DXMtrlBuffer_Moon->Release();
	pD3DXMtrlBuffer_Mars->Release();
	pD3DXMtrlBuffer_Sun->Release();
	pD3DXMtrlBuffer_Earth->Release();
	pD3DXMtrlBuffer_Nava->Release();
	pD3DXMtrlBuffer_Neptun->Release();
	pD3DXMtrlBuffer_Uranus->Release();
}

HRESULT InitGeometry()
{
	if (loadMesh() == E_FAIL) {
		return E_FAIL;
	}

	if (loadTexture() == E_FAIL) {
		return E_FAIL;
	}

	if (createSkyBox() == E_FAIL) {
		return E_FAIL;
	}

	InitiateCamera();
	return S_OK;
}

VOID SetupWorldMatrix()
{
	D3DXMATRIX scaling_SkyBox;
	D3DXMatrixIdentity(&scaling_SkyBox);
	D3DXMatrixScaling(&scaling_SkyBox, 5.0, 5.0, 5.0);

	direct3Device9->SetTransform(D3DTS_WORLD, &scaling_SkyBox);
}
VOID SetupViewMatrix()
{
	distance = 0.001 * direction;
	currentStep = currentStep + direction;
	if (currentStep > maximumStepDirection)
	{
		direction = -1;
	}
	if (currentStep < 0)
		direction = 1;

	camera->Update();
}
VOID SetupProjectionMatrix()
{
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 500.0f);
	direct3Device9->SetTransform(D3DTS_PROJECTION, &matProj);
}

VOID SetupMatrices()
{
	SetupWorldMatrix();
	SetupViewMatrix();
	SetupProjectionMatrix();
}

float move_x_nava = -5;
float move_y_nava = 3;
float move_z_nava = -190;
float rotate_nava = 180.0f;
VOID NavaRender()
{
	D3DXMatrixRotationY(&rotate_mesh, rotate_nava);
	D3DXMatrixScaling(&scaling_mesh, 0.02, 0.02, 0.02);
	D3DXMatrixTranslation(&translate_mesh, move_x_nava, -move_y_nava, move_z_nava);
	D3DXMatrixMultiply(&translate_mesh, &rotate_mesh, &translate_mesh);
	D3DXMatrixMultiply(&worldMatrix, &translate_mesh, &scaling_mesh);
	direct3Device9->SetTransform(D3DTS_WORLD, &worldMatrix);

	for (DWORD i = 0; i < NumMaterials_Nava; i++)
	{
		direct3Device9->SetMaterial(&MeshMaterials_Nava[i]);
		direct3Device9->SetTexture(0, MeshTextures_Nava[i]);

		Mesh_Nava->DrawSubset(i);
	}
}
VOID EarthRender()
{
	D3DXMatrixScaling(&scaling_mesh, 0.05, 0.05, 0.05);
	D3DXMatrixRotationY(&rotate_mesh, (float)timeGetTime() / 5000.0f);
	D3DXMatrixTranslation(&translate_mesh, 0.0f, 0.0f, 20);
	translate_mesh = translate_mesh * rotate_mesh;
	D3DXMatrixMultiply(&worldMatrix, &translate_mesh, &scaling_mesh);
	direct3Device9->SetTransform(D3DTS_WORLD, &worldMatrix);

	for (DWORD i = 0; i < NumMaterials_Earth; i++)
	{
		direct3Device9->SetMaterial(&MeshMaterials_Earth[i]);
		direct3Device9->SetTexture(0, MeshTextures_Earth[i]);

		Mesh_Earth->DrawSubset(i);
	}

}
VOID SunRender()
{
	D3DXMatrixScaling(&scaling_mesh, 0.1, 0.1, 0.1);
	D3DXMatrixRotationX(&rotate_mesh, (float)timeGetTime() / 1000.0f);
	D3DXMatrixTranslation(&translate_mesh, 0, 0, 0);
	translate_mesh = translate_mesh * rotate_mesh;
	D3DXMatrixMultiply(&worldMatrix, &translate_mesh, &scaling_mesh);
	direct3Device9->SetTransform(D3DTS_WORLD, &worldMatrix);

	for (DWORD i = 0; i < NumMaterials_Sun; i++)
	{
		direct3Device9->SetMaterial(&MeshMaterials_Sun[i]);
		direct3Device9->SetTexture(0, MeshTextures_Sun[i]);
		direct3Device9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		direct3Device9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		direct3Device9->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 4);
		Mesh_Sun->DrawSubset(i);
	}
}
VOID MarsRender()
{
	D3DXMatrixRotationZ(&rotate_mesh, (float)timeGetTime() / 1000.0f);
	D3DXMatrixScaling(&scaling_mesh, 0.07, 0.07, 0.07);
	D3DXMatrixTranslation(&translate_mesh, 30, 2, 15);
	D3DXMatrixMultiply(&translate_mesh, &rotate_mesh, &translate_mesh);
	D3DXMatrixMultiply(&worldMatrix, &translate_mesh, &scaling_mesh);
	direct3Device9->SetTransform(D3DTS_WORLD, &worldMatrix);

	for (DWORD i = 0; i < NumMaterials_Mars; i++)
	{
		direct3Device9->SetMaterial(&MeshMaterials_Mars[i]);
		direct3Device9->SetTexture(0, MeshTextures_Mars[i]);

		Mesh_Mars->DrawSubset(i);
	}
}
VOID MoonRender()
{
	D3DXMatrixRotationY(&rotate_mesh, (float)timeGetTime() / 1000.0f);
	D3DXMatrixScaling(&scaling_mesh, 1, 1, 1);
	D3DXMatrixTranslation(&translate_mesh, -10, 15, 40);
	D3DXMatrixMultiply(&translate_mesh, &rotate_mesh, &translate_mesh);
	D3DXMatrixMultiply(&worldMatrix, &translate_mesh, &scaling_mesh);
	direct3Device9->SetTransform(D3DTS_WORLD, &worldMatrix);

	for (DWORD i = 0; i < NumMaterials_Moon; i++)
	{
		direct3Device9->SetMaterial(&MeshMaterials_Moon[i]);
		direct3Device9->SetTexture(0, MeshTextures_Moon[i]);

		Mesh_Moon->DrawSubset(i);
	}
}
VOID NeptunRender()
{
	D3DXMatrixRotationY(&rotate_mesh, (float)timeGetTime() / 100.0f);
	D3DXMatrixScaling(&scaling_mesh, 0.07, 0.07, 0.07);
	D3DXMatrixTranslation(&translate_mesh, 60, 10, 150);
	D3DXMatrixMultiply(&translate_mesh, &rotate_mesh, &translate_mesh);
	D3DXMatrixMultiply(&worldMatrix, &translate_mesh, &scaling_mesh);
	direct3Device9->SetTransform(D3DTS_WORLD, &worldMatrix);

	for (DWORD i = 0; i < NumMaterials_Neptun; i++)
	{
		direct3Device9->SetMaterial(&MeshMaterials_Neptun[i]);
		direct3Device9->SetTexture(0, MeshTextures_Neptun[i]);

		Mesh_Neptun->DrawSubset(i);
	}
}
VOID UranusRender()
{
	D3DXMatrixRotationY(&rotate_mesh, (float)timeGetTime() / 100.0f);
	D3DXMatrixScaling(&scaling_mesh, 0.07, 0.07, 0.07);
	D3DXMatrixTranslation(&translate_mesh, 25, 1, 30);
	D3DXMatrixMultiply(&translate_mesh, &rotate_mesh, &translate_mesh);
	D3DXMatrixMultiply(&worldMatrix, &translate_mesh, &scaling_mesh);
	direct3Device9->SetTransform(D3DTS_WORLD, &worldMatrix);

	for (DWORD i = 0; i < NumMaterials_Uranus; i++)
	{
		direct3Device9->SetMaterial(&MeshMaterials_Uranus[i]);
		direct3Device9->SetTexture(0, MeshTextures_Uranus[i]);

		Mesh_Uranus->DrawSubset(i);
	}
}

VOID Render()
{
	direct3Device9->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	if (SUCCEEDED(direct3Device9->BeginScene()))
	{
		SetupMatrices();

		direct3Device9->SetStreamSource(0, cubVertexBuffer, 0, sizeof(CUSTOMVERTEX));
		direct3Device9->SetFVF(D3DFVF_CUSTOMVERTEX);

		for (int i = 0; i < 6; i++)
		{
			direct3Device9->SetTexture(0, Textures[i]);
			direct3Device9->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
		}

		NavaRender();
		SunRender();
		EarthRender();
		MarsRender();
		MoonRender();
		UranusRender();
		NeptunRender();
		direct3Device9->EndScene();
	}

	direct3Device9->Present(NULL, NULL, NULL, NULL);
}

VOID Cleanup()
{
	if (cubVertexBuffer)
		cubVertexBuffer->Release();

	if (direct3Device9 != NULL)
		direct3Device9->Release();

	if (directD3D != NULL)
		directD3D->Release();

	if (MeshTextures_Nava)
	{
		for (DWORD i = 0; i < NumMaterials_Nava; i++)
		{
			if (MeshTextures_Nava[i])
				MeshTextures_Nava[i]->Release();
		}
		delete[] MeshTextures_Nava;
	}

	if (graphBuilder)
		graphBuilder->Release();

	if (mediaControl)
		mediaControl->Release();


}
VOID CleanDInput()
{
	g_pDinKeyboard->Unacquire();
	g_pDinmouse->Unacquire();
	g_pDin->Release();
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();
		CleanDInput();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

VOID keyPress(HWND hWnd) {
	if (g_Keystate[DIK_ESCAPE] & 0x80) {
		PostMessage(hWnd, WM_DESTROY, 0, 0);
	}

	if (g_Keystate[DIK_LEFT] & 0x80) {
		move_x_nava -= 1;
	}
	if (g_Keystate[DIK_RIGHT] & 0x80) {
		move_x_nava += 1;
	}
	if (g_Keystate[DIK_UP] & 0x80) {
		move_z_nava += 1;
	}
	if (g_Keystate[DIK_DOWN] & 0x80) {
		move_z_nava -= 1;
	}
	if (g_Keystate[DIK_H] & 0x80) {
		rotate_nava += 0.01;
	}
	if (g_Keystate[DIK_L] & 0x80) {
		rotate_nava -= 0.01;
	}

	if (g_Keystate[DIK_F] & 0x80) {
		camera->MoveForward(-0.1f);
	}
	if (g_Keystate[DIK_S] & 0x80) {
		camera->MoveForward(0.1f);
	}
	if (g_Keystate[DIK_D] & 0x80) {
		camera->MoveRight(0.1f);
	}
	if (g_Keystate[DIK_A] & 0x80) {
		camera->MoveRight(-0.1f);
	}
	if (g_Keystate[DIK_Q] & 0x80) {
		camera->RotateDown(-0.01f);
	}
	if (g_Keystate[DIK_Z] & 0x80) {
		camera->RotateRight(-0.01f);
	}
	if (g_Keystate[DIK_W] & 0x80) {
		camera->RotateDown(0.01f);
	}
	if (g_Keystate[DIK_X] & 0x80) {
		camera->RotateRight(0.01f);
	}

	if (g_Keystate[DIK_B] & 0x80) {
		mediaControl->Stop();
	}

	if (g_pMousestate.lZ != 0)
	{
		rotate_nava -= (g_pMousestate.lZ / 120);
	}

}
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  "DirectX Colocviu", NULL };
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow("DirectX Colocviu", "Colocviu Laborator",
		WS_OVERLAPPEDWINDOW, 100, 100, 900, 900,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	HRESULT hr = CoInitialize(NULL);

	if (SUCCEEDED(InitD3D(hWnd)))
	{
		InitDInput(hInst, hWnd);
		if (FAILED(InitDirectShow(hWnd)))
			return 0;
		if (SUCCEEDED(InitGeometry()))
		{
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			MSG msg;
			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					DetectInput();
					Render();
					keyPress(hWnd);
				}
			}
		}
	}

	UnregisterClass("DirectX Colocviu", wc.hInstance);
	return 0;
}