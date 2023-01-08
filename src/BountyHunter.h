#pragma once

#ifndef _BOUNTYHUNTER_H_
#define _BOUNTYHUNTER_H_

#include "Configuration/Config.h"
#include "ScriptMgr.h"
#include <mutex>

enum class BountyPriceType : uint8
{
    NONE   = 0,
    GOLD   = 1,
    HONOR  = 2,
    TOKENS = 3
};

enum class BountyHunter_Menu : uint8
{
    GOSSIP_PLACE_BOUNTY  = 0,
    GOSSIP_LIST_BOUNTY   = 1,
    GOSSIP_WIPE_BOUNTY   = 2,
    GOSSIP_BOUNTY_NAME   = 3,
    GOSSIP_GOLD          = 4,
    GOSSIP_HONOR         = 5,
    GOSSIP_TOKENS        = 6,
    GOSSIP_SUBMIT_BOUNTY = 7,
    GOSSIP_EXIT          = 8
};

enum class BountyAnnounceType : uint8
{
    TYPE_REGISTERED = 0,
    TYPE_COLLECTED  = 1
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

typedef std::unordered_map<ObjectGuid, BountyGossipData> BountyGossipContainer;
typedef std::unordered_map<ObjectGuid, BountyContainerData>  BountyContainer;

class BountyHunter
{
public:
    static BountyHunter* instance();

    void LoadConfig();
    bool IsEnabled() const;
    uint32 GetTokenId() const;
    uint32 GetTokenMaxAmount() const;
    uint32 GetGoldMaxAmount() const;
    uint32 GetHonorMaxAmount() const;

    bool IsReadyToSubmitBounty(const ObjectGuid& playerGuid);
    bool FindGossipInfoName(const ObjectGuid& playerGuid);
    BountyPriceType GetGossipPriceType(const ObjectGuid& playerGuid);
    uint32 GetGossipInfoPriceAmount(const ObjectGuid& playerGuid);

    void AddGossipInfo(const ObjectGuid& playerGuid, BountyGossipData gossipData);
    void RemoveGossipInfo(const ObjectGuid& playerGuid);

    void SubmitBounty(Player* player);

    BountyGossipContainer& GetBountyGossipData();

    const std::string GetTokenLink(const WorldSession* session) const;
    const std::string GetTokenIcon() const;
    const std::string GetTokenName() const;

    BountyPriceType GetBountyPriceType(const ObjectGuid& playerGuid);
    uint32 GetBountyPriceAmount(const ObjectGuid& playerGuid);

    bool FindBounty(const ObjectGuid& playerGuid);
    void AddBounty(const ObjectGuid& playerGuid, BountyContainerData data);
    void RemoveBounty(const ObjectGuid& playerGuid);

    void ListBounties(Player* player, Creature* creature);
    void DeleteAllBounties(Player* player);

    void PayForBounty(Player* player);

    void Announce(const Player* bounty, BountyAnnounceType type, const std::string_view killerName = "");
    void SaveBountiesToDB();

    void LoadBountiesFromDB();

private:
    bool   m_IsEnabled;
    uint32 m_TokenId;
    uint32 m_TokenMaxAmount;
    uint32 m_GoldMaxAmount;
    uint32 m_HonorMaxAmount;

    std::mutex m_Mu;

    BountyContainer       m_BountyContainer;
    BountyGossipContainer m_BountyGossipContainer;
};

#define sBountyHunter BountyHunter::instance()

#endif // _BOUNTYHUNTER_H_
