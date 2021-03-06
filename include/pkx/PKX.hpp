/*
 *   This file is part of PKSM-Core
 *   Copyright (C) 2016-2020 Bernardo Giordano, Admiral Fish, piepie62
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

#ifndef PKX_HPP
#define PKX_HPP

#include "i18n/Language.hpp"
#include "sav/Item.hpp"
#include "utils/DateTime.hpp"
#include "utils/Ribbons.hpp"
#include "utils/coretypes.h"
#include "utils/genToPkx.hpp"
#include "utils/generation.hpp"
#include "utils/stat.hpp"
#include <memory>
#include <string>

class Sav;
class PKFilter;

class PKX
{
private:
    bool directAccess;
    virtual int eggYear(void) const  = 0;
    virtual void eggYear(int v)      = 0;
    virtual int eggMonth(void) const = 0;
    virtual void eggMonth(int v)     = 0;
    virtual int eggDay(void) const   = 0;
    virtual void eggDay(int v)       = 0;
    virtual int metYear(void) const  = 0;
    virtual void metYear(int v)      = 0;
    virtual int metMonth(void) const = 0;
    virtual void metMonth(int v)     = 0;
    virtual int metDay(void) const   = 0;
    virtual void metDay(int v)       = 0;

protected:
    struct PrivateConstructor
    {
    };

    PKX(u8* data, size_t length, bool directAccess = false);

    u32 expTable(u8 row, u8 col) const;
    u8 blockPosition(u8 index) const;
    u8 blockPositionInvert(u8 index) const;
    u32 seedStep(u32 seed);
    virtual void reorderMoves(void);

    virtual void crypt(void)         = 0;
    virtual void shuffleArray(u8 sv) = 0;

    u32 length = 0;
    u8* data;

public:
    static constexpr int PKSM_MAX_SPECIES = 890;

    static std::unique_ptr<PKX> getPKM(Generation gen, u8* data, bool party = false, bool directAccess = false);
    template <Generation g>
    static std::unique_ptr<typename GenToPkx<g>::PKX> getPKM(u8* data, bool party = false, bool directAccess = false)
    {
        return std::make_unique<typename GenToPkx<g>::PKX>(PrivateConstructor{}, data, party, directAccess);
    }

    // Returns null if length is not valid for that generation, and a party Pokemon depending on length
    static std::unique_ptr<PKX> getPKM(Generation gen, u8* data, size_t length, bool directAccess = false);
    template <Generation g>
    static std::unique_ptr<typename GenToPkx<g>::PKX> getPKM(u8* data, size_t length, bool directAccess = false)
    {
        if (GenToPkx<g>::PKX::PARTY_LENGTH == length || GenToPkx<g>::PKX::BOX_LENGTH == length)
        {
            return getPKM<g>(data, length == GenToPkx<g>::PKX::PARTY_LENGTH, directAccess);
        }
        return nullptr;
    }
    static u8 genFromBytes(u8* data, size_t length);

    virtual ~PKX();
    PKX(const PKX& pk);
    PKX(PKX&& pk);
    PKX& operator=(const PKX& pk);
    PKX& operator=(PKX&& pk);
    bool operator==(const PKFilter& filter) const;

    virtual std::string extension(void) const { return ".pk" + genToString(generation()); }

    u8* rawData(void) { return data; }
    const u8* rawData(void) const { return data; }
    u32 getLength(void) const { return length; }
    virtual bool isParty(void) const = 0;

    virtual void decrypt(void);
    virtual void encrypt(void);
    virtual bool isEncrypted() const = 0;

    virtual std::unique_ptr<PK3> convertToG3(Sav& save) const;
    virtual std::unique_ptr<PK4> convertToG4(Sav& save) const;
    virtual std::unique_ptr<PK5> convertToG5(Sav& save) const;
    virtual std::unique_ptr<PK6> convertToG6(Sav& save) const;
    virtual std::unique_ptr<PK7> convertToG7(Sav& save) const;
    virtual std::unique_ptr<PB7> convertToLGPE(Sav& save) const;
    virtual std::unique_ptr<PK8> convertToG8(Sav& save) const;
    virtual std::unique_ptr<PKX> clone(void) const = 0;
    std::unique_ptr<PKX> partyClone(void) const;

    virtual Generation generation(void) const = 0;
    bool originGen7(void) const;
    bool originGen6(void) const;
    bool originGen5(void) const;
    bool originGen4(void) const;
    bool originGen3(void) const;
    int originGenNumber(void) const;
    void fixMoves(void);

    static u32 getRandomPID(u16 species, u8 gender, u8 originGame, u8 nature, u8 form, u8 abilityNum, u32 oldPid, Generation gen);
    static u8 genderFromRatio(u32 pid, u8 gt);

    // BLOCK A
    virtual u32 encryptionConstant(void) const = 0;
    virtual void encryptionConstant(u32 v)     = 0;
    virtual u16 sanity(void) const             = 0;
    virtual void sanity(u16 v)                 = 0;
    virtual u16 checksum(void) const           = 0;
    virtual void checksum(u16 v)               = 0;
    virtual u16 species(void) const            = 0;
    virtual void species(u16 v)                = 0;
    virtual u16 heldItem(void) const           = 0;
    virtual void heldItem(u16 v)               = 0;
    void heldItem(const Item& item) { heldItem(item.id()); }
    virtual u16 TID(void) const               = 0;
    virtual void TID(u16 v)                   = 0;
    virtual u16 SID(void) const               = 0;
    virtual void SID(u16 v)                   = 0;
    virtual u32 experience(void) const        = 0;
    virtual void experience(u32 v)            = 0;
    virtual u16 ability(void) const           = 0;
    virtual void ability(u16 v)               = 0;
    virtual u8 abilityNumber(void) const      = 0;
    virtual void abilityNumber(u8 v)          = 0;
    virtual void setAbility(u8 abilityNumber) = 0;
    virtual u16 markValue(void) const         = 0;
    virtual void markValue(u16 v)             = 0;
    virtual u32 PID(void) const               = 0;
    virtual void PID(u32 v)                   = 0;
    virtual u8 nature(void) const             = 0;
    virtual void nature(u8 v)                 = 0;
    virtual bool fatefulEncounter(void) const = 0;
    virtual void fatefulEncounter(bool v)     = 0;
    virtual u8 gender(void) const             = 0;
    virtual void gender(u8 g)                 = 0;
    virtual u16 alternativeForm(void) const   = 0;
    virtual void alternativeForm(u16 v)       = 0;
    virtual u8 ev(Stat ev) const              = 0;
    virtual void ev(Stat ev, u8 v)            = 0;
    virtual u8 contest(u8 contest) const      = 0;
    virtual void contest(u8 contest, u8 v)    = 0;
    virtual u8 pkrs(void) const               = 0;
    virtual void pkrs(u8 v)                   = 0;
    virtual u8 pkrsDays(void) const           = 0;
    virtual void pkrsDays(u8 v)               = 0;
    virtual u8 pkrsStrain(void) const         = 0;
    virtual void pkrsStrain(u8 v)             = 0;
    virtual bool hasRibbon(Ribbon rib) const  = 0;
    virtual bool ribbon(Ribbon rib) const     = 0;
    virtual void ribbon(Ribbon rib, bool v)   = 0;

    // BLOCK B
    virtual std::string nickname(void) const    = 0;
    virtual void nickname(const std::string& v) = 0;
    virtual u16 move(u8 move) const             = 0;
    virtual void move(u8 move, u16 v)           = 0;
    virtual u16 relearnMove(u8 move) const      = 0;
    virtual void relearnMove(u8 move, u16 v)    = 0;
    virtual u8 PP(u8 move) const                = 0;
    virtual void PP(u8 move, u8 v)              = 0;
    virtual u8 PPUp(u8 move) const              = 0;
    virtual void PPUp(u8 move, u8 v)            = 0;
    virtual u8 iv(Stat iv) const                = 0;
    virtual void iv(Stat iv, u8 v)              = 0;
    virtual bool egg(void) const                = 0;
    virtual void egg(bool v)                    = 0;
    virtual bool nicknamed(void) const          = 0;
    virtual void nicknamed(bool v)              = 0;

    // BLOCK C
    virtual u8 currentHandler(void) const = 0;
    virtual void currentHandler(u8 v)     = 0;

    // BLOCK D
    virtual std::string otName(void) const    = 0;
    virtual void otName(const std::string& v) = 0;
    virtual u8 otFriendship(void) const       = 0;
    virtual void otFriendship(u8 v)           = 0;
    // Raw information handled in private functions
    virtual Date eggDate(void) const { return Date{(u8)eggDay(), (u8)eggMonth(), (u32)eggYear()}; }
    virtual void eggDate(const Date& v)
    {
        eggDay(v.day());
        eggMonth(v.month());
        eggYear(v.year());
    }
    virtual Date metDate(void) const { return Date{(u8)metDay(), (u8)metMonth(), (u32)metYear()}; }
    virtual void metDate(const Date& v)
    {
        metDay(v.day());
        metMonth(v.month());
        metYear(v.year());
    }
    virtual u16 eggLocation(void) const   = 0;
    virtual void eggLocation(u16 v)       = 0;
    virtual u16 metLocation(void) const   = 0;
    virtual void metLocation(u16 v)       = 0;
    virtual u8 ball(void) const           = 0;
    virtual void ball(u8 v)               = 0;
    virtual u8 metLevel(void) const       = 0;
    virtual void metLevel(u8 v)           = 0;
    virtual u8 otGender(void) const       = 0;
    virtual void otGender(u8 v)           = 0;
    virtual u8 version(void) const        = 0;
    virtual void version(u8 v)            = 0;
    virtual Language language(void) const = 0;
    virtual void language(Language v)     = 0;

    virtual u8 currentFriendship(void) const = 0;
    virtual void currentFriendship(u8 v)     = 0;
    virtual void refreshChecksum(void)       = 0;
    virtual u8 hpType(void) const            = 0;
    virtual void hpType(u8 v)                = 0;
    virtual u16 TSV(void) const              = 0;
    virtual u16 PSV(void) const              = 0;
    u32 versionTID(void) const;
    u32 versionSID(void) const;
    u32 formatTID(void) const;
    u32 formatSID(void) const;
    virtual u8 level(void) const        = 0;
    virtual void level(u8 v)            = 0;
    virtual bool shiny(void) const      = 0;
    virtual void shiny(bool v)          = 0;
    virtual u16 formSpecies(void) const = 0;
    virtual u16 stat(Stat stat) const   = 0;

    // Hehehehe... to be done
    // virtual u8 sleepTurns(void) const = 0;
    // virtual void sleepTurns(u8 v) = 0;
    // virtual bool poison(void) const = 0;
    // virtual void poison(bool v) = 0;
    // virtual bool burn(void) const = 0;
    // virtual void burn(bool v) = 0;
    // virtual bool frozen(void) const = 0;
    // virtual void frozen(bool v) = 0;
    // virtual bool paralyzed(void) const = 0;
    // virtual void paralyzed(bool v) = 0;
    // virtual bool toxic(void) const = 0;
    // virtual void toxic(bool v) = 0;
    // -1 means not a party pokemon
    virtual int partyCurrHP(void) const      = 0;
    virtual void partyCurrHP(u16 v)          = 0;
    virtual int partyStat(Stat stat) const   = 0;
    virtual void partyStat(Stat stat, u16 v) = 0;
    virtual int partyLevel(void) const       = 0;
    virtual void partyLevel(u8 v)            = 0;
    // Takes any calculated stats and writes them into the party offsets, provided they exist
    virtual void updatePartyData(void) = 0;

    // Personal interface
    virtual u8 baseHP(void) const         = 0;
    virtual u8 baseAtk(void) const        = 0;
    virtual u8 baseDef(void) const        = 0;
    virtual u8 baseSpe(void) const        = 0;
    virtual u8 baseSpa(void) const        = 0;
    virtual u8 baseSpd(void) const        = 0;
    virtual u8 type1(void) const          = 0;
    virtual u8 type2(void) const          = 0;
    virtual u8 genderType(void) const     = 0;
    virtual u8 baseFriendship(void) const = 0;
    virtual u8 expType(void) const        = 0;
    virtual u16 abilities(u8 n) const     = 0;
    virtual u16 formStatIndex(void) const = 0;
};

#endif
