/*******************************
*      Copyright Alistar.      *
* https://github.com/lineagedr *
*                              *
*    Created on: 14/08/2021    *
*    Updated on: 21/09/2022    *
********************************/

#include "BountyHunter.h"
#include "ScriptedGossip.h"
#include "Player.h"

class BountyHunter_Player : public PlayerScript
{
public:
    BountyHunter_Player() : PlayerScript("BountyHunter_Player") {}

    void OnPVPKill(Player* killer, Player* bounty) override
    {
        if (!sBountyHunter->IsEnabled() || killer == bounty)
            return;

        if (sBountyHunter->FindBounty(bounty->GetGUID()))
        {
            switch (sBountyHunter->GetBountyPriceType(bounty->GetGUID()))
            {
            case BountyPriceType::GOLD:
                killer->ModifyMoney(sBountyHunter->GetBountyPriceAmount(bounty->GetGUID()) * 10000);
                break;
            case BountyPriceType::HONOR:
                killer->ModifyHonorPoints(sBountyHunter->GetBountyPriceAmount(bounty->GetGUID()));
                break;
            case BountyPriceType::TOKENS:
                killer->AddItem(sBountyHunter->GetTokenId(), sBountyHunter->GetBountyPriceAmount(bounty->GetGUID()));
                break;
            default:
                break;
            }
            sBountyHunter->Announce(bounty, BountyAnnounceType::TYPE_COLLECTED, killer->GetName());
            sBountyHunter->RemoveBounty(bounty->GetGUID());
        }
    }
};

class BountyHunter_Npc : public CreatureScript
{
public:
    BountyHunter_Npc() : CreatureScript("BountyHunter_Npc") {}

    inline void ModifyGossipStrings(Player* player, BountyGossipSelectText& text)
    {
        if (sBountyHunter->FindGossipInfoName(player->GetGUID()))
        {
            if (!sBountyHunter->GetBountyGossipData()[player->GetGUID()].bountyName.empty())
            {
                text.bountyName += "[|cff00ff00";
                text.bountyName += sBountyHunter->GetBountyGossipData()[player->GetGUID()].bountyName;
                text.bountyName += "|r]";
            }
            else
                text.bountyName += "[|cffff0000Unknown|r]";
        }
        else
            text.bountyName += "[|cffff0000Unknown|r]";

        const std::string selected = " [|cff00ff00Selected|r][Amount]: ";
        const std::string notSelected = " [|cffff0000Not Selected|r] ";

        switch (sBountyHunter->GetBountyGossipData()[player->GetGUID()].priceType)
        {
        case BountyPriceType::GOLD:
            text.gold += selected;
            text.gold += std::to_string(sBountyHunter->GetBountyGossipData()[player->GetGUID()].priceAmount);
            break;
        case BountyPriceType::HONOR:
            text.honor += selected;
            text.honor += std::to_string(sBountyHunter->GetBountyGossipData()[player->GetGUID()].priceAmount);
            break;
        case BountyPriceType::TOKENS:
            text.tokens += selected;
            text.tokens += std::to_string(sBountyHunter->GetBountyGossipData()[player->GetGUID()].priceAmount);
            break;
        default:
            text.gold   += notSelected;
            text.honor  += notSelected;
            text.tokens += notSelected;
            break;
        }
    }

    inline void GossipForPlaceBounty(Player* player, Creature* creature)
    {
        BountyGossipSelectText text = { "|TInterface/ICONS/achievement_bg_killingblow_berserker:25:25|tPlayer's Name ", "|TInterface/ICONS/inv_misc_coin_02:25:25|tGold", "|TInterface/ICONS/ability_dualwield:25:25|tHonor", "" };
        text.tokens = sBountyHunter->GetTokenIcon().c_str();
        text.tokens += sBountyHunter->GetTokenName().c_str();

        ModifyGossipStrings(player, text);

        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, text.bountyName.c_str(), GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_BOUNTY_NAME), "", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, text.gold.c_str(), GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_GOLD), "", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, text.honor.c_str(), GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_HONOR), "", 0, true);
        if (sBountyHunter->GetTokenId())
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, text.tokens.c_str(), GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_TOKENS), "", 0, true);
        if (sBountyHunter->IsReadyToSubmitBounty(player->GetGUID()))
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "|TInterface/ICONS/inv_misc_book_11:25:25|tSubmit bounty", GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_SUBMIT_BOUNTY));
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!IsBountyEnabled(player))
            return true;

        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/achievement_pvp_h_13:25:25|tI would like to place a bounty", GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_PLACE_BOUNTY));
        AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface/ICONS/inv_misc_note_05:25:25|tList the current bounties", GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_LIST_BOUNTY));
        if (player->GetSession()->GetSecurity() > SEC_PLAYER)
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|TInterface/ICONS/inv_misc_note_06:25:25|tWipe the current bounties", GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_WIPE_BOUNTY));
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "|TInterface/ICONS/spell_shadow_sacrificialshield:25:25|tExit", GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_EXIT));
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!IsBountyEnabled(player))
            return true;

        ClearGossipMenuFor(player);

        switch (action)
        {
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_PLACE_BOUNTY):
            GossipForPlaceBounty(player, creature);
            break;
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_LIST_BOUNTY):
            sBountyHunter->ListBounties(player, creature);
            break;
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_WIPE_BOUNTY):
            sBountyHunter->DeleteAllBounties(player);
            CloseGossipMenuFor(player);
            break;
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_SUBMIT_BOUNTY):
            if (!ValidateAmount(player, sBountyHunter->GetGossipPriceType(player->GetGUID()), sBountyHunter->GetGossipInfoPriceAmount(player->GetGUID())))
                return true;
            sBountyHunter->SubmitBounty(player);
            CloseGossipMenuFor(player);
            break;
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_EXIT):
            CloseGossipMenuFor(player);
            break;
        default:
            break;
        }

        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override
    {
        if (!IsBountyEnabled(player))
            return true;

        ClearGossipMenuFor(player);

        if (!Validate(player, action, code))
            return false;

        switch (action)
        {
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_BOUNTY_NAME):
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_GOLD):
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_HONOR):
        case static_cast<uint8>(BountyHunter_Menu::GOSSIP_TOKENS):
            OnGossipSelect(player, creature, sender, static_cast<uint8>(BountyHunter_Menu::GOSSIP_PLACE_BOUNTY));
            break;
        default:
            break;
        }

        return true;
    }

private:
    bool ValidateAmount(Player* player, BountyPriceType action, uint32 amount)
    {
        if (!amount) return false;

        switch (action)
        {
        case BountyPriceType::GOLD:
            if (!player->HasEnoughMoney(amount * 10000))
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000You don't have enough gold.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            break;
        case BountyPriceType::HONOR:
            if (player->GetHonorPoints() < amount)
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000You don't have honor points.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            break;
        case BountyPriceType::TOKENS:
            if (player->GetItemCount(sBountyHunter->GetTokenId()) < amount)
            {
                std::string tokenName = sBountyHunter->GetTokenName();
                player->GetSession()->SendAreaTriggerMessage("|cffff0000You don't have enough %s's.|r", tokenName.c_str());
                CloseGossipMenuFor(player);
                return false;
            }
            break;
        default:
            break;
        }
        return true;
    }

    bool Validate(Player* player, uint32 action, const char* code)
    {
        const std::string temp = code;
        uint32 amount = 0;

        const auto validateAmount = [&]()
        {
            // max amount of 5 numbers
            if (temp.size() > 5)
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000Amount is too large.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            // only numbers
            if (!std::all_of(temp.begin(), temp.end(), ::isdigit))
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000Amount must contain only numbers.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            // amount must be greater than 0
            amount = stoi(temp);
            if (amount <= 0)
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000Amount must be greater than 0.|r");
                CloseGossipMenuFor(player);
                return false;
            }

            switch (action)
            {
            case static_cast<uint8>(BountyHunter_Menu::GOSSIP_GOLD):
                if (amount > sBountyHunter->GetGoldMaxAmount())
                {
                    player->GetSession()->SendAreaTriggerMessage("|cffff0000Gold amount cannot exceed the maximum of %u.|r", sBountyHunter->GetGoldMaxAmount());
                    CloseGossipMenuFor(player);
                    return false;
                }
                sBountyHunter->AddGossipInfo(player->GetGUID(), { "", BountyPriceType::GOLD, amount });
                break;
            case static_cast<uint8>(BountyHunter_Menu::GOSSIP_HONOR):
                if (amount > sBountyHunter->GetHonorMaxAmount())
                {
                    player->GetSession()->SendAreaTriggerMessage("|cffff0000Honor amount cannot exceed the maximum of %u.|r", sBountyHunter->GetHonorMaxAmount());
                    CloseGossipMenuFor(player);
                    return false;
                }
                sBountyHunter->AddGossipInfo(player->GetGUID(), { "", BountyPriceType::HONOR, amount });
                break;
            case static_cast<uint8>(BountyHunter_Menu::GOSSIP_TOKENS):
                if (amount > sBountyHunter->GetTokenMaxAmount())
                {
                    player->GetSession()->SendAreaTriggerMessage("|cffff0000Token amount cannot exceed the maximum of %u.|r", sBountyHunter->GetTokenMaxAmount());
                    CloseGossipMenuFor(player);
                    return false;
                }
                sBountyHunter->AddGossipInfo(player->GetGUID(), { "", BountyPriceType::TOKENS, amount });
                break;
            default:
                break;
            }
            return true;
        };

        // validate name
        if (action == static_cast<uint8>(BountyHunter_Menu::GOSSIP_BOUNTY_NAME))
        {
            // max name length is 12 characters
            if (temp.size() > 12)
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000Character name length exceeds 12.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            // only letters
            if (!std::all_of(temp.begin(), temp.end(), [](unsigned char c) { return std::isalpha(c); }))
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000Name must contain only letters.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            // player must be online
            Player* bounty = ObjectAccessor::FindPlayerByName(temp, true);
            if (!bounty)
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000The player is offline or doesn't exist.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            // player must not have any other bounties
            if (sBountyHunter->FindBounty(bounty->GetGUID()))
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000The player already has a bounty.|r");
                CloseGossipMenuFor(player);
                return false;
            }
            // can't place a bounty on self
            if (player == bounty)
            {
                player->GetSession()->SendAreaTriggerMessage("|cffff0000You can't place a bounty on yourself.|r");
                CloseGossipMenuFor(player);
                return false;
            }

            sBountyHunter->AddGossipInfo(player->GetGUID(), { temp, BountyPriceType::NONE, 0 });
            return true;
        }

        // validate amount
        if (!validateAmount())
            return false;

        return true;
    }

    bool IsBountyEnabled(Player* player)
    {
        if (!sBountyHunter->IsEnabled())
        {
            player->GetSession()->SendAreaTriggerMessage("|cffff0000The bounty hunter is disabled.|r");
            CloseGossipMenuFor(player);
            return false;
        }
        return true;
    }
};

class BountyHunter_World : public WorldScript
{
public:
    BountyHunter_World() : WorldScript("BountyHunter_World") {}

    void OnShutdown() override
    {
        if (!sBountyHunter->IsEnabled())
            return;

        sBountyHunter->SaveBountiesToDB();
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sBountyHunter->LoadConfig();

        if (!sBountyHunter->IsEnabled())
            return;

        sBountyHunter->LoadBountiesFromDB();
    }
};

void AddSC_BountyHunter()
{
    new BountyHunter_Player();
    new BountyHunter_Npc();
    new BountyHunter_World();
}
