#pragma once

class MainScript : public SquareCore::Script
{
public:
    void OnStart() override;
    void OnUpdate(float delta_time) override;
    void OnExit() override;
};