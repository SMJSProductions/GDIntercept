#pragma once

#include "../../include.hpp"
#include "CenterLabel.hpp"

class Character : public CenterLabel {
public:
    static Character* create(const char character, const ZStringView font, const bool rescale = false);

    void setCString(const char* label) override;
protected:
    void updateLabel() override;
private:
    bool m_rescale;

    Character(const bool rescale);
};