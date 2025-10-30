#include <iostream>
#include <windows.h>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

struct AttackResult {
    int damage;
    int blocked;
};

// ===================== Класс персонажа =====================
class Character {
protected:
    string name;
    int hp;
    int maxHp;
    int attack;
    bool isDefending;

public:
    Character(string n, int h, int a)
        : name(n), hp(h), maxHp(h), attack(a), isDefending(false) {}

    virtual ~Character() = default;

    [[nodiscard]] string getName() const { return name; }
    [[nodiscard]] int getHP() const { return hp; }
    [[nodiscard]] int getMaxHP() const { return maxHp; }
    [[nodiscard]] bool isAlive() const { return hp > 0; }

    AttackResult attackTarget(Character &target) const {
        int damage = attack + rand() % 6;
        int blocked = 0;

        if (target.isDefending) {
            blocked = target.maxHp / 10 + rand() % 10;
            damage -= blocked;
        }

        if (damage < 0) damage = 0;
        target.hp -= damage;
        return {damage, blocked};
    }

    int heal() {
        int healAmount = maxHp / 5 + rand() % 10;
        healAmount = min(healAmount, maxHp - hp);
        hp += healAmount;
        return healAmount;
    }

    void defend() {
        isDefending = true;
    }

    void resetDefense() {
        isDefending = false;
    }

    void setHp(int newHp) {
        if (newHp < 0) hp = 0;
        else hp = newHp;
    }
};

// ===================== Игрок =====================
class Player final : public Character {
private:
    static string generateName() {
        vector<string> names = {
            "Инокентий", "Ибрагимус", "Мессенджер Макс", "Серафиночка", "Дрон SSR-ранга",
            "Юнмины", "Ватафакер", "Хотьбыненикита", "Нефоренок", "Таймбомб"
        };
        return names[rand() % names.size()];
    }
    int potions;

public:
    Player() : Character(generateName(), 100, 15), potions(3) {}

    [[nodiscard]] int getPotions() const { return maxHp; }

    void addPotions(int n = 1) { potions += n; }

    void printStatus() const {
        cout << "[Игрок] " << name
             << " | HP: " << hp << "/" << maxHp
             << " | Зелья: " << potions
             << "\n";
    }

    static int chooseAction() {
        int choice;
        cout << "\nВыбери действие:\n";
        cout << "1. Атаковать\n";
        cout << "2. Защищаться\n";
        cout << "3. Выпить зелье\n";
        cout << "> ";

        while (!(cin >> choice) || (choice < 1 || choice > 3) && choice != 131313) {
            cout << "Неверный ввод. Попробуйте снова: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return choice;
    }

    void summonAttack(Character &target) {
        target.setHp(0);
    }

    int drinkPotion() {
        if (potions > 0) {
            if (hp == maxHp) return 0;
            int healAmount = maxHp / 3 + rand() % 5;
            healAmount = min(healAmount, maxHp - hp);
            hp += healAmount;
            potions--;
            return healAmount;
        } else {
            return -1;
        }
    }
};


// ===================== Враг =====================
class Enemy final : public Character {
public:
    Enemy(string n, int h, int a)
        : Character(n, h, a) {}

    void printStatus() const {
        cout << "[Враг] " << name
             << " | HP: " << hp << "/" << maxHp
             << "\n";
    }

    int chooseAction() const {
        if (hp < maxHp / 2 && rand() % 3 == 0) {
            return 3; // лечится с небольшой вероятностью
        }
        if (rand() % 100 < 70) return 1; // чаще атакует
        return 2; // иногда защищается
    }
};

// ===================== Бой =====================
class Battle {
private:
    Player &player;
    Enemy &enemy;

public:
    Battle(Player &p, Enemy &e) : player(p), enemy(e) {}

    void start() {
        cout << "\n===== БОЙ НАЧИНАЕТСЯ =====\n";
        while (player.isAlive() && enemy.isAlive()) {
            cout << "-------------------------------\n";
            player.printStatus();
            enemy.printStatus();
            cout << "-------------------------------\n";

            // Ход игрока
            player.resetDefense();
            switch (player.chooseAction()) {
                case 1: {
                    AttackResult result = player.attackTarget(enemy);
                    cout << "Вы ударили " << enemy.getName() << " на " << result.damage << " урона!\n";
                    if (result.blocked > 0) {
                        cout << "Ещё " << result.blocked << " урона было заблокировано!\n";
                    }
                    break;
                }
                case 2:
                    player.defend();
                    cout << "Вы готовитесь к защите!\n";
                    break;
                case 3: {
                    const int healed = player.drinkPotion();
                    if (healed == -1) {
                        cout << "Вы хотели выпить зелье, но оно закончилось!\n";
                    }
                    else if (healed == 0) {
                        cout << "Вы полностью здоровы. Зелье не было использовано!\n";
                    }
                    else {
                        cout << "Вы выпили зелье и восстановили " << healed << " HP!\n";
                    }
                    break;
                }
                case 131313:
                    cout << "Вы разбудили Юну. " << enemy.getName() <<
                        " пытался бежать в страхе, но был укушен за жэпу - какая нелепая смерть...\n";
                    player.summonAttack(enemy);
                    break;
                default:
                    break;
            }

            cout << '\n';

            if (!enemy.isAlive()) {
                cout << "Вы победили врага!\n";
                player.resetDefense();
                break;
            }

            // Ход врага
            enemy.resetDefense();
            switch (enemy.chooseAction()) {
                case 1: {
                    AttackResult result = enemy.attackTarget(player);
                    cout << enemy.getName() << " атакует!\n";
                    if (result.blocked > 0) {
                        cout << "Вы блокируете " << result.blocked << " урона!\n";
                    }
                    cout << "Получено " << result.damage << " урона!\n";
                    break;
                }
                case 2:
                    enemy.defend();
                    cout << enemy.getName() << " готовится к защите!\n";
                    break;
                case 3:
                    cout << enemy.getName() << " восстановил " << enemy.heal() << " HP!\n";
                    break;
                default:
                    break;
            }

            cout << '\n';

            if (!player.isAlive()) {
                cout << "Вы пали в бою...\n";
                break;
            }
        }
    }
};

// ===================== Основная программа =====================
int main() {
    SetConsoleOutputCP(CP_UTF8);
    srand(time(nullptr));

    Player player;
    const string playerName = player.getName();

    cout << "\nТеперь тебя зовут " << player.getName() << "! Сочувствую...\n";

    vector<Enemy> enemies = {
        Enemy("Гоблин", 50, 10),
        Enemy("Орк", 80, 12),
        Enemy("Дракон", 150, 20)
    };

    cout << "А теперь приготовься к сражениям!\n";

    for (auto &enemy : enemies) {
        cout << "\nНа твоём пути встречается " << enemy.getName() << "!\n";
        Battle battle(player, enemy);
        battle.start();

        if (player.isAlive()) {
            cout << "\n" << playerName << " победил врага во славу ДГТУ!\n";
            cout << playerName << " сделал привал после тяжелой битвы и восстановил " << player.heal() << " HP!\n";
        } else {
            cout << "\nИгра окончена. " << playerName << " погиб.\n";
            return 0;
        }
    }

    cout << "\nПоздравляем, " << playerName << "! Ты победил всех врагов и стал хокагэ в Деревне Скрытого Т-университета!\n";
    return 0;
}