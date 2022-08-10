#include "BountyHunter.h"

BountyHunter* BountyHunter::instance()
{
    static BountyHunter instance;
    return &instance;
}

void BountyHunter::LoadConfig()
{
    m_IsEnabled = sConfigMgr->GetOption<bool>("BountyHunter.Enable", false);
    m_TokenId = sConfigMgr->GetOption<uint32>("BountyHunter.TokenId", 0);
    m_TokenMaxAmount = sConfigMgr->GetOption<uint32>("BountyHunter.MaxTokenAmount", 10);
    m_GoldMaxAmount = sConfigMgr->GetOption<uint32>("BountyHunter.MaxGoldAmount", 1000);
    m_HonorMaxAmount = sConfigMgr->GetOption<uint32>("BountyHunter.MaxHonorAmount", 75000);
}

bool BountyHunter::IsEnabled() const
{
    return m_IsEnabled;
}

uint32 BountyHunter::GetTokenId() const
{
    return m_TokenId;
}

uint32 BountyHunter::GetTokenMaxAmount() const
{
    return m_TokenMaxAmount;
}

uint32 BountyHunter::GetGoldMaxAmount() const
{
    return m_GoldMaxAmount;
}

uint32 BountyHunter::GetHonorMaxAmount() const
{
    return m_HonorMaxAmount;
}

bool BountyHunter::IsReadyToSubmitBounty(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyGossipContainer.find(playerGuid) != m_BountyGossipContainer.end())
        if (!m_BountyGossipContainer[playerGuid].bountyName.empty() && m_BountyGossipContainer[playerGuid].priceAmount)
            return true;

    return false;
}

bool BountyHunter::FindGossipInfoName(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyGossipContainer.find(playerGuid) != m_BountyGossipContainer.end())
        if (m_BountyGossipContainer[playerGuid].bountyName.c_str())
            return true;

    return false;
}

BountyPriceType BountyHunter::GetGossipPriceType(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyGossipContainer.find(playerGuid) != m_BountyGossipContainer.end())
        return m_BountyGossipContainer[playerGuid].priceType;

    return BountyPriceType::NONE;
}

uint32 BountyHunter::GetGossipInfoPriceAmount(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyGossipContainer.find(playerGuid) != m_BountyGossipContainer.end())
        return m_BountyGossipContainer[playerGuid].priceAmount;

    return 0;
}

void BountyHunter::AddGossipInfo(ObjectGuid playerGuid, BountyGossipData gossipData)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyGossipContainer.find(playerGuid) != m_BountyGossipContainer.end())
    {
        if (!gossipData.bountyName.empty())
            m_BountyGossipContainer[playerGuid].bountyName = gossipData.bountyName;
        if (gossipData.priceAmount)
            m_BountyGossipContainer[playerGuid].priceAmount = gossipData.priceAmount;
        if (gossipData.priceType != BountyPriceType::NONE)
            m_BountyGossipContainer[playerGuid].priceType = gossipData.priceType;
    }
    else
        m_BountyGossipContainer[playerGuid] = gossipData;
}

void BountyHunter::RemoveGossipInfo(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    m_BountyGossipContainer.erase(playerGuid);
}

bool BountyHunter::FindBounty(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyContainer.find(playerGuid) != m_BountyContainer.end())
        return true;

    return false;
}

BountyPriceType BountyHunter::GetBountyPriceType(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyContainer.find(playerGuid) != m_BountyContainer.end())
        return m_BountyContainer[playerGuid].type;

    return BountyPriceType::NONE;
}

uint32 BountyHunter::GetBountyPriceAmount(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    if (m_BountyContainer.find(playerGuid) != m_BountyContainer.end())
        return m_BountyContainer[playerGuid].amount;

    return 0;
}

void BountyHunter::AddBounty(ObjectGuid playerGuid, BountyContainerData data)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    m_BountyContainer[playerGuid] = data;
}

void BountyHunter::RemoveBounty(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    m_BountyContainer.erase(playerGuid);
}

void BountyHunter::Announce(const Player* bounty, BountyAnnounceType type, const std::string_view killerName)
{
    std::string msg = "|cffff0000[Bounty Hunter]:|r ";
    const std::string bountyName = bounty->GetName();

    const auto GetTokenType = [&]()
    {
        switch (m_BountyContainer[bounty->GetGUID()].type)
        {
        case BountyPriceType::GOLD: msg += "Gold"; break;
        case BountyPriceType::HONOR: msg += "Honor"; break;
        case BountyPriceType::TOKENS: msg += GetTokenLink(bounty->GetSession()); break;
        default:
            break;
        }
    };

    switch (type)
    {
    case BountyAnnounceType::TYPE_REGISTERED:
        msg += "A bounty has been placed on ";
        msg += bountyName;
        msg += " for x";
        msg += std::to_string(m_BountyContainer[bounty->GetGUID()].amount);
        msg += ' ';
        GetTokenType();
        msg += " slay them to collect the bounty.";
        break;
    case BountyAnnounceType::TYPE_COLLECTED:
        msg += killerName.data();
        msg += " has collected ";
        msg += "x";
        msg += std::to_string(m_BountyContainer[bounty->GetGUID()].amount);
        msg += ' ';
        GetTokenType();
        msg += " for claiming the bounty of ";
        msg += bountyName;
        break;
    }
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.c_str(), nullptr);
}

BountyGossipContainer& BountyHunter::GetBountyGossipData()
{
    return m_BountyGossipContainer;
}

const std::string BountyHunter::GetTokenLink(const WorldSession* session) const
{
    const LocaleConstant loc_idx = session->GetSessionDbLocaleIndex();
    const ItemTemplate* temp = sObjectMgr->GetItemTemplate(sBountyHunter->GetTokenId());

    if (!temp) return "[Unknown]";

    std::string name = temp->Name1;
    if (ItemLocale const* il = sObjectMgr->GetItemLocale(temp->ItemId))
        ObjectMgr::GetLocaleString(il->Name, loc_idx, name);

    std::ostringstream oss;
    oss << "|c" << std::hex << ItemQualityColors[temp->Quality]
        << std::dec << "|Hitem:" << temp->ItemId << ":" << "0" << ":" << "0" << ":"
        << "0" << ":" << "0" << ":" << "0" << ":" << "0" << ":" << "0" << ":" << "80"
        << "|h[" << name << "]|h|r";

    return oss.str();
}

const std::string BountyHunter::GetTokenIcon() const
{
    std::ostringstream ss;
    ss << "|TInterface";
    const ItemTemplate* temp = sObjectMgr->GetItemTemplate(sBountyHunter->GetTokenId());
    const ItemDisplayInfoEntry* dispInfo = nullptr;
    if (temp)
    {
        dispInfo = sItemDisplayInfoStore.LookupEntry(temp->DisplayInfoID);
        if (dispInfo)
            ss << "/ICONS/" << dispInfo->inventoryIcon;
    }
    if (!dispInfo)
        ss << "/InventoryItems/WoWUnknownItem01";
    ss << ":" << 25 << ":" << 25 << "|t";
    return ss.str();
}

const std::string BountyHunter::GetTokenName() const
{
    const ItemTemplate* temp = sObjectMgr->GetItemTemplate(sBountyHunter->GetTokenId());
    return temp ? temp->Name1 : "Unknown";
}

void BountyHunter::LoadBountiesFromDB()
{
    uint32 count = 0;
    if (QueryResult bountyList = CharacterDatabase.PQuery("SELECT * FROM bounties"))
    {
        do
        {
            Field*     fields      = bountyList->Fetch();
            ObjectGuid guid        = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());
            uint8      priceType   = fields[1].GetUInt8();
            uint32     priceAmount = fields[2].GetUInt32();

            sBountyHunter->AddBounty(guid, { static_cast<BountyPriceType>(priceType), priceAmount });
            ++count;

        } while (bountyList->NextRow());
    }
    LOG_INFO("server.loading", ">> Loaded %u bounties", count);
}

void BountyHunter::SaveBountiesToDB()
{
    if (!m_BountyContainer.empty())
    {
        CharacterDatabase.PExecute("TRUNCATE TABLE bounties");
        for (const auto& [guid, bounties] : m_BountyContainer)
            CharacterDatabase.PExecute("INSERT INTO bounties (guid, priceType, priceAmount) VALUES ('%u', '%u', '%u')",
                guid.GetCounter(), static_cast<uint8>(bounties.type), bounties.amount);
    }
}

void BountyHunter::ListBounties(Player* player, Creature* creature)
{
    std::lock_guard<std::mutex> lock(m_Mu);

    if (m_BountyContainer.empty())
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "There are no active bounties at this moment.", GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_EXIT));

    uint32 count = 0;
    for (const auto& [guid, bountyInfo] : m_BountyContainer)
    {
        if (Player* bounty = ObjectAccessor::FindPlayer(guid))
        {
            if (count >= GOSSIP_OPTION_MAX)
                break;

            std::string msg;
            msg += "[";
            msg += bounty->GetName();
            msg += "]";

            msg += " Reward: ";
            switch (bountyInfo.type)
            {
            case BountyPriceType::GOLD: msg += "Gold"; break;
            case BountyPriceType::HONOR: msg += "Honor"; break;
            case BountyPriceType::TOKENS: msg += GetTokenLink(player->GetSession()); break;
            default: break;
            }
            msg += " x";
            msg += std::to_string(bountyInfo.amount);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, msg.c_str(), GOSSIP_SENDER_MAIN, static_cast<uint8>(BountyHunter_Menu::GOSSIP_EXIT));
            ++count;
        }
    }
    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
}

void BountyHunter::DeleteAllBounties(Player* player)
{
    std::lock_guard<std::mutex> lock(m_Mu);
    CharacterDatabase.PExecute("TRUNCATE TABLE bounties");
    m_BountyContainer.clear();
    player->GetSession()->SendAreaTriggerMessage("|cff00ff00All bounties have been successfully deleted.|r");
}

void BountyHunter::PayForBounty(Player* player)
{
    const uint32 amount = m_BountyGossipContainer[player->GetGUID()].priceAmount;
    switch (m_BountyGossipContainer[player->GetGUID()].priceType)
    {
    case BountyPriceType::GOLD:
        player->ModifyMoney(-(amount * 10000));
        break;
    case BountyPriceType::HONOR:
        player->ModifyHonorPoints(-amount);
        break;
    case BountyPriceType::TOKENS:
        player->DestroyItemCount(sBountyHunter->GetTokenId(), amount, true, false);
        break;
    default:
        break;
    }
}

void BountyHunter::SubmitBounty(Player* player)
{
    if (Player* bounty = ObjectAccessor::FindPlayerByName(m_BountyGossipContainer[player->GetGUID()].bountyName, true))
    {
        m_BountyContainer[bounty->GetGUID()].type = m_BountyGossipContainer[player->GetGUID()].priceType;
        m_BountyContainer[bounty->GetGUID()].amount = m_BountyGossipContainer[player->GetGUID()].priceAmount;

        PayForBounty(player);

        player->GetSession()->SendAreaTriggerMessage("|cff00ff00You've placed a bounty on %s.|r", bounty->GetName().c_str());
        Announce(bounty, BountyAnnounceType::TYPE_REGISTERED, "");
    }
    else
        player->GetSession()->SendAreaTriggerMessage("|cffff0000The player is offline or doesn't exist.|r");

    RemoveGossipInfo(player->GetGUID());
}
