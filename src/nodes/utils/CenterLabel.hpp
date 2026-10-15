#pragma once

#include "../../include.hpp"

class CenterLabel : public CCLabelBMFont {
public:
    static CenterLabel* create(const ZStringView string, const ZStringView font);
protected:
    void updateLabel() override;
};