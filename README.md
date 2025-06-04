---

# MP_Inventory - Multiplayer Inventory Plugin for Unreal Engine

  MP_Inventory is a powerful multiplayer-ready inventory system for Unreal Engine, built with full replication support. It allows storing, managing, and sharing items with a robust tag system, private/public access control, and persistent item data. Designed to simplify multiplayer item management, it's highly modular, extendable, and open-source.

---

## 🚀 Features

- ✅ **Multiplayer and Replication Ready**: Fully replicated inventory system designed for multiplayer games.
- 📁 **Persistent Storage**: Items persist in subsystem-backed storage for seamless inventory management.
- 🏷️ **Advanced Tag System**: Built-in tag-based filtering to manage and retrieve specific item types.
- 🔒 **Private and Public Item Support**: Control item visibility with private and public inventory settings.
- 🔄 Item Exchange & Trading – Secure player-to-player and player-to-marketplace trading.
- 💰 Dynamic Economy System – Item prices adjust based on trade volume & demand.
- 📊 Historical Metadata Storage – Tracks item trade history and price fluctuations.
- ⚙️ **Subsystem-Based Architecture**: Clean, modular approach using Unreal's subsystem for flexibility and scalability.
- 🛠 **Blueprint & C++ API Support**: Exposes inventory and trading functions to Blueprints.
- 🔁 **Real-Time UI Sync**: Inventory changes trigger event dispatchers for live UI updates.

---

## 🧩 Plugin Code Features

- **Subsystem-based inventory management** for global access and modular design.
- **Built-in tag system** to categorize and filter inventory items.
- **Replicated item storage** to ensure all clients and servers remain synchronized.
- **Event dispatchers** for item update notifications, allowing easy UI and logic binding.
- **Inventory manipulation methods**: Add, remove, swap, replace, and exchange items.
- **Access control** via private/public flags for inventory visibility.
- **MP_ItemDefinitionStorage** Stores item definitions and custom attributes.
- **MP_ItemMetadataStorage** Tracks item trade history, pricing, and metadata. Server-side only to prevent tampering.
- **Trading Manager** is Responsible For Trade Item through Player or non Player.
- **MP_TradeExchange** Handles manual & auto-listing of public items on the trade system.
- **MP_ItemTrackerSubsystem** Maps ItemID → PlayerID for item searching and trading.
- **MP_EconomySystem** Adjusts item prices dynamically based on trading trends.

---

## 🛫 Getting Started
# 📥 Installation
1. **Download or Clone** this repository:
   ```bash
   git clone https://github.com/Yuvi-GD/MP_Inventory.git
   ```

2. **Add Plugin to Your Unreal Engine Project**:
   - Copy the `MP_Inventory` folder to your project's `Plugins` directory.
   - Or install as a marketplace plugin if available (future support).

3. **Enable Plugin**:
   - Open your project in Unreal Engine.
   - Go to `Edit` ➡ `Plugins` ➡ Search for **MP_Inventory** and enable it.
   - Restart the engine if prompted.

# 🛠 Usage

1️⃣ Attach **MP_InventoryComponent** to Player Characters.

2️⃣ Set up **MP_ItemDefinitionStorage** to define items.

3️⃣ Use **MP_TradingManager** for secure trades.

4️⃣ Utilize **MP_TradeExchange** for listing items.

5️⃣ Access inventory and trading functions in Blueprints.

---

## 🤝 Contributions

We warmly welcome contributions from the community!  
If you want to add features, fix bugs, or improve documentation, feel free to fork this repository and submit a pull request.  
**Your support helps keep this project alive and evolving!**

Also, feel free to open **Issues** for suggestions, questions, or bugs.

---

## 📜 License

This plugin is licensed under the **GNU General Public License v3.0**.  
You are free to use, modify, and distribute this plugin under the terms of the GNU GPL v3.0.  
Please refer to the [LICENSE](./LICENSE) file for more details.

---

## 💬 Feedback & Suggestions

Would you like to add any additional sections or details to the README file?  
Feel free to open an **[Issue](https://github.com/Yuvi-GD/MP_Inventory/issues)** to share your thoughts!

---
