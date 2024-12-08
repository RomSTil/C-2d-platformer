#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

// Константы
const int WINDOW_WIDTH = 1520;
const int WINDOW_HEIGHT = 900;
const int TILE_SIZE = 80;  // Размер тайла (платформы)
const float GRAVITY = 0.5f;
const float JUMP_SPEED = -10.0f;
const float MOVE_SPEED = 5.0f;
const int MAX_HEALTH = 100;  // Максимальное здоровье игрока

// Функция загрузки уровня
std::vector<std::string> loadLevelFromFile(const std::string& filename) {
    std::vector<std::string> level;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Ошибка: файл не открывается!\n";
        exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        level.push_back(line);
    }

    file.close();

    if (level.empty()) {
        std::cerr << "Ошибка: файл пуст!\n";
        exit(1);
    }

    return level;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D Платформер");
    window.setFramerateLimit(60);

    // Загрузка текстур
    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile("player.png")) {
        std::cerr << "Ошибка: не удалось загрузить текстуру для игрока!\n";
        return 1;
    }

    sf::Texture platformTexture;
    if (!platformTexture.loadFromFile("platform.png")) {
        std::cerr << "Ошибка: не удалось загрузить текстуру для платформы!\n";
        return 1;
    }

    sf::Texture coinTexture;
    if (!coinTexture.loadFromFile("coin.png")) {
        std::cerr << "Ошибка: не удалось загрузить текстуру для монеток!\n";
        return 1;
    }

    // Вектор для хранения монеток
    std::vector<sf::Sprite> coins;

    // Загрузка уровня
    std::vector<std::string> level = loadLevelFromFile("level.txt");

    // Проверка уровня
    size_t rowLength = level[0].size();
    for (const auto& row : level) {
        if (row.size() != rowLength) {
            std::cerr << "Ошибка: строки уровня имеют разную длину!\n";
            return 1;
        }
    }

    // Игрок
    sf::Sprite player;
    player.setTexture(playerTexture);
    player.setScale(
        static_cast<float>(TILE_SIZE) / playerTexture.getSize().x,
        static_cast<float>(TILE_SIZE) / playerTexture.getSize().y); // Пропорциональный масштаб

    // Установка начальной позиции игрока
    sf::Vector2f playerStartPos;
    bool playerPositionSet = false;

    // Платформы
    std::vector<sf::Sprite> platforms;

    // Обработка уровня
    for (size_t y = 0; y < level.size(); ++y) {
        for (size_t x = 0; x < level[y].size(); ++x) {
            if (level[y][x] == '#') {
                sf::Sprite platform;
                platform.setTexture(platformTexture);
                platform.setScale(
                    static_cast<float>(TILE_SIZE) / platformTexture.getSize().x,
                    static_cast<float>(TILE_SIZE) / platformTexture.getSize().y); // Пропорциональный масштаб
                platform.setPosition(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE); // Позиция с учетом TILE_SIZE
                platforms.push_back(platform);
            } else if (level[y][x] == 'P' && !playerPositionSet) {
                player.setPosition(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE);
                playerStartPos = {static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE};
                playerPositionSet = true;
            } else if (level[y][x] == 'C') {
                sf::Sprite coin;
                coin.setTexture(coinTexture);
                float scale = static_cast<float>(TILE_SIZE) / std::max(coinTexture.getSize().x, coinTexture.getSize().y);
                coin.setScale(scale, scale);  // Устанавливаем нормальный масштаб
                coin.setPosition(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE); // Позиция с учетом TILE_SIZE
                coins.push_back(coin);
                // Вывод координат монеты для отладки
                std::cout << "Coin at: " << coin.getPosition().x << ", " << coin.getPosition().y << std::endl;
            }
        }
    }

    if (!playerPositionSet) {
        std::cerr << "Ошибка: в уровне не найден символ 'P' для позиции игрока!\n";
        return 1;
    }

    // Создание текста
    sf::Font font;
    if (!font.loadFromFile("zeldadxt.ttf")) {
        std::cerr << "Ошибка: не удалось загрузить шрифт!\n";
        return 1;
    }

    // Текст для отображения информации
    sf::Text playerText;
    playerText.setFont(font);
    playerText.setString("Player");
    playerText.setCharacterSize(20);
    playerText.setFillColor(sf::Color::White);
    playerText.setStyle(sf::Text::Bold);

    // Счетчик очков
    int score = 0;
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setPosition(20, 20);

    // Игрок
    int health = MAX_HEALTH;  // Начальное здоровье игрока
    sf::Text healthText;
    healthText.setFont(font);
    healthText.setString("Health: " + std::to_string(health));
    healthText.setCharacterSize(30);
    healthText.setFillColor(sf::Color::White);
    healthText.setStyle(sf::Text::Bold);
    healthText.setPosition(20, 60);

    // Переменные игрока
    sf::Vector2f velocity(0.0f, 0.0f);
    bool isOnGround = false;

    while (window.isOpen()) {
        // События
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Управление
        velocity.x = 0.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            velocity.x = -MOVE_SPEED;

            // Отражаем спрайт влево
            if (player.getScale().x > 0) {
                player.setScale(-std::abs(player.getScale().x), player.getScale().y);
                player.setOrigin(player.getLocalBounds().width, 0); // Устанавливаем новую точку отсчета
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            velocity.x = MOVE_SPEED;

            // Возвращаем спрайт в исходное положение
            if (player.getScale().x < 0) {
                player.setScale(std::abs(player.getScale().x), player.getScale().y);
                player.setOrigin(0, 0); // Возвращаем точку отсчета к стандартной
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && isOnGround) {
            velocity.y = JUMP_SPEED;
            isOnGround = false;
        }

        // Перемещение игрока
        player.move(velocity);

        // Ограничения экрана
        if (player.getPosition().x < 0) player.setPosition(0, player.getPosition().y);
        if (player.getPosition().x > WINDOW_WIDTH) player.setPosition(WINDOW_WIDTH, player.getPosition().y);

        // Обработка вертикального движения
        player.move(0.0f, velocity.y);
        isOnGround = false;

        // Проходим по платформам для проверки коллизий
        for (const auto& platform : platforms) {
            if (player.getGlobalBounds().intersects(platform.getGlobalBounds())) {
                
                // Коллизия с платформой по вертикали
                if (velocity.y > 0) { // Падение
                    float playerBottom = player.getPosition().y + player.getGlobalBounds().height;
                    float platformTop = platform.getPosition().y;

                    // Проверяем, находится ли игрок над платформой
                    if (playerBottom > platformTop) {
                        player.setPosition(player.getPosition().x, platformTop - player.getGlobalBounds().height);
                        velocity.y = 0;
                        isOnGround = true;
                    }
                } else if (velocity.y < 0) { // Прыжок вверх
                    float playerTop = player.getPosition().y;
                    float platformBottom = platform.getPosition().y + platform.getGlobalBounds().height;

                    // Проверяем, находится ли игрок под платформой
                    if (playerTop < platformBottom) {
                        player.setPosition(player.getPosition().x, platformBottom);
                        velocity.y = 0;
                    }
                }
            }
        }

        // Если игрок не на земле, применяем гравитацию
        if (!isOnGround) {
            velocity.y += GRAVITY; // gravity - величина гравитации
        }

        // Проверка столкновений с монетами
        for (auto& coin : coins) {
            if (player.getGlobalBounds().intersects(coin.getGlobalBounds())) {
                score += 10;
                coin.setPosition(-TILE_SIZE, -TILE_SIZE); // Убираем монету после сбора
            }
        }

        // Отображение информации
        scoreText.setString("Score: " + std::to_string(score));
        healthText.setString("Health: " + std::to_string(health));

        // Отображение монет и объектов
        window.clear();
        for (const auto& platform : platforms) {
            window.draw(platform);
        }
        for (const auto& coin : coins) {
            window.draw(coin);
        }
        window.draw(player);
        window.draw(scoreText);
        window.draw(healthText);
        window.display();
    }

    return 0;
}
