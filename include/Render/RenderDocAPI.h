#pragma once

class RenderDocAPI
{
public:
    static void Initialize();

    static void StartFrameCapture();
    static void EndFrameCapture();

    static bool ShouldCaptureFrame() { return m_shouldCapture; }
    static void SetShouldCaptureFrame(bool value) { m_shouldCapture = value; }
    
private:
    static void* m_rdoc_api;
    static bool m_shouldCapture;
};
