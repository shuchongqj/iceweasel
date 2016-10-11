#pragma once
#include <Urho3D/Core/Variant.h>

struct Args : public Urho3D::RefCounted
{
    Args();

    Urho3D::StringVector resourcePaths_;
    Urho3D::String sceneName_;
    Urho3D::String networkAddress_;
    unsigned short networkPort_;
    bool editor_;
    bool server_;
    bool fullscreen_;
    bool vsync_;
    int multisample_;
};
