# Griljor Web Rewrite — ToDo list

## Playability (2 button mouse)
- [x] change the s hotkey to be move down 1 square, in addition to the x button
- [x] make a new hotkey for picking up things (F key picks up items at your feet)

Later Playability:
- [ ] set a hotkey that lets you swap what left and right mouse buttons do, in case you like moving with the other mouse button
- [ ] make shift T take everything around you that you can reach, started with the lightest items first (so you can take many items fast)
- [ ] make a new hotkey that let's you swap what's in your hand with what is in your first inventory slot (or maybe a set of funciton keys you can set things in your inventory to let you quickly swap with (advanced feature)

## Feedback from play
- [ ] sometimes seeing people in rooms when they are not really there anymore
- [x] consider something new to help you understand when you are invisible to others
- [x] check this bug: are you still visible to others when standing on a potted plant?
- [ ] consider in team games, making all teammates visible everywhere (this might be at the same time we save them from friendly fire)
- [x] player list doesn't immediately include other people that have joined the game until you see them for the first time, they should be announced when they join
- [ ] no sound yet, add some rudimentary sounds?

## Maps
- [ ] fix initial spawn in locked room without a key in castle map
- [ ] in the Playtester Paradise map, in room 9, Range's End, the block gun drops a block gun and not a block
- [ ] in the Playtester Paradise map is is possible to respawn in room 19 which is completely empty, that room should probably be deleted?
- [ ] the nuke is Playtester Paradise Radio Shack room (the building), the nuke is too heavy to pick up, even if you are holding nothing else
- [ ] are you supposed to be able to repair the broken window walls in room 10 of Playtester Paradise? What are the repair kits good for?
- [ ] I can throw a candle in the Playtester Paradise, but it loses the candle, maybe it should drop on floor. This might be a flashlight.
- [ ] the block gun in battle map, the blocks don't drop at end of flight

## Bugs in Play
- [ ] there's still a display glitch in the client sometimes when you have joined a 2nd or 3rd map sometimes, is everything getting cleared?
- [ ] when you leave the browser open too long and there are hot reloads, chrome has to be restarted (probably only a dev experience problem)
- [ ] there might be a bug when player reappears after death, they spawn in another room, appear then jump to another location in the same room quickly

## Combat Bugs
- [ ] test: exit squares — a grenade thrown at stairs or other exit square should explode on the first square in the next room
- [ ] test: explosion rays from the blast should be able to travel one square into an adjacent room as well

## Change / Add / Reorg
- [ ] wait a little longer to reset the map after no one is left in it, and consider a timer in the lobby that tells you how long until the map resets (30 seconds?)
- [ ] help users in the lobby know when server is going to get reset (when no one is left in it)
- [ ] when someone leaves the game after 5 seconds there should be a message that they left the game, but when someone disconnects suddenly, they should be said to have disconnected
- [ ] it might be nice for the dmg and health? numbers to move upwards a little more than they do
- [x] let the chat history be taller, contain more text, perhaps use up space that is not needed by the player list
- [x] make the mouse graphic not so tall, figure out a better way to display what the mouse buttons are for, it kind of needs a redesign for the modern era
- [x] need a tutorial to help teach how to pickup and manipulate inventory and learn shortcut keys (or at least a help page)
- [x] we need to provide some hotkey discovery help in the UI
- [ ] add hot key setting and mouse changes settings (but how will they save these, do we need profiles, or maybe just 3 good hotkey setting choices)

## Make teams work:
- [ ] display which team people are in the player list, make your team more obvious on the map (in colors?)
- [ ] consider friendly fire options for teammates (i.e. cannot hurt members of your own team)

## Remaining Differences with Legacy
- [ ] **friendly fire** — team membership is tracked server-side but teammates can still deal damage to each other
- [ ] **periodic item placement (`.pla` files)** — weapons and items respawn on a timer per map script; the game gets stale without this
or spawn placement
- [ ] **win conditions** — no game ever ends; the original had capture-the-flag mechanics (`flag`/`flagteams` on objects) and a game timer with rewards
- [ ] **dark rooms + flashlight** — several maps (castle, hack, outdoor) have rooms that are dark by default; flashlight items illuminate a radius around the bearer
- [ ] **day/night cycle** — some rooms are lit during the day and dark at night based on in-game time
- [ ] **spread/arc weapons** — shotguns, wide-arc blasters, and 360° explosions fire multiple projectiles; a whole weapon category is missing
- [ ] **flammable chain reactions** — flammable map objects (barrels, etc.) should ignite and explode when hit by an explosion; currently not implemented
- [x] **ammo reload from quiver** — bows should reload from arrows carried in inventory; currently they just run out
- [ ] **object look/examine messages** — each item in the original had flavor text shown when looking at or examining it; currently only a name tooltip is shown

## Production
- [ ] I had to shift reload the client after updating the server, and I hadn't loaded them in a long time, they are cached too long?
- [ ] make it possible for other collaborators to deploy a production release (give them access to the VPS, or make updates automatic when the games are empty and there are pushes to the main branch)
- [ ] consider time of day scheduled resets for production

## Later / Polish / Maintainability
- [ ] add Burt back into the plan somehow, maybe to respond to chats if you address him directly?
- [ ] add monsters that can roam the map for your to fight with
- [ ] where is the robot avatar we used to have?
