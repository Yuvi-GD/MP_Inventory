---

## MP_Inventory - Multiplayer Inventory Plugin for Unreal Engine

**MP_Inventory** is a powerful multiplayer-ready inventory system for Unreal Engine, built with full replication support. It allows storing, managing, and sharing items with a robust tag system, private/public access control, and persistent item data. Designed to simplify multiplayer item management, it's highly modular, extendable, and open-source.

---

## 🚀 Features

- ✅ **Multiplayer and Replication Ready**: Fully replicated inventory system designed for multiplayer games.
- 📁 **Persistent Storage**: Items persist in subsystem-backed storage for seamless inventory management.
- 🏷️ **Advanced Tag System**: Built-in tag-based filtering to manage and retrieve specific item types.
- 🔒 **Private and Public Item Support**: Control item visibility with private and public inventory settings.
- 🔄 **Item Exchange & Swap**: Exchange, swap, or replace items directly between player inventories.
- 🛠️ **Dynamic Updates and Events**: Real-time update of item data and tags, with dispatchers for custom logic binding.
- ⚙️ **Subsystem-Based Architecture**: Clean, modular approach using Unreal's subsystem for flexibility and scalability.

---

## 🧩 Plugin Code Features

- **Subsystem-based inventory management** for global access and modular design.
- **Built-in tag system** to categorize and filter inventory items.
- **Replicated item storage** to ensure all clients and servers remain synchronized.
- **Event dispatchers** for item update notifications, allowing easy UI and logic binding.
- **Inventory manipulation methods**: Add, remove, swap, replace, and exchange items.
- **Access control** via private/public flags for inventory visibility.

---

## 🛫 Startup Guide

1. **Download or Clone** this repository:
   ```bash
   git clone https://github.com/YourGitHubName/MP_Inventory.git
   ```

2. **Add Plugin to Your Unreal Engine Project**:
   - Copy the `MP_Inventory` folder to your project's `Plugins` directory.
   - Or install as a marketplace plugin if available (future support).

3. **Enable Plugin**:
   - Open your project in Unreal Engine.
   - Go to `Edit` ➡ `Plugins` ➡ Search for **MP_Inventory** and enable it.
   - Restart the engine if prompted.

4. **Start Using MP_Inventory**:
   - Access the Inventory Subsystem to start adding and managing items.
   - Bind event dispatchers to your UI or gameplay logic to reflect inventory changes.
   - Use provided APIs for item exchange, filtering, and tag-based operations.

5. **Check Samples**: Example content and blueprint usage available in `Examples` folder (if provided).

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
