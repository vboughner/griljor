# Griljor Web Rewrite — TODO

## Client / Server

- [ ] Item use requirements — level/XP gates on equipping or using items
- [ ] Consumables — items with health/power effects beyond the door mechanism
- [ ] Teams — lobby team selection + server-side enforcement (friendly fire, spawn zones)
- [ ] 8-direction movement — currently 4-directional only
- [ ] Persistence / auth — player profiles saved across sessions (flat JSON; currently in-memory only, lost on disconnect)
- [ ] AI monsters — NPC enemies with patrol/attack behavior
- [ ] Fix `nearbyFreeTile` in `session.ts` — currently uses `permeable` instead of `movement` for free-tile checks
- [ ] When everyone leaves a map, it should respawn and reset to original state
- [ ] Research how the original screen was laid out so the UI matches better
- [ ] Make the frontend much nicer to look at (consider a frontend design pass / design tool)
- [ ] Figure out how to use ammo — numbered weapon items should consume charges on fire
- [ ] Audit pickup and drop mechanisms — verify they are working correctly end-to-end
- [ ] Decide whether to show the border arrow indicators that reveal adjacent rooms

## Networking / Deployment

- [ ] Figure out how to set up a free VM on Oracle Cloud
- [ ] Review the deployment plan in `deployment-plan.md` and execute it
- [ ] Figure out what can be demoed next Sunday night (use Chrome + screen share in Google Meet); ideally have it deployed so others can join from their browser

## Maps

- [ ] Test unlocking a door or repairing a broken one (they appear normal on the castle map — investigate)
- [ ] Pick the best maps for showcasing in the demo

## Later / Polish

- [ ] Title screen and game listing refinements before public release
- [ ] Make it possible for others to deploy a production release (if they have VM access)
- [ ] Consider whether a code refactor is worthwhile to improve clarity and maintainability
