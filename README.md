# MP_Inventory

**MP_Inventory** is a fully replicated multiplayer inventory and trading system for **Unreal Engine**. It provides robust item storage, access control, and a dynamic economy system. Designed to be highly modular, it relies on Unreal's Subsystem architecture to keep your project clean while exposing everything seamlessly to both C++ and Blueprints.

## Key Features

* **Multiplayer Ready:** Fully replicated inventory and trading mechanics designed to stay synchronized across clients and the server.
* **Subsystem Architecture:** Utilizes Unreal Subsystems for global access, preventing logic bloat in your GameModes or PlayerControllers.
* **Advanced Tagging:** Built-in tag filtering to categorize, query, and retrieve specific items instantly.
* **Trading and Economy:** Features `MP_TradingManager` for secure player-to-player exchanges and `MP_EconomySystem` to dynamically adjust prices based on trade volume and demand.
* **Access Control:** Assign private or public visibility flags to control who can view specific inventory items.
* **Secure Metadata Tracking:** `MP_ItemMetadataStorage` tracks item trade history and pricing strictly server-side to prevent client tampering.
* **Data Persistence:** Subsystem-backed storage ensures items persist reliably.
* **Live UI Sync:** Event dispatchers automatically broadcast state changes for real-time UMG/Slate updates.
* **Dual API:** All inventory manipulation methods (add, remove, swap, replace, exchange) are fully exposed to both Blueprints and C++.

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
2. Configure `MP_ItemDefinitionStorage` with your custom item definitions and attributes.
3. Use `MP_TradeExchange` to handle manual or automatic listing of public items on the market.
4. Execute secure trades by calling `MP_TradingManager` functions via Blueprints or C++.
5. Track specific items across the server using the `MP_ItemTrackerSubsystem`.

## Contributing

Pull requests are always welcome. Whether it is a small bug fix, a performance tweak, or expanding the API, your contributions are appreciated. 

Here is the standard process to get your changes merged:

1. **Open an issue first:** For major features or architectural changes, please open an issue to discuss it before you start coding. This ensures we are on the same page and saves everyone time.
2. **Fork and branch:** Create a fork and do your work on a dedicated feature branch.
3. **Follow the style:** Keep your C++ and Blueprint code consistent with the existing project structure and formatting conventions.
4. **Submit a PR:** Keep your pull request focused on a single issue and explain exactly what your code does.

## License

This plugin is licensed under the GNU General Public License v3.0. See the [LICENSE](./LICENSE) file for full details.
