#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

// Константы
const int WINDOW_WIDTH = 1520;
const int WINDOW_HEIGHT = 900;
const int TILE_SIZE = 80;
const float GRAVITY = 0.5f;
const float JUMP_SPEED = -14.0f;
const float MOVE_SPEED = 5.0f;
const int MAX_HEALTH = 100;
const float ANIMATION_SPEED = 0.2f;  // Скорость смены кадров (0.2 секунды на кадр)

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

    if (level.empty()) {
        std::cerr << "Ошибка: файл пуст!\n";
        exit(1);
    }
    return level;
}

// Функция для обработки столкновений игрока с платформами
void resolveCollision(sf::Sprite& player, sf::Vector2f& velocity, const sf::Sprite& platform, bool& isOnGround) {
    sf::FloatRect playerBounds = player.getGlobalBounds();
    sf::FloatRect platformBounds = platform.getGlobalBounds();

    if (!playerBounds.intersects(platformBounds)) return;

    float overlapLeft = playerBounds.left + playerBounds.width - platformBounds.left;
    float overlapRight = platformBounds.left + platformBounds.width - playerBounds.left;
    float overlapTop = playerBounds.top + playerBounds.height - platformBounds.top;
    float overlapBottom = platformBounds.top + platformBounds.height - playerBounds.top;

    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    if (minOverlapX < minOverlapY) {
        // Горизонтальное столкновение
        if (overlapLeft < overlapRight) {
            player.setPosition(platformBounds.left - playerBounds.width, player.getPosition().y);
        } else {
            player.setPosition(platformBounds.left + platformBounds.width, player.getPosition().y);
        }
        velocity.x = 0;
    } else {
        // Вертикальное столкновение
        if (overlapTop < overlapBottom) {
            player.setPosition(player.getPosition().x, platformBounds.top - playerBounds.height);
            velocity.y = 0;
            isOnGround = true;
        } else {
            player.setPosition(player.getPosition().x, platformBounds.top + platformBounds.height);
            velocity.y = 0;
        }
    }
}

// Функция для обновления анимации ходьбы
void updateAnimation(sf::Sprite& player, sf::Vector2f velocity, float& animationTimer, int& currentFrame, sf::Texture& playerTexture1, sf::Texture& playerTexture2, sf::Texture& playerTexture3) {
    if (velocity.x != 0) {
        animationTimer += 1.0f / 60.0f;  // увеличиваем таймер на каждом кадре

        if (animationTimer >= ANIMATION_SPEED) {
            animationTimer = 0.0f;  // сбросим таймер

            // Переключаем текстуру в зависимости от текущего кадра
            if (currentFrame == 0) {
                player.setTexture(playerTexture1);  // первый кадр
                currentFrame = 1;
            } else if (currentFrame == 1) {
                player.setTexture(playerTexture2);  // второй кадр
                currentFrame = 2;
            } else {
                player.setTexture(playerTexture3);  // третий кадр
                currentFrame = 0;
            }
        }
    } else {
        player.setTexture(playerTexture1);  // Если игрок не двигается, показываем первый кадр
    }
}

// Функция для обработки ввода
void handleInput(sf::Sprite& player, sf::Vector2f& velocity, bool& isOnGround) {
    velocity.x = 0.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        velocity.x = -MOVE_SPEED;
        player.setScale(-std::abs(player.getScale().x), player.getScale().y);  // инвертируем горизонтальный масштаб
        player.setOrigin(player.getLocalBounds().width, 0); // Устанавливаем новую точку отсчета
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        velocity.x = MOVE_SPEED;
        player.setScale(std::abs(player.getScale().x), player.getScale().y);  // нормальный масштаб
        player.setOrigin(0, 0); // Возвращаем точку отсчета к стандартной
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && isOnGround) {
        velocity.y = JUMP_SPEED;
        isOnGround = false;
    }
}

// Функция для применения гравитации
void applyGravity(sf::Vector2f& velocity, bool isOnGround) {
    if (!isOnGround) {
        velocity.y += GRAVITY;
    }
}

// Функция для обработки монет
void handleCoins(sf::Sprite& player, std::vector<sf::Sprite>& coins, int& score) {
    for (auto& coin : coins) {
        if (player.getGlobalBounds().intersects(coin.getGlobalBounds())) {
            score += 10;
            coin.setPosition(-TILE_SIZE, -TILE_SIZE); // Убираем монету
        }
    }
}

// Функция для отрисовки элементов
void render(sf::RenderWindow& window, const std::vector<sf::Sprite>& platforms, const std::vector<sf::Sprite>& coins, const sf::Sprite& player, const sf::Text& scoreText, const sf::Text& healthText) {
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

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D Платформер");
    window.setFramerateLimit(60);

    // Загрузка текстур для анимации
    sf::Texture playerTexture1, playerTexture2, playerTexture3, platformTexture, coinTexture;
    if (!playerTexture1.loadFromFile("1.png") ||
        !playerTexture2.loadFromFile("3.png") ||
        !playerTexture3.loadFromFile("5.png") ||
        !platformTexture.loadFromFile("platform.png") ||
        !coinTexture.loadFromFile("coin.png")) {
        std::cerr << "Ошибка загрузки текстур!\n";
        return 1;
    }

    // Игрок
    sf::Sprite player(playerTexture1);
    player.setScale(static_cast<float>(TILE_SIZE) / playerTexture1.getSize().x,
                    static_cast<float>(TILE_SIZE) / playerTexture1.getSize().y);
    sf::Vector2f velocity(0.0f, 0.0f);
    bool isOnGround = false;
    int health = MAX_HEALTH;

    // Загрузка уровня
    std::vector<std::string> level = loadLevelFromFile("level.txt");

    // Платформы и монеты
    std::vector<sf::Sprite> platforms;
    std::vector<sf::Sprite> coins;

    // Парсинг уровня
    for (size_t y = 0; y < level.size(); ++y) {
        for (size_t x = 0; x < level[y].size(); ++x) {
            if (level[y][x] == '#') {
                sf::Sprite platform(platformTexture);
                platform.setScale(static_cast<float>(TILE_SIZE) / platformTexture.getSize().x,
                                  static_cast<float>(TILE_SIZE) / platformTexture.getSize().y);
                platform.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                platforms.push_back(platform);
            } else if (level[y][x] == 'C') {
                sf::Sprite coin(coinTexture);
                coin.setScale(static_cast<float>(TILE_SIZE) / coinTexture.getSize().x,
                              static_cast<float>(TILE_SIZE) / coinTexture.getSize().y);
                coin.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                coins.push_back(coin);
            } else if (level[y][x] == 'P') {
                player.setPosition(x * TILE_SIZE, y * TILE_SIZE);
            }
        }
    }

    // Счет
    int score = 0;

    // Шрифт для текста
    sf::Font font;
    if (!font.loadFromFile("zeldadxt.ttf")) {
        std::cerr << "Ошибка: не удалось загрузить шрифт!\n";
        return 1;
    }

    // Текст для очков
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setPosition(20, 20);

    // Текст для здоровья
    sf::Text healthText;
    healthText.setFont(font);
    healthText.setCharacterSize(30);
    healthText.setFillColor(sf::Color::White);
    healthText.setStyle(sf::Text::Bold);
    healthText.setPosition(20, 60);

    // Таймер анимации и текущий кадр
    float animationTimer = 0.0f;
    int currentFrame = 0;

    // Основной цикл
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Обработка ввода
        handleInput(player, velocity, isOnGround);

        // Применение гравитации
        applyGravity(velocity, isOnGround);

        // Перемещение игрока
        player.move(velocity);

        // Обновление анимации
        updateAnimation(player, velocity, animationTimer, currentFrame, playerTexture1, playerTexture2, playerTexture3);

        // Обработка столкновений с платформами
        isOnGround = false;
        for (const auto& platform : platforms) {
            resolveCollision(player, velocity, platform, isOnGround);
        }

        // Обработка монет
        handleCoins(player, coins, score);

        // Обновление текста
        scoreText.setString("Score: " + std::to_string(score));
        healthText.setString("Health: " + std::to_string(health));

        // Отрисовка элементов
        render(window, platforms, coins, player, scoreText, healthText);
    }

    return 0;
}
