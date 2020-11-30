#define _CRT_SECURE_NO_WARNINGS
#include <d3dcompiler.h>
#include "dx11_basic.h"

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

namespace Render {

  static IDXGISwapChain* swap_chain = nullptr;
  ID3D11Device* device = nullptr;
  ID3D11DeviceContext* ctx = nullptr;
  ID3D11RenderTargetView* render_target_view = nullptr;
  ID3D11SamplerState*     sampler_clamp_linear = nullptr;

  uint32_t                render_width = 0;
  uint32_t                render_height = 0;

  bool create(HWND hWnd) {
    HRESULT hr;

    RECT rc;
    GetClientRect(hWnd, &rc);
    render_width = rc.right - rc.left;
    render_height = rc.bottom - rc.top;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);
    D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = render_width;
    sd.BufferDesc.Height = render_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, numFeatureLevels,
      D3D11_SDK_VERSION, &sd, &swap_chain, &device, &featureLevel, &ctx);
    if (FAILED(hr))
      return false;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
      return false;
    hr = device->CreateRenderTargetView(pBackBuffer, nullptr, &render_target_view);
    pBackBuffer->Release();
    if (FAILED(hr))
      return false;

    ctx->OMSetRenderTargets(1, &render_target_view, nullptr);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)render_width;
    vp.Height = (FLOAT)render_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    ctx->RSSetViewports(1, &vp);

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&sampDesc, &sampler_clamp_linear);
    if (FAILED(hr))
      return false;
    ctx->PSSetSamplers(0, 1, &sampler_clamp_linear);

    return true;
  }

  void destroy() {
    SAFE_RELEASE(sampler_clamp_linear);
    SAFE_RELEASE(render_target_view);
    SAFE_RELEASE(swap_chain);
    SAFE_RELEASE(ctx);
    SAFE_RELEASE(device);
  }

  void swapChain() {
    swap_chain->Present(0, 0);
  }

  //--------------------------------------------------------------------------------------
  // Helper for compiling shaders with D3DCompile
  //
  // With VS 11, we could load up prebuilt .cso files instead...
  //--------------------------------------------------------------------------------------
  bool compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
  {
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;

    wchar_t wFilename[MAX_PATH];
    mbstowcs(wFilename, szFileName, MAX_PATH);

    hr = D3DCompileFromFile(wFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
      dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr)) {
      if (pErrorBlob) {
        OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
        pErrorBlob->Release();
      }
      return false;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return true;
  }

  //--------------------------------------------------------------------------------------
  bool Pipeline::create(const char* filename, D3D11_INPUT_ELEMENT_DESC* input_elements, uint32_t ninput_elements) {
    HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    if (!compileShaderFromFile(filename, "VS", "vs_4_0", &pVSBlob))
      return false;

    // Create the vertex shader
    hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &vs);
    if (FAILED(hr)) {
      pVSBlob->Release();
      return false;
    }

    // Create the input layout
    hr = device->CreateInputLayout(input_elements, ninput_elements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &input_layout);
    pVSBlob->Release();
    if (FAILED(hr))
      return false;

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    if (!compileShaderFromFile(filename, "PS", "ps_4_0", &pPSBlob))
      return false;

    // Create the pixel shader
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &ps);
    pPSBlob->Release();
    if (FAILED(hr))
      return false;

    return true;
  }

  void Pipeline::destroy() {
    SAFE_RELEASE(vs);
    SAFE_RELEASE(ps);
    SAFE_RELEASE(input_layout);
  }

  void Pipeline::activate() const {
    ctx->IASetInputLayout(input_layout);
    ctx->VSSetShader(vs, nullptr, 0);
    ctx->PSSetShader(ps, nullptr, 0);
  }

  //--------------------------------------------------------------------------------------
  bool Mesh::create(const void* vertices, uint32_t new_nvertices, uint32_t new_bytes_per_vertex, eTopology new_topology) {
    topology = new_topology;
    nvertices = new_nvertices;
    bytes_per_vertex = new_bytes_per_vertex;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = nvertices * bytes_per_vertex;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    HRESULT hr = device->CreateBuffer(&bd, &InitData, &vb);
    if (FAILED(hr))
      return false;

    return true;
  }

  void Mesh::activate() const {
    // Set vertex buffer
    UINT stride = bytes_per_vertex;
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    ctx->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)topology);
  }

  void Mesh::render() const {
    ctx->Draw(nvertices, 0);
  }

  void Mesh::activateAndRender() const {
    activate();
    render();
  }

  void Mesh::destroy() {
    SAFE_RELEASE(vb);
  }

  //--------------------------------------------------------------------------------------
  bool Texture::create(uint32_t new_xres, uint32_t new_yres, DXGI_FORMAT new_format, bool is_dynamic) {
    xres = new_xres;
    yres = new_yres;
    format = new_format;
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = xres;
    desc.Height = yres;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;  
    if (is_dynamic) {
      desc.Usage = D3D11_USAGE_DYNAMIC;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    ID3D11Texture2D* tex2d = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&texture);
    if (FAILED(hr))
      return false;

    // Create a resource view so we can use the data in a shader
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    ZeroMemory(&srv_desc, sizeof(srv_desc));
    srv_desc.Format = new_format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = desc.MipLevels;
    hr = device->CreateShaderResourceView(texture, &srv_desc, &shader_resource_view);
    if (FAILED(hr))
      return false;
    
    return true;
  }

  void Texture::activate(int slot) const {
    ctx->PSSetShaderResources(slot, 1, &shader_resource_view);
  }

  void Texture::destroy() {
    SAFE_RELEASE(texture);
    SAFE_RELEASE(shader_resource_view);
  }

  bool Texture::updateFromIYUV(const uint8_t* data, size_t data_size) {
    assert(data);
    D3D11_MAPPED_SUBRESOURCE ms;
    HRESULT hr = ctx->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    if (FAILED(hr))
      return false;

    uint32_t bytes_per_texel = 1;
    assert(format == DXGI_FORMAT_R8_UNORM);
    assert(data_size == xres * yres * 3 / 4);

    const uint8_t* src = data;
    uint8_t*       dst = (uint8_t*)ms.pData;

    // Copy the Y lines
    uint32_t nlines = yres / 2;
    uint32_t bytes_per_row = xres * bytes_per_texel;
    for (uint32_t y = 0; y < nlines; ++y) {
      memcpy(dst, src, bytes_per_row);
      src += bytes_per_row;
      dst += ms.RowPitch;
    }

    // Now the U and V lines, need to add Width/2 pixels of padding between each line
    uint32_t uv_bytes_per_row = bytes_per_row / 2;
    for (uint32_t y = 0; y < nlines; ++y) {
      memcpy(dst, src, uv_bytes_per_row);
      src += uv_bytes_per_row;
      dst += ms.RowPitch;
    }

    ctx->Unmap(texture, 0);
    return true;
  }

};

