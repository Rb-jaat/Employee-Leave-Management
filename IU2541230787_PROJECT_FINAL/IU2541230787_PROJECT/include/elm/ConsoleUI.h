// ConsoleUI.h - menus and prompts only; every decision is delegated to LeaveSystem.
#pragma once
#include "LeaveSystem.h"

namespace elm {

class ConsoleUI {
    LeaveSystem& sys_;
    void banner() const;
    void mainMenu() const;
    // main menu entries
    void addMenu();      void displayMenu();   void searchMenu();
    void updateMenu();   void deleteMenu();    void processMenu();
    void reportMenu();   void fileMenu();      bool exitMenu();
    // helpers
    void applyForLeave();   void reviewQueue();   void cancelRequest();   void showBalance();
    std::string askType() const;
    void table(const std::string& header, const std::vector<std::string>& rows) const;
    template <class F> void guarded(F&& action);
public:
    explicit ConsoleUI(LeaveSystem& s) : sys_(s) {}
    int run();
};

}  // namespace elm
