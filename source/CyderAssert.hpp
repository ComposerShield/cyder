/******************************************************************************
 * Cyder — Hot-Reloading Audio Plugin Wrapper
 *
 * @file     CyderHeaderBar.hpp
 *
 * @author   Adam Shield
 * @date     2025-06-30
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include <juce_core/juce_core.h>

inline bool cyderAssertsEnabled = true;

#define CYDER_ASSERT(statement) \
    if (cyderAssertsEnabled) jassert(statement)

#define CYDER_ASSERT_FALSE \
    if (cyderAssertsEnabled) jassertfalse

struct ScopedDisableCyderAssert final
{
    ScopedDisableCyderAssert()  noexcept : wasEnabled(cyderAssertsEnabled) { cyderAssertsEnabled = false; }
    ~ScopedDisableCyderAssert() noexcept { cyderAssertsEnabled = wasEnabled; }
private:
    const bool wasEnabled;
};
