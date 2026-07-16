#pragma once

#include <json.hpp>
#include <Stream.hpp>
#include <LookupTable.hpp>
#include <fmt/chrono.h>
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "concepts.hpp"
#include "macro_utils.hpp"
#include "objects/Theme.hpp"

using namespace nlohmann;
using namespace geode::prelude;

#define PADDING 5.0f
#define FULL_OPACITY 0xFF

#define ZERO_POINT ccp(0, 0)

#define TOP_LEFT ccp(0, 1)
#define TOP_CENTER ccp(0.5f, 1)
#define TOP_RIGHT ccp(1, 1)
#define CENTER_LEFT ccp(0, 0.5f)
#define CENTER ccp(0.5f, 0.5f)
#define CENTER_RIGHT ccp(1, 0.5f)
#define BOTTOM_LEFT ccp(0, 0)
#define BOTTOM_CENTER ccp(0.5f, 0)
#define BOTTOM_RIGHT ccp(1, 0)

constexpr ccColor3B LIGHTER_BROWN_3B({ 0xBF, 0x72, 0x3E });
constexpr ccColor4B LIGHTER_BROWN_4B({ 0xBF, 0x72, 0x3E, FULL_OPACITY });
constexpr ccColor3B LIGHT_BROWN_3B({ 0xB0, 0x5C, 0x34 });
constexpr ccColor4B LIGHT_BROWN_4B({ 0xB0, 0x5C, 0x34, FULL_OPACITY });
constexpr ccColor3B BROWN_3B({ 0xA0, 0x54, 0x34 });
constexpr ccColor4B BROWN_4B({ 0xA0, 0x54, 0x34, FULL_OPACITY });
constexpr ccColor3B DARK_BROWN_3B({ 0x82, 0x40, 0x21 });
constexpr ccColor4B DARK_BROWN_4B({ 0x82, 0x40, 0x21, FULL_OPACITY });