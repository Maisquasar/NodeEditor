#include "Render/RenderDocAPI.h"

#include <cassert>

#include "renderdoc_app.h"
#ifdef _WIN32
#include <Windows.h>
#endif

void* RenderDocAPI::m_rdoc_api = nullptr;
bool RenderDocAPI::m_shouldCapture = false;
void RenderDocAPI::Initialize()
{
#ifdef _WIN32
    // At init, on windows
    if(HMODULE mod = GetModuleHandleA("renderdoc.dll"))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&m_rdoc_api);
        assert(ret == 1);
    }
#else
    // At init, on linux/android.
    // For android replace librenderdoc.so with libVkLayer_GLES_RenderDoc.so
    if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
        assert(ret == 1);
    }
#endif

    
}

void RenderDocAPI::StartFrameCapture()
{
    if (!ShouldCaptureFrame())
        return;
    RENDERDOC_API_1_1_2* rdoc_api = static_cast<RENDERDOC_API_1_1_2*>(m_rdoc_api);
    if(rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
}

void RenderDocAPI::EndFrameCapture()
{
    if (!ShouldCaptureFrame())
        return;
    RENDERDOC_API_1_1_2* rdoc_api = static_cast<RENDERDOC_API_1_1_2*>(m_rdoc_api);
    if(rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
}
