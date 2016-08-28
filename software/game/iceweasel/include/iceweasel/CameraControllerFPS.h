#pragma once

#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Input;
    class PhysicsWorld;
    class GravityManager;
}

class CameraControllerFPS : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(CameraControllerFPS, Urho3D::LogicComponent)

public:
    CameraControllerFPS(Urho3D::Context* context);

private:
    virtual void Start() override;
    virtual void Stop() override;
    virtual void FixedUpdate(float timeStep) override;

    void HandleCameraAngleChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::Input> input_;
    Urho3D::SharedPtr<Urho3D::PhysicsWorld> physicsWorld_;
    Urho3D::SharedPtr<Urho3D::GravityManager> gravityManager_;
    Urho3D::SharedPtr<Urho3D::Node> thisNode_;

    Urho3D::Vector3 currentVelocity_;
    Urho3D::Quaternion currentRotation_;
    float downVelocity_;
    float cameraAngleY_;
    bool jumpKeyPressed_;
};