# Griljor Web Rewrite — ToDo list

## Maps
- [ ] fix initial spawn in locked room without a key in castle map
- [ ] in the Playtester Paradise map, in room 9, Range's End, the block gun drops a block gun and not a block
- [ ] in the Playtester Paradise map is is possible to respawn in room 19 which is completely empty, that room should probably be deleted?
- [ ] the nuke is Playtester Paradise Radio Shack room (the building), the nuke is too heavy to pick up, even if you are holding nothing else
- [ ] are you supposed to be able to repair the broken window walls in room 10 of Playtester Paradise? What are the repair kits good for?
- [ ] I can throw a candle in the Playtester Paradise, but it loses the candle, maybe it should drop on floor. This might be a flashlight.
- [ ] the block gun in battle map, the blocks don't drop at end of flight

## Bugs in Play
- [ ] player should only be able to take things that are near it and not across the room
- [ ] there's still a display glitch in the client sometimes when you have joined a 2nd or 3rd map sometimes, is everything getting cleared?
- [ ] when you leave the browser open too long and there are hot reloads, chrome has to be restarted (probably only a dev experience problem)
- [ ] there might be a bug when player reappears after death, they spawn in another room, appear then jump to another location in the same room quickly

## Combat Bugs
- [ ] get punching to work
- [ ] bug: you cannot throw a grenade into the next room — instead your player moves toward the border; the grenade should blow up on the first square in the next room
- [ ] test: exit squares — a grenade thrown at a room border should explode on the first square in the next room
- [ ] test: explosion rays from the blast should be able to travel one square into an adjacent room as well
- [ ] bug: missiles should go faster — a player can outrun a potted plant; consider giving missiles the same 40% speedup players received, or consider reverting the player speed increase
- [ ] ammo count is a little hard to see as yellow, and is covered by the flame thrower ammo, it should be drawn on top and be a more contrasting color, like amber, and the number needs a mask behind it (a circle badge in black maybe?)

## Change / Add / Reorg
- [x] move a bunch of files to the docs folder instead and update all the references from CLAUDE.md and README.md
- [x] move nginx-example.conf to server folder and update references to it
- [ ] add something to the title screen that mentions it is an old school game from 1989 and has been modernized, include a link to the repository
- [ ] time people out of AFK games so that they don't stay on the server forever if they leave their browser tab open
- [ ] wait a little longer to reset the map after no one is left in it, and consider a timer in the lobby that tells you how long until the map resets (30 seconds?)
- [ ] help users in the lobby know when server is going to get reset (when no one is left in it)
- [ ] when someone leaves the game after 5 seconds there should be a message that they left the game, but when someone disconnects suddenly, they should be said to have disconnected
- [ ] it might be nice for the dmg and health? numbers to move upwards a little more than they do
- [ ] let the chat history be taller, contain more text, perhaps use up space that is not needed by the player list
- [ ] make the mouse graphic not so tall, figure out a better way to display what the mouse buttons are for, it kind of needs a redesign for the modern era
- [ ] need the border boxes around other player (or to make them a different color than everything else? team colors?)
- [ ] need a tutorial to help teach how to pickup and manipulate inventory and learn shortcut keys (or at least a help page)
- [ ] we need to provide some hotkey discovery help in the UI
- [ ] add hot key setting and mouse changes settings (but how will they save these, do we need profiles, or maybe just 3 good hotkey setting choices)

## Make teams work:
- [x] players start in their team room
- [x] add join team buttons (2 of them, and double the height of that map row for the avatars?)
- [ ] display which team people are in the player list, make your team more obvious on the map (in colors?)
- [ ] consider friendly fire options for teammates (i.e. cannot hurt members of your own team)

## Remaining Differences with Legacy
- [ ] **friendly fire** — team membership is tracked server-side but teammates can still deal damage to each other
- [ ] **periodic item placement (`.pla` files)** — weapons and items respawn on a timer per map script; the game gets stale without this
or spawn placement
- [ ] **win conditions** — no game ever ends; the original had capture-the-flag mechanics (`flag`/`flagteams` on objects) and a game timer with rewards
- [ ] **dark rooms + flashlight** — several maps (castle, hack, outdoor) have rooms that are dark by default; flashlight items illuminate a radius around the bearer
- [ ] **day/night cycle** — some rooms are lit during the day and dark at night based on in-game time
- [ ] **wearable items** — armor, rings, helmets with a slot system exist in item definitions but cannot be equipped
- [ ] **spread/arc weapons** — shotguns, wide-arc blasters, and 360° explosions fire multiple projectiles; a whole weapon category is missing
- [ ] **flammable chain reactions** — flammable map objects (barrels, etc.) should ignite and explode when hit by an explosion; currently not implemented
- [ ] **ammo reload from quiver** — bows should reload from arrows carried in inventory; currently they just run out
- [ ] **object look/examine messages** — each item in the original had flavor text shown when looking at or examining it; currently only a name tooltip is shown

## Later / Polish / Maintainability
- [x] add unit and integration testing
- [ ] make it possible for other collaborators to deploy a production release (give them access to the VPS)
- [ ] add Burt back into the plan somehow, maybe to respond to chats if you address him directly?
