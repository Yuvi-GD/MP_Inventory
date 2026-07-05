# MP_Inventory

**MP_Inventory** is a highly optimized, fully replicated multiplayer inventory system for **Unreal Engine 5.0+**. It is designed around a stateless memory footprint and server-authoritative command execution, providing a robust foundation for multiplayer games without burdening your network or frame budget.

## Key Features

* **Multiplayer Ready:** Built entirely on `FFastArraySerializer`, ensuring precise delta replication. When an inventory changes, only the specific modified slot replicates across the network.
* **Stateless Memory:** Avoids heavy `TMap` lookups and persistent iteration caches. Utilizes an O(1) `IndexTracker` to instantly resolve UI grid slots to physical array indices.
* **Strict and Infinite Modes:** Configurable via a single flag. Use **Strict Mode** for fixed-size grid inventories (e.g., Resident Evil, Diablo) with drag-and-drop gap support, or **Infinite Mode** for appendable scrolling lists (e.g., Skyrim).
* **Live UI Sync:** Deep integration with UMG's `UTileView`. Event dispatchers automatically broadcast state changes, allowing the UI to mutate widgets in place without rebuilding the entire grid.
* **Ghost Mitigation:** Built-in engine hooks monitor logical slot reassignments and automatically instruct the client UI to clear abandoned visual slots, preventing replication ghosting.
* **Secure API:** All inventory manipulation methods (Add, Remove, Swap, Split) are exposed to both Blueprints and C++ as validate-then-mutate Server RPCs. Stack limits and weights are strictly clamped on the server to prevent client exploitation.

## Installation

1. **Clone the repository:** 
   ```bash
   git clone https://github.com/Yuvi-GD/MP_Inventory.git
   ```
2. **Add to project:** Copy the `MP_Inventory` folder into your Unreal Engine project's `Plugins` directory.
3. **Enable the plugin:** Open your project, navigate to **Edit > Plugins**, search for MP_Inventory, and check the enable box.
4. **Restart:** Restart the engine to compile and load the plugin module.

## Getting Started

1. Attach the `MP_InventoryComponent` to your Player Character or Player Controller.
2. Configure your `MaxInventorySlots` and toggle `bUseStrictSlots` depending on your game's design.
3. Call `AddItem` or `RemoveItem` from the server to mutate state.
4. Bind your UI logic to the `OnInventoryUpdated` dispatcher to drive visual updates.

## Contributing

Pull requests are welcome. Whether it is a small bug fix, a performance tweak, or expanding the API, contributions are appreciated. 

1. **Open an issue first:** For major features or architectural changes, please open an issue to discuss it before you start coding.
2. **Fork and branch:** Create a fork and do your work on a dedicated feature branch.
3. **Follow the style:** Keep your C++ and Blueprint code consistent with the existing project structure and formatting conventions.
4. **Submit a PR:** Keep your pull request focused on a single issue and explain exactly what your code does.

## License

This plugin is licensed under the GNU General Public License v3.0. See the [LICENSE](./LICENSE) file for full details.
