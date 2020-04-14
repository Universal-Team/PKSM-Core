/*
 *   This file is part of PKSM-Core
 *   Copyright (C) 2016-2020 Bernardo Giordano, Admiral Fish, piepie62, Pk11
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
 *       * Requiring preservation of specified reasonable legal notices or
 *         author attributions in that material or in the Appropriate Legal
 *         Notices displayed by works containing it.
 *       * Prohibiting misrepresentation of the origin of that material,
 *         or requiring that modified versions of such material be marked in
 *         reasonable ways as different from the original version.
 */

#ifndef BALL_HPP
#define BALL_HPP

#include "enums/Language.hpp"
#include "utils/coretypes.h"
#include <limits>
#include <type_traits>

class Ball_impl
{
    friend class Ball;

private:
    enum class BallEnum : u8
    {
        None,
        Master,
        Ultra,
        Great,
        Poke,
        Safari,
        Net,
        Dive,
        Nest,
        Repeat,
        Timer,
        Luxury,
        Premier,
        Dusk,
        Heal,
        Quick,
        Cherish,
        Fast,
        Level,
        Lure,
        Heavy,
        Love,
        Friend,
        Moon,
        Sport,
        Dream,
        Beast,

        INVALID = std::numeric_limits<std::underlying_type_t<BallEnum>>::max()
    } v;

    constexpr explicit Ball_impl(BallEnum v) : v(v) {}
    constexpr Ball_impl(const Ball_impl&) = default;
    constexpr Ball_impl(Ball_impl&&)      = default;
    constexpr Ball_impl& operator=(const Ball_impl&) = default;
    constexpr Ball_impl& operator=(Ball_impl&&) = default;

public:
    template <typename T>
    constexpr explicit operator T() const noexcept
    {
        static_assert(std::is_integral_v<T>);
        return T(v);
    }
    constexpr operator BallEnum() const noexcept { return v; }

    constexpr bool operator<(const Ball_impl& other) const noexcept { return v < other.v; }
    constexpr bool operator<=(const Ball_impl& other) const noexcept { return v <= other.v; }

    constexpr bool operator>(const Ball_impl& other) const noexcept { return v > other.v; }
    constexpr bool operator>=(const Ball_impl& other) const noexcept { return v >= other.v; }

    constexpr bool operator==(const Ball_impl& other) const noexcept { return v == other.v; }
    constexpr bool operator!=(const Ball_impl& other) const noexcept { return v != other.v; }

    const std::string& localize(Language lang) const;
};

class Ball
{
private:
    Ball_impl impl;

public:
    using EnumType = Ball_impl::BallEnum;
    constexpr Ball() noexcept : impl(EnumType::INVALID) {}
    constexpr Ball(const Ball_impl& impl) noexcept : impl(impl) {}
    constexpr explicit Ball(std::underlying_type_t<EnumType> v) noexcept : impl(EnumType{v}) {}
    template <typename T>
    constexpr explicit operator T() const noexcept
    {
        static_assert(std::is_integral_v<T>);
        return T(impl);
    }
    constexpr operator EnumType() const noexcept { return (EnumType)impl; }

    constexpr bool operator<(const Ball& other) const noexcept { return impl < other.impl; }
    constexpr bool operator<=(const Ball& other) const noexcept { return impl <= other.impl; }

    constexpr bool operator>(const Ball& other) const noexcept { return impl > other.impl; }
    constexpr bool operator>=(const Ball& other) const noexcept { return impl >= other.impl; }

    constexpr bool operator==(const Ball& other) const noexcept { return impl == other.impl; }
    constexpr bool operator!=(const Ball& other) const noexcept { return impl != other.impl; }

    constexpr bool operator<(const Ball_impl& other) const noexcept { return impl < other; }
    constexpr bool operator<=(const Ball_impl& other) const noexcept { return impl <= other; }

    constexpr bool operator>(const Ball_impl& other) const noexcept { return impl > other; }
    constexpr bool operator>=(const Ball_impl& other) const noexcept { return impl >= other; }

    constexpr bool operator==(const Ball_impl& other) const noexcept { return impl == other; }
    constexpr bool operator!=(const Ball_impl& other) const noexcept { return impl != other; }

    const std::string& localize(Language lang) const { return impl.localize(lang); }

    static constexpr Ball_impl None{EnumType::None};
    static constexpr Ball_impl Master{EnumType::Master};
    static constexpr Ball_impl Ultra{EnumType::Ultra};
    static constexpr Ball_impl Great{EnumType::Great};
    static constexpr Ball_impl Poke{EnumType::Poke};
    static constexpr Ball_impl Safari{EnumType::Safari};
    static constexpr Ball_impl Net{EnumType::Net};
    static constexpr Ball_impl Dive{EnumType::Dive};
    static constexpr Ball_impl Nest{EnumType::Nest};
    static constexpr Ball_impl Repeat{EnumType::Repeat};
    static constexpr Ball_impl Timer{EnumType::Timer};
    static constexpr Ball_impl Luxury{EnumType::Luxury};
    static constexpr Ball_impl Premier{EnumType::Premier};
    static constexpr Ball_impl Dusk{EnumType::Dusk};
    static constexpr Ball_impl Heal{EnumType::Heal};
    static constexpr Ball_impl Quick{EnumType::Quick};
    static constexpr Ball_impl Cherish{EnumType::Cherish};
    static constexpr Ball_impl Fast{EnumType::Fast};
    static constexpr Ball_impl Level{EnumType::Level};
    static constexpr Ball_impl Lure{EnumType::Lure};
    static constexpr Ball_impl Heavy{EnumType::Heavy};
    static constexpr Ball_impl Love{EnumType::Love};
    static constexpr Ball_impl Friend{EnumType::Friend};
    static constexpr Ball_impl Moon{EnumType::Moon};
    static constexpr Ball_impl Sport{EnumType::Sport};
    static constexpr Ball_impl Dream{EnumType::Dream};
    static constexpr Ball_impl Beast{EnumType::Beast};

    static constexpr Ball_impl INVALID{EnumType::INVALID};
};

#endif