# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore module

### Module currently requires:
* Need AC commit [`d40e8946`](https://github.com/azerothcore/azerothcore-wotlk/commit/faef417d92313f3b787a5f9be24c363515a5cabf) or newer.

#### Features:
- Change you faction in bg for balance faction.

#### Config option (CFBG.conf.dist)
```ini
########################################
# Bounty Hunter Module configuration
########################################
#
#    BountyHunter.Enable
#        Description: Enables the bounty hunter module
#        Default:     0 - Disabled
#                     1 - Enabled
#
#    BountyHunter.TokenId
#        Description: Sets the Token itemId
#        Default:     0 - No Token
#
#
#    BountyHunter.MaxGoldAmount
#    BountyHunter.MaxHonorAmount
#    BountyHunter.MaxTokenAmount
#        Description: Sets the max amount that can be placed

BountyHunter.Enable = 1
BountyHunter.TokenId = 6265
BountyHunter.MaxTokenAmount = 10
BountyHunter.MaxGoldAmount  = 1000
BountyHunter.MaxHonorAmount = 75000
```

### How to install
1. Simply place the module under the `modules` folder of your AzerothCore source folder.
2. Re-run cmake and launch a clean build of AzerothCore
3. Run the SQL file in sql folder.
4. Done :)

### Edit module configuration (optional)
If you need to change the module configuration, go to your server configuration folder (where your worldserver or `worldserver.exe` is), copy `BountyHunter.conf.dist` to `BountyHunter.conf` and edit that new file.

### Usage
- Enable system `BountyHunter.Enable = 1`
- Spawn and talk to the npc

## Credits
- [Alistar](https://github.com/lineagedr) (Author of the module)
