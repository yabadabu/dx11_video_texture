#pragma once

#include <stdint.h>
#include <d3d11.h>
#include "SimpleMath.h"

#define SAFE_RELEASE(x) if(x) { (x)->Release(); x = nullptr; }

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace Render {
  extern ID3D11Device*           device;
  extern ID3D11DeviceContext*    ctx;
  extern ID3D11RenderTargetView* render_target_view;
  extern uint32_t                width;
  extern uint32_t                height;

  bool create(HWND hWnd);
  void destroy();
  void swapChain();

  struct Pipeline {
    ID3D11VertexShader* vs = nullptr;
    ID3D11InputLayout*  input_layout = nullptr;
    ID3D11PixelShader*  ps = nullptr;
    bool create(const char* filename, D3D11_INPUT_ELEMENT_DESC* input_elements, uint32_t ninput_elements);
    void activate() const;
    void destroy();
  };

  enum class eTopology {
    UNDEFINED = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    TRIANGLE_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
  };

  struct Mesh {
    ID3D11Buffer* vb = nullptr;
    uint32_t      nvertices = 0;
    uint32_t      bytes_per_vertex = 0;
    eTopology     topology = eTopology::UNDEFINED;

    bool create(const void* vertices, uint32_t new_nvertices, uint32_t new_bytes_per_vertex, eTopology new_topology);
    void activate() const;
    void render() const;
    void activateAndRender() const;
    void destroy();
  };

  struct Texture {
    ID3D11Resource*           texture = nullptr;
    ID3D11ShaderResourceView* shader_resource_view = nullptr;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    uint32_t xres = 0;
    uint32_t yres = 0;
    bool create(uint32_t xres, uint32_t yres, DXGI_FORMAT new_format, bool is_dynamic);
    void activate(int slot) const;
    void destroy();
    bool updateFrom(const uint8_t* new_data, size_t data_size);
  };

  template< typename Data >
  struct Cte : public Data {
    ID3D11Buffer* cb = nullptr;
    int           slot = 0;
    bool create(int new_slot) {
      slot = new_slot;
      // Create the constant buffer
      D3D11_BUFFER_DESC bd = {};
      bd.Usage = D3D11_USAGE_DEFAULT;
      bd.ByteWidth = sizeof(Data);
      bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      bd.CPUAccessFlags = 0;
      HRESULT hr = Render::device->CreateBuffer(&bd, nullptr, &cb);
      if (FAILED(hr))
        return false;
      return true;
    }
    
    void destroy() {
      SAFE_RELEASE(cb);
    }

    void uploadToGPU() {
      Render::ctx->UpdateSubresource(cb, 0, nullptr, this, 0, 0);
    }

    void activate() {
      Render::ctx->VSSetConstantBuffers(slot, 1, &cb);
      Render::ctx->PSSetConstantBuffers(slot, 1, &cb);
    }
  };

}
