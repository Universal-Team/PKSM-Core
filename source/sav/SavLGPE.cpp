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

#include "sav/SavLGPE.hpp"
#include "i18n/i18n.hpp"
#include "pkx/PB7.hpp"
#include "utils/endian.hpp"
#include "utils/random.hpp"
#include "utils/utils.hpp"
#include "wcx/WB7.hpp"
#include <algorithm>

namespace
{
    bool isPKM(u8* pkmData)
    {
        if (LittleEndian::convertTo<u16>(pkmData + 8) == 0)
        {
            return false;
        }
        return true;
    }
}

SavLGPE::SavLGPE(std::shared_ptr<u8[]> dt) : Sav(dt, 0x100000)
{
    game    = Game::LGPE;
    PokeDex = 0x2A00;
}

u32 SavLGPE::boxOffset(u8 box, u8 slot) const
{
    return 0x5C00 + box * 30 * PB7::BOX_LENGTH + slot * PB7::BOX_LENGTH;
}

u16 SavLGPE::partyBoxSlot(u8 slot) const
{
    return LittleEndian::convertTo<u16>(&data[0x5A00 + slot * 2]);
}

void SavLGPE::partyBoxSlot(u8 slot, u16 v)
{
    LittleEndian::convertFrom<u16>(&data[0x5A00 + slot * 2], v);
}

u32 SavLGPE::partyOffset(u8 slot) const
{
    u16 boxSlot = partyBoxSlot(slot);
    if (boxSlot == 1001)
    {
        return 0;
    }
    return boxOffset(boxSlot / 30, boxSlot % 30);
}

u16 SavLGPE::boxedPkm() const
{
    return LittleEndian::convertTo<u16>(&data[0x5A00 + 14]);
}

void SavLGPE::boxedPkm(u16 v)
{
    LittleEndian::convertFrom<u16>(&data[0x5A00 + 14], v);
}

u16 SavLGPE::followPkm() const
{
    return LittleEndian::convertTo<u16>(&data[0x5A00 + 12]);
}

void SavLGPE::followPkm(u16 v)
{
    LittleEndian::convertFrom<u16>(&data[0x5A00 + 12], v);
}

u8 SavLGPE::partyCount() const
{
    u16 ret = 0;
    for (int i = 0; i < 6; i++)
    {
        if (partyBoxSlot(i) != 1001)
        {
            ret++;
        }
    }
    return ret;
}
void SavLGPE::partyCount(u8) {}

void SavLGPE::fixParty()
{
    for (int i = 0; i < 5; i++)
    {
        if (partyBoxSlot(i) == 1001 && partyBoxSlot(i + 1) != 1001)
        {
            partyBoxSlot(i, partyBoxSlot(i + 1));
            partyBoxSlot(i + 1, 1001);
            i = -1;
        }
    }
}

void SavLGPE::compressBox()
{
    u16 emptyIndex                  = 1001;
    u8 emptyData[PB7::PARTY_LENGTH] = {0};
    for (u16 i = 0; i < 1000; i++)
    {
        u32 offset = boxOffset(i / 30, i % 30);
        if (emptyIndex == 1001 && !isPKM(&data[offset]))
        {
            emptyIndex = i;
        }
        else if (emptyIndex != 1001)
        {
            if (isPKM(&data[offset]))
            {
                u32 emptyOffset = boxOffset(emptyIndex / 30, emptyIndex % 30);
                // Swap the two slots
                std::copy(&data[emptyOffset], &data[emptyOffset + PB7::PARTY_LENGTH], emptyData);
                std::copy(&data[offset], &data[offset + PB7::PARTY_LENGTH], &data[emptyOffset]);
                std::copy(emptyData, emptyData + PB7::PARTY_LENGTH, &data[offset]);
                for (int j = 0; j < partyCount(); j++)
                {
                    if (partyBoxSlot(j) == i)
                    {
                        partyBoxSlot(j, emptyIndex);
                    }
                }
                if (followPkm() == i)
                {
                    followPkm(emptyIndex);
                }
                emptyIndex++;
            }
        }
    }
}

u16 SavLGPE::check16(u8* buf, u32 len)
{
    u16 chk = 0;
    for (u32 i = 0; i < len; i++)
    {
        chk = (crc16[(buf[i] ^ chk) & 0xFF] ^ chk >> 8);
    }
    return chk;
}

void SavLGPE::resign()
{
    const u8 blockCount = 21;
    u8* tmp             = new u8[*std::max_element(chklen, chklen + blockCount)];
    const u32 csoff     = 0xB861A;

    for (u8 i = 0; i < blockCount; i++)
    {
        std::copy(&data[chkofs[i]], &data[chkofs[i] + chklen[i]], tmp);
        LittleEndian::convertFrom<u16>(&data[csoff + i * 8], check16(tmp, chklen[i]));
    }

    delete[] tmp;
}

u16 SavLGPE::TID() const
{
    return LittleEndian::convertTo<u16>(&data[0x1000]);
}

void SavLGPE::TID(u16 v)
{
    LittleEndian::convertFrom<u16>(&data[0x1000], v);
}

u16 SavLGPE::SID() const
{
    return LittleEndian::convertTo<u16>(&data[0x1002]);
}

void SavLGPE::SID(u16 v)
{
    LittleEndian::convertFrom<u16>(&data[0x1002], v);
}

u8 SavLGPE::version() const
{
    return data[0x1004];
}

void SavLGPE::version(u8 v)
{
    data[0x1004] = v;
}

u8 SavLGPE::gender() const
{
    return data[0x1005];
}

void SavLGPE::gender(u8 v)
{
    data[0x1005] = v;
}

Language SavLGPE::language() const
{
    return Language(data[0x1035]);
}

void SavLGPE::language(Language v)
{
    data[0x1035] = u8(v);
}

std::string SavLGPE::otName() const
{
    return StringUtils::getString(data.get(), 0x1000 + 0x38, 13);
}

void SavLGPE::otName(const std::string& v)
{
    StringUtils::setString(data.get(), v, 0x1000 + 0x38, 13);
}

u32 SavLGPE::money() const
{
    return LittleEndian::convertTo<u32>(&data[0x4C04]);
}

void SavLGPE::money(u32 v)
{
    LittleEndian::convertFrom<u32>(&data[0x4C04], v);
}

u8 SavLGPE::badges() const
{
    struct
    {
        u8 unimportant1 : 4;
        u8 b1 : 1;
        u8 b2 : 1;
        u8 b3 : 1;
        u8 b4 : 1;
        u8 b5 : 1;
        u8 b6 : 1;
        u8 b7 : 1;
        u8 b8 : 1;
        u8 unimportant2 : 4;
    } badgeBits;
    std::copy(&data[0x21b1], &data[0x21b1 + 2], (u8*)&badgeBits);
    return badgeBits.b1 + badgeBits.b2 + badgeBits.b3 + badgeBits.b4 + badgeBits.b5 + badgeBits.b6 + badgeBits.b7 + badgeBits.b8;
}

u16 SavLGPE::playedHours(void) const
{
    return LittleEndian::convertTo<u16>(&data[0x45400]);
}

void SavLGPE::playedHours(u16 v)
{
    LittleEndian::convertFrom<u16>(&data[0x45400], v);
}

u8 SavLGPE::playedMinutes(void) const
{
    return data[0x45402];
}

void SavLGPE::playedMinutes(u8 v)
{
    data[0x45402] = v;
}

u8 SavLGPE::playedSeconds(void) const
{
    return data[0x45403];
}

void SavLGPE::playedSeconds(u8 v)
{
    data[0x45403] = v;
}

std::unique_ptr<PKX> SavLGPE::pkm(u8 slot) const
{
    u32 off = partyOffset(slot);
    if (off != 0)
    {
        return PKX::getPKM<Generation::LGPE>(&data[off], true);
    }
    else
    {
        return emptyPkm();
    }
}

std::unique_ptr<PKX> SavLGPE::pkm(u8 box, u8 slot) const
{
    return PKX::getPKM<Generation::LGPE>(&data[boxOffset(box, slot)], true);
}

void SavLGPE::pkm(const PKX& pk, u8 box, u8 slot, bool applyTrade)
{
    if (pk.generation() == Generation::LGPE)
    {
        auto pb7 = pk.partyClone();
        if (applyTrade)
        {
            trade(*pb7);
        }

        std::copy(pb7->rawData(), pb7->rawData() + PB7::PARTY_LENGTH, &data[boxOffset(box, slot)]);
    }
}

void SavLGPE::pkm(const PKX& pk, u8 slot)
{
    if (pk.generation() == Generation::LGPE)
    {
        u32 off     = partyOffset(slot);
        u16 newSlot = partyBoxSlot(slot);
        if (pk.species() == 0)
        {
            if (off != 0)
            {
                std::fill_n(&data[off], PB7::PARTY_LENGTH, 0);
            }
            partyBoxSlot(slot, 1001);
            return;
        }
        if (off == 0)
        {
            for (int i = 999; i >= 0; i--)
            {
                if (!isPKM(&data[0x5C00 + i * PB7::PARTY_LENGTH]))
                {
                    off     = boxOffset(i / 30, i % 30);
                    newSlot = i;
                    break;
                }
            }
            if (off == 0)
            {
                // Gui::warn(i18n::localize("LGPE_NO_PARTY_SLOT"));
                return;
            }
        }

        auto pb7 = pk.partyClone();
        std::copy(pb7->rawData(), pb7->rawData() + PB7::PARTY_LENGTH, &data[off]);
        partyBoxSlot(slot, newSlot);
    }
}

void SavLGPE::trade(PKX& pk, const Date& date) const
{
    PB7* pb7 = (PB7*)&pk;
    if (pb7->egg() && !(otName() == pb7->otName() && TID() == pb7->TID() && SID() == pb7->SID() && gender() == pb7->otGender()))
    {
        pb7->metLocation(30002);
        pb7->metDate(date);
    }
    else if (!(otName() == pb7->otName() && TID() == pb7->TID() && SID() == pb7->SID() && gender() == pb7->otGender()))
    {
        pb7->currentHandler(0);
    }
    else
    {
        if (pb7->htName() != otName())
        {
            pb7->htFriendship(pb7->currentFriendship()); // copy friendship instead of resetting (don't alter CP)
            pb7->htAffection(0);
        }
        pb7->currentHandler(1);
        pb7->htName(otName());
        pb7->htGender(gender());
    }
}

std::unique_ptr<PKX> SavLGPE::emptyPkm() const
{
    return PKX::getPKM<Generation::LGPE>(nullptr, true);
}

std::string SavLGPE::boxName(u8 box) const
{
    return i18n::localize(Language(language()), "BOX") + " " + std::to_string((int)box + 1);
}
void SavLGPE::boxName(u8, const std::string&) {}

int SavLGPE::dexFormCount(int species) const
{
    for (int i = 0; i < 62; i += 2)
    {
        if (formtable[i] == species)
        {
            return formtable[i + 1];
        }
        else if (formtable[i] > species)
        {
            return 0;
        }
    }
    return 0;
}

int SavLGPE::dexFormIndex(int species, int formct, int start) const
{
    int formindex = start;
    int f         = 0;
    for (int i = 0; i < 62; i += 2)
    {
        if (formtable[i] == species)
        {
            break;
        }
        f = formtable[i + 1];
        formindex += f - 1;
    }
    return f > formct ? -1 : formindex;
}

bool SavLGPE::sanitizeFormsToIterate(int species, int& fs, int& fe, int formIn) const
{
    switch (species)
    {
        case 20:  // Raticate
        case 105: // Marowak
            fs = 0;
            fe = 1;
            return true;
        default:
            int count = dexFormCount(species);
            fs = fe = 0;
            return count < formIn;
    }
}

void SavLGPE::setDexFlags(int index, int gender, int shiny, int baseSpecies)
{
    const int brSize = 0x8C;
    int shift        = gender | (shiny << 1);
    int off          = 0x2AF0;
    int bd           = index >> 3;
    int bm           = index & 7;
    int bd1          = baseSpecies >> 3;
    int bm1          = baseSpecies & 7;

    int brSeen = shift * brSize;
    data[off + brSeen + bd] |= (u8)(1 << bm);

    bool displayed = false;
    for (u8 i = 0; i < 4; i++)
    {
        int brDisplayed = (4 + i) * brSize;
        displayed |= (data[off + brDisplayed + bd1] & (u8)(1 << bm1)) != 0;
    }

    if (!displayed && baseSpecies != index)
    {
        for (u8 i = 0; i < 4; i++)
        {
            int brDisplayed = (4 + i) * brSize;
            displayed |= (data[off + brDisplayed + bd] & (u8)(1 << bm)) != 0;
        }
    }
    if (displayed)
        return;

    data[off + (4 + shift) * brSize + bd] |= (u8)(1 << bm);
}

int SavLGPE::getDexFlags(int index, int baseSpecies) const
{
    int ret          = 0;
    const int brSize = 0x8C;
    int ofs          = PokeDex + 0x08 + 0x80 + 0x68;
    int bd           = index >> 3;
    int bm           = index & 7;
    int bd1          = baseSpecies >> 3;
    int bm1          = baseSpecies & 7;

    for (u8 i = 0; i < 4; i++)
    {
        if (data[ofs + i * brSize + bd] & (u8)(1 << bm))
        {
            ret++;
        }
        if (data[ofs + i * brSize + bd1] & (u8)(1 << bm1))
        {
            ret++;
        }
    }

    return ret;
}

void SavLGPE::dex(const PKX& pk)
{
    int n                    = pk.species();
    int MaxSpeciesID         = 809;
    int PokeDex              = 0x2A00;
    int PokeDexLanguageFlags = PokeDex + 0x550;

    if (n == 0 || n > MaxSpeciesID || pk.egg())
        return;

    int bit    = n - 1;
    int bd     = bit >> 3;
    int bm     = bit & 7;
    int gender = pk.gender() % 2;
    int shiny  = pk.shiny() ? 1 : 0;
    if (n == 351)
        shiny = 0;
    int shift = gender | (shiny << 1);

    if (n == 327) // Spinda
    {
        if ((data[PokeDex + 0x84] & (1 << (shift + 4))) != 0)
        { // Already 2
            LittleEndian::convertFrom<u32>(&data[PokeDex + 0x8E8 + shift * 4], pk.encryptionConstant());
            data[PokeDex + 0x84] |= (u8)(1 << shift);
        }
        else if ((data[PokeDex + 0x84] & (1 << shift)) == 0)
        {                                             // Not yet 1
            data[PokeDex + 0x84] |= (u8)(1 << shift); // 1
        }
    }

    int off = PokeDex + 0x08 + 0x80;
    data[off + bd] |= (u8)(1 << bm);

    int formstart = pk.alternativeForm();
    int formend   = formstart;

    int fs = 0, fe = 0;
    if (sanitizeFormsToIterate(n, fs, fe, formstart))
    {
        formstart = fs;
        formend   = fe;
    }

    for (int form = formstart; form <= formend; form++)
    {
        int bitIndex = bit;
        if (form > 0)
        {
            u8 fc = dexFormCount(n);
            if (fc > 1)
            { // actually has forms
                int f = dexFormIndex(n, fc, MaxSpeciesID - 1);
                if (f >= 0) // bit index valid
                    bitIndex = f + form;
            }
        }
        setDexFlags(bitIndex, gender, shiny, n - 1);
    }

    int lang            = u8(pk.language());
    const int langCount = 9;
    if (lang <= 10 && lang != 6 && lang != 0)
    {
        if (lang >= 7)
            lang--;
        lang--;
        if (lang < 0)
            lang = 1;
        int lbit = bit * langCount + lang;
        if (lbit >> 3 < 920)
            data[PokeDexLanguageFlags + (lbit >> 3)] |= (u8)(1 << (lbit & 7));
    }
}

int SavLGPE::dexSeen(void) const
{
    int ret = 0;
    for (int species : availableSpecies())
    {
        int forms = formCount(species);
        for (int form = 0; form < forms; form++)
        {
            int dexForms = form == 0 ? -1 : dexFormIndex(species, forms, maxSpecies() - 1);

            int index = species - 1;
            if (dexForms >= 0)
            {
                index = dexForms + form;
            }

            if (getDexFlags(index, species) > 0)
            {
                ret++;
                break;
            }
        }
    }
    return ret;
}

int SavLGPE::dexCaught(void) const
{
    int ret = 0;
    for (int species : availableSpecies())
    {
        if (data[PokeDex + 0x88 + (species - 1) / 8] & (1 << ((species - 1) % 8)))
        {
            ret++;
        }
    }
    return ret;
}

void SavLGPE::cryptBoxData(bool crypted)
{
    for (u8 box = 0; box < maxBoxes(); box++)
    {
        for (u8 slot = 0; slot < 30; slot++)
        {
            if (box * 30 + slot > 1000)
            {
                return;
            }
            std::unique_ptr<PKX> pb7 = PKX::getPKM<Generation::LGPE>(&data[boxOffset(box, slot)], true, true);
            if (!crypted)
            {
                pb7->encrypt();
            }
        }
    }
}

void SavLGPE::mysteryGift(WCX& wc, int&)
{
    if (wc.generation() == Generation::LGPE)
    {
        WB7* wb7 = (WB7*)&wc;
        if (wb7->pokemon())
        {
            if (boxedPkm() == maxSlot())
            {
                // Gui::warn(i18n::localize("LGPE_TOO_MANY_PKM"), i18n::localize("BAD_INJECT"));
                return;
            }
            auto pb7 = PKX::getPKM<Generation::LGPE>(nullptr, false);
            pb7->species(wb7->species());
            pb7->alternativeForm(wb7->alternativeForm());
            if (wb7->level() > 0)
            {
                pb7->level(wb7->level());
            }
            else
            {
                pb7->level(randomNumbers() % 100 + 1);
            }
            if (wb7->metLevel() > 0)
            {
                pb7->metLevel(wb7->metLevel());
            }
            else
            {
                pb7->metLevel(pb7->level());
            }
            pb7->TID(wb7->TID());
            pb7->SID(wb7->SID());
            for (int i = 0; i < 4; i++)
            {
                pb7->move(i, wb7->move(i));
                pb7->relearnMove(i, wb7->move(i));
            }
            if (wb7->nature() == 255)
            {
                pb7->nature(randomNumbers() % 25);
            }
            else
            {
                pb7->nature(wb7->nature());
            }
            if (wb7->gender() == 3)
            {
                pb7->gender(randomNumbers() % 3);
            }
            else
            {
                pb7->gender(wb7->gender());
            }
            pb7->heldItem(wb7->heldItem());
            pb7->encryptionConstant(wb7->encryptionConstant());
            if (wb7->version() == 0)
            {
                pb7->version(wb7->version());
            }
            else
            {
                pb7->version(version());
            }
            pb7->language(language());
            pb7->ball(wb7->ball());
            pb7->country(country());
            pb7->region(subRegion());
            pb7->consoleRegion(consoleRegion());
            pb7->metLocation(wb7->metLocation());
            pb7->eggLocation(wb7->eggLocation());
            for (int i = 0; i < 6; i++)
            {
                pb7->awakened(Stat(i), wb7->awakened(Stat(i)));
                pb7->ev(Stat(i), wb7->ev(Stat(i)));
            }
            if (wb7->nickname((Language)language()).length() == 0)
            {
                pb7->nickname(i18n::species(language(), pb7->species()));
            }
            else
            {
                pb7->nickname(wb7->nickname((Language)language()));
                pb7->nicknamed(pb7->nickname() != i18n::species(language(), pb7->species()));
            }
            if (wb7->otName((Language)language()).length() == 0)
            {
                pb7->otName(otName());
                pb7->otGender(gender());
                pb7->currentHandler(0);
            }
            else
            {
                pb7->otName(wb7->otName((Language)language()));
                pb7->htName(otName());
                pb7->otGender(wb7->otGender());
                pb7->htGender(gender());
                pb7->otFriendship(PersonalLGPE::baseFriendship(pb7->formSpecies()));
                pb7->currentHandler(1);
            }

            int numPerfectIVs = 0;
            for (Stat stat : {Stat::HP, Stat::ATK, Stat::DEF, Stat::SPD, Stat::SPATK, Stat::SPDEF})
            {
                if (wb7->iv(stat) - 0xFC < 3)
                {
                    numPerfectIVs = wb7->iv(stat) - 0xFB;
                    break;
                }
            }
            for (int iv = 0; iv < numPerfectIVs; iv++)
            {
                Stat setMeTo31 = Stat(randomNumbers() % 6);
                while (pb7->iv(setMeTo31) == 31)
                {
                    setMeTo31 = Stat(randomNumbers() % 6);
                }
                pb7->iv(setMeTo31, 31);
            }
            for (Stat stat : {Stat::HP, Stat::ATK, Stat::DEF, Stat::SPD, Stat::SPATK, Stat::SPDEF})
            {
                if (pb7->iv(stat) != 31)
                {
                    pb7->iv(stat, randomNumbers() % 32);
                }
            }

            if (wb7->otGender() == 3)
            {
                pb7->TID(TID());
                pb7->SID(SID());
            }

            // Sets the ability to the one specific to the formSpecies and sets abilitynumber (Why? Don't quite understand that)
            switch (wb7->abilityType())
            {
                case 0:
                case 1:
                case 2:
                    pb7->setAbility(wb7->abilityType());
                    break;
                case 3:
                case 4:
                    pb7->setAbility(randomNumbers() % (wb7->abilityType() - 1));
                    break;
            }

            switch (wb7->PIDType())
            {
                case 0: // Fixed value
                    pb7->PID(wb7->PID());
                    break;
                case 1: // Random
                    pb7->PID((u32)randomNumbers());
                    break;
                case 2: // Always shiny
                    pb7->PID((u32)randomNumbers());
                    pb7->shiny(true);
                    break;
                case 3: // Never shiny
                    pb7->PID((u32)randomNumbers());
                    pb7->shiny(false);
                    break;
            }

            if (wb7->egg())
            {
                pb7->egg(true);
                pb7->eggDate(wb7->date());
                pb7->nickname(i18n::species(language(), pb7->species()));
                pb7->nicknamed(true);
            }

            pb7->metDate(wb7->date());
            pb7->currentFriendship(PersonalLGPE::baseFriendship(pb7->formSpecies()));

            pb7->height(randomNumbers() % 256);
            pb7->weight(randomNumbers() % 256);
            pb7->fatefulEncounter(true);

            pb7->refreshChecksum();
            pkm(*pb7, boxedPkm());
            boxedPkm(this->boxedPkm() + 1);
        }
        else if (wb7->item())
        {
            auto valid  = validItems();
            auto limits = pouches();
            for (int itemNum = 0; itemNum < wb7->items(); itemNum++)
            {
                bool currentSet = false;
                for (size_t pouch = 0; pouch < limits.size(); pouch++)
                {
                    // Check this is the correct pouch
                    if (!currentSet && std::binary_search(valid[limits[pouch].first].begin(), valid[limits[pouch].first].end(), wb7->object(itemNum)))
                    {
                        for (int slot = 0; slot < limits[pouch].second; slot++)
                        {
                            auto occupying = item(limits[pouch].first, slot);
                            if (occupying->id() == 0)
                            {
                                occupying->id(wb7->object(itemNum));
                                occupying->count(wb7->objectQuantity(itemNum));
                                ((Item7b*)occupying.get())->newFlag(true);
                                item(*occupying, limits[pouch].first, slot);
                                currentSet = true;
                                break;
                            }
                            else if (occupying->id() == wb7->object(itemNum) && limits[pouch].first != Pouch::TM)
                            {
                                occupying->count(occupying->count() + 1);
                                item(*occupying, limits[pouch].first, slot);
                                currentSet = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void SavLGPE::item(const Item& item, Pouch pouch, u16 slot)
{
    Item7b item7b = (Item7b)item;
    auto write    = item7b.bytes();
    switch (pouch)
    {
        case Pouch::Medicine:
            if (slot < 60)
            {
                std::copy(write.begin(), write.end(), &data[slot * 4]);
            }
            break;
        case Pouch::TM:
            if (slot < 108)
            {
                std::copy(write.begin(), write.end(), &data[0xF0 + slot * 4]);
            }
            break;
        case Pouch::Candy:
            if (slot < 200)
            {
                std::copy(write.begin(), write.end(), &data[0x2A0 + slot * 4]);
            }
            break;
        case Pouch::ZCrystals:
            if (slot < 150)
            {
                std::copy(write.begin(), write.end(), &data[0x5C0 + slot * 4]);
            }
            break;
        case Pouch::Ball:
            if (slot < 50)
            {
                std::copy(write.begin(), write.end(), &data[0x818 + slot * 4]);
            }
            break;
        case Pouch::Battle:
            if (slot < 150)
            {
                std::copy(write.begin(), write.end(), &data[0x8E0 + slot * 4]);
            }
            break;
        case Pouch::KeyItem:
        case Pouch::NormalItem:
            if (slot < 150)
            {
                std::copy(write.begin(), write.end(), &data[0xB38 + slot * 4]);
            }
            break;
        default:
            break;
    }
}

std::unique_ptr<Item> SavLGPE::item(Pouch pouch, u16 slot) const
{
    switch (pouch)
    {
        case Pouch::Medicine:
            return std::make_unique<Item7b>(&data[slot * 4]);
        case Pouch::TM:
            return std::make_unique<Item7b>(&data[0xF0 + slot * 4]);
        case Pouch::Candy:
            return std::make_unique<Item7b>(&data[0x2A0 + slot * 4]);
        case Pouch::ZCrystals:
            return std::make_unique<Item7b>(&data[0x5C0 + slot * 4]);
        case Pouch::Ball:
            return std::make_unique<Item7b>(&data[0x818 + slot * 4]);
        case Pouch::Battle:
            return std::make_unique<Item7b>(&data[0x8E0 + slot * 4]);
        case Pouch::KeyItem:
        case Pouch::NormalItem:
            return std::make_unique<Item7b>(&data[0xB38 + slot * 4]);
        default:
            return nullptr;
    }
}

std::unique_ptr<WCX> SavLGPE::mysteryGift(int) const
{
    return nullptr;
}

std::vector<std::pair<Sav::Pouch, int>> SavLGPE::pouches() const
{
    return {{Pouch::Medicine, 60}, {Pouch::TM, 108}, {Pouch::Candy, 200}, {Pouch::ZCrystals, 150}, {Pouch::Ball, 50}, {Pouch::Battle, 150},
        {Pouch::NormalItem, 150}};
}

std::map<Sav::Pouch, std::vector<int>> SavLGPE::validItems() const
{
    return {{Medicine, {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 38, 39, 40, 41, 709, 903}},
        {TM, {328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354,
                 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380,
                 381, 382, 383, 384, 385, 386, 387}},
        {Candy, {50, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985,
                    986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009,
                    1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030,
                    1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051,
                    1052, 1053, 1054, 1055, 1056, 1057}},
        {ZCrystals, {51, 53, 81, 82, 83, 84, 85, 849}}, {Ball, {1, 2, 3, 4, 12, 164, 166, 168, 861, 862, 863, 864, 865, 866}},
        {Battle, {55, 56, 57, 58, 59, 60, 61, 62, 656, 659, 660, 661, 662, 663, 671, 672, 675, 676, 678, 679, 760, 762, 770, 773}},
        {NormalItem,
            {76, 77, 78, 79, 86, 87, 88, 89, 90, 91, 92, 93, 101, 102, 103, 113, 115, 121, 122, 123, 124, 125, 126, 127, 128, 442, 571, 632, 651, 795,
                796, 872, 873, 874, 875, 876, 877, 878, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 900, 901, 902}}};
}

std::string SavLGPE::pouchName(Language lang, Pouch pouch) const
{
    switch (pouch)
    {
        case Medicine:
            return i18n::localize(lang, "MEDICINE");
        case TM:
            return i18n::localize(lang, "TMS");
        case Candy:
            return i18n::localize(lang, "CANDIES");
        case ZCrystals:
            return i18n::localize(lang, "ZCRYSTALS");
        case Ball:
            return i18n::localize(lang, "CATCHING_ITEMS");
        case Battle:
            return i18n::localize(lang, "BATTLE_ITEMS");
        case NormalItem:
            return i18n::localize(lang, "ITEMS");
        default:
            return "";
    }
}

const std::set<int>& SavLGPE::availableItems(void) const
{
    if (items.empty())
    {
        items = {0, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 38, 39, 40, 41, 709, 903, 328, 329, 330, 331, 332, 333, 334, 335,
            336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362,
            363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 50, 960, 961,
            962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988,
            989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013,
            1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036,
            1037, 1038, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 51, 53, 81,
            82, 83, 84, 85, 849, 1, 2, 3, 4, 12, 164, 166, 168, 861, 862, 863, 864, 865, 866, 55, 56, 57, 58, 59, 60, 61, 62, 656, 659, 660, 661, 662,
            663, 671, 672, 675, 676, 678, 679, 760, 762, 770, 773, 76, 77, 78, 79, 86, 87, 88, 89, 90, 91, 92, 93, 101, 102, 103, 113, 115, 121, 122,
            123, 124, 125, 126, 127, 128, 442, 571, 632, 651, 795, 796, 872, 873, 874, 875, 876, 877, 878, 885, 886, 887, 888, 889, 890, 891, 892,
            893, 894, 895, 896, 900, 901, 902};
    }
    return items;
}

const std::set<int>& SavLGPE::availableMoves(void) const
{
    if (moves.empty())
    {
        moves = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
            36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
            70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
            103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
            130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156,
            157, 158, 159, 160, 161, 162, 163, 164, 182, 188, 200, 224, 227, 231, 242, 243, 247, 252, 257, 261, 263, 269, 270, 276, 280, 281, 339,
            347, 355, 364, 369, 389, 394, 398, 399, 403, 404, 405, 406, 417, 420, 430, 438, 446, 453, 483, 492, 499, 503, 504, 525, 529, 583, 585,
            603, 605, 606, 607, 729, 730, 731, 733, 734, 735, 736, 737, 738, 739, 740, 742};
    }
    return moves;
}

const std::set<int>& SavLGPE::availableSpecies(void) const
{
    if (species.empty())
    {
        for (int i = 1; i <= 151; i++)
        {
            species.insert(i);
        }
        species.insert(808);
        species.insert(809);
    }
    return species;
}

const std::set<int>& SavLGPE::availableAbilities(void) const
{
    if (abilities.empty())
    {
        fill_set(abilities, 1, maxAbility());
    }
    return abilities;
}

const std::set<int>& SavLGPE::availableBalls(void) const
{
    if (balls.empty())
    {
        fill_set(balls, 1, maxBall());
    }
    return balls;
}
