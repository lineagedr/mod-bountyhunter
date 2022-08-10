#pragma once

#ifndef _BOUNTYHUNTER_H_
#define _BOUNTYHUNTER_H_

#include "Configuration/Config.h"
#include "ScriptMgr.h"
#include <mutex>

enum class BountyPriceType : uint8
{
    NONE = 0,
    GOLD,
    HONOR,
    TOKENS
};

enum class BountyHunter_Menu : uint8
{
    GOSSIP_PLACE_BOUNTY = 0,
    GOSSIP_LIST_BOUNTY,
    GOSSIP_WIPE_BOUNTY,
    GOSSIP_BOUNTY_NAME,
    GOSSIP_GOLD,
    GOSSIP_HONOR,
    GOSSIP_TOKENS,
    GOSSIP_SUBMIT_BOUNTY,
    GOSSIP_EXIT,
};

enum class BountyAnnounceType : uint8
{
    TYPE_REGISTERED = 0,
    TYPE_COLLECTED
};

struct BountyGossipSelectText
{
    std::string bountyName, gold, honor, tokens;
};

struct BountyGossipData
{
    std::string bountyName;
    BountyPriceType priceType{ BountyPriceType::NONE };
    uint32 priceAmount{ 0 };
};

struct BountyContainerData
{
    BountyPriceType type{ BountyPriceType::NONE };
    uint32 amount{ 0 };
};

class BountyHunter
{
public:
    typedef std::unordered_map<ObjectGuid, BountyGossipData> BountyGossipContainer;
    typedef std::unordered_map<ObjectGuid, BountyContainerData>  BountyContainer;

    static BountyHunter* instance();
    std::mutex m_Mu;

    void LoadConfig();
    bool IsEnabled() const;
    uint32 GetTokenId() const;
    uint32 GetTokenMaxAmount() const;
    uint32 GetGoldMaxAmount() const;
    uint32 GetHonorMaxAmount() const;

    bool IsReadyToSubmitBounty(ObjectGuid playerGuid);
    bool FindGossipInfoName(ObjectGuid playerGuid);
    BountyPriceType GetGossipPriceType(ObjectGuid playerGuid);
    uint32 GetGossipInfoPriceAmount(ObjectGuid playerGuid);

    void AddGossipInfo(ObjectGuid playerGuid, BountyGossipData gossipData);
    void RemoveGossipInfo(ObjectGuid playerGuid);

    void SubmitBounty(Player* player);

    BountyGossipContainer& GetBountyGossipData();

    const std::string GetTokenLink(const WorldSession* session) const;
    const std::string GetTokenIcon() const;
    const std::string GetTokenName() const;

    BountyPriceType GetBountyPriceType(ObjectGuid playerGuid);
    uint32 GetBountyPriceAmount(ObjectGuid playerGuid);

    bool FindBounty(ObjectGuid playerGuid);
    void AddBounty(ObjectGuid playerGuid, BountyContainerData data);
    void RemoveBounty(ObjectGuid playerGuid);

    void ListBounties(Player* player, Creature* creature);
    void DeleteAllBounties(Player* player);

    void PayForBounty(Player* player);

    void Announce(const Player* bounty, BountyAnnounceType type, const std::string_view killerName);
    void SaveBountiesToDB();

    void LoadBountiesFromDB();

private:
    bool   m_IsEnabled;
    uint32 m_TokenId;
    uint32 m_TokenMaxAmount;
    uint32 m_GoldMaxAmount;
    uint32 m_HonorMaxAmount;

    BountyContainer m_BountyContainer;
    BountyGossipContainer m_BountyGossipContainer;
};

#define sBountyHunter BountyHunter::instance()

#endif // _BOUNTYHUNTER_H_
