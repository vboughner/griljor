# Griljor Web Rewrite — ToDo list

## Make teams work:
- [ ] consider friendly fire options for teammates (i.e. cannot hurt members of your own team)
- [ ] consider in team games, making all teammates visible everywhere (this might be at the same time we save them from friendly fire)

## Maps
- [ ] fix initial spawn in locked room without a key in castle map
- [ ] in the Playtester Paradise map, in room 9, Range's End, the block gun drops a block gun and not a block, and in battle map, the blocks don't drop at end of flight
- [ ] in the Playtester Paradise map is is possible to respawn in room 19 which is completely empty, that room should probably be deleted?
- [ ] the nuke is Playtester Paradise Radio Shack room (the building), the nuke is too heavy to pick up, even if you are holding nothing else
- [ ] are you supposed to be able to repair the broken window walls in room 10 of Playtester Paradise? What are the repair kits good for?
- [ ] can throw a candle in the Playtester Paradise, but it loses the candle, maybe it should drop on floor. This might be a flashlight.

## Bugs in Play
- [ ] there might be a bug when player reappears after death, they spawn in another room, appear then jump to another location in the same room quickly
- [ ] test: exit squares — a grenade thrown at stairs or other exit square should explode on the first square in the next room
- [ ] test: explosion rays from the blast should be able to travel one square into an adjacent room as well

Playability:
- [ ] add hot key setting and mouse changes settings (but how will they save these, do we need profiles, or maybe just 3 good hotkey setting choices)
- [ ] set a hotkey that lets you swap what left and right mouse buttons do, in case you like moving with the other mouse button
- [ ] make shift T take everything around you that you can reach, started with the lightest items first (so you can take many items fast)
- [ ] make a new hotkey that let's you swap what's in your hand with what is in your first inventory slot (or maybe a set of funciton keys you can set things in your inventory to let you quickly swap with (advanced feature)

## Remaining Differences with Legacy
- [ ] **friendly fire** — team membership is tracked server-side but teammates can still deal damage to each other
- [ ] **win conditions** — no game ever ends; the original had capture-the-flag mechanics (`flag`/`flagteams` on objects) and a game timer with rewards
- [ ] **dark rooms + flashlight** — several maps (castle, hack, outdoor) have rooms that are dark by default; flashlight items illuminate a radius around the bearer
- [ ] **day/night cycle** — some rooms are lit during the day and dark at night based on in-game time
- [ ] **flammable chain reactions** — flammable map objects (barrels, etc.) should ignite and explode when hit by an explosion; currently not implemented
- [ ] **object look/examine messages** — each item in the original had flavor text shown when looking at or examining it; currently only a name tooltip is shown

## Production
- [ ] I had to shift reload the client after updating the server, and I hadn't loaded them in a long time, they are cached too long?
- [ ] make it possible for other collaborators to deploy a production release (give them access to the VPS, or make updates automatic when the games are empty and there are pushes to the main branch)
- [ ] consider time of day scheduled resets for production

## Later / Polish / Maintainability
- [ ] add Burt back into the plan somehow, maybe to respond to chats if you address him directly?
- [ ] where is the robot avatar we used to have?
- [ ] we need some monster avatars
