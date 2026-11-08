#define VERSION 3

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

#if VERSION==3
    #include <optional>
#endif

using namespace sf;
using namespace std;

//........................CHARACTER..........................................

class Character: public Drawable, public Transformable {
private:
    Texture textures[4];
    int current;
    Clock animationClock;
    Texture jumpTexture;

public:
    float speed=4.f;
    Sprite sprite;
    bool isJump=false;
    float direct=1.f;
    float dy=0.f;
    const float gravity=0.5f;

    bool isMovingLeft = false;
    bool isMovingRight = false;

    bool invuln=false;
    Clock invulnClock;
    Clock blinkClock;
    bool isBlink=false;

    Character(): sprite(textures[0]){
        textures[0].loadFromFile("sprites/characters/panda/run_1.png");
        textures[1].loadFromFile("sprites/characters/panda/run_2.png");
        textures[2].loadFromFile("sprites/characters/panda/run_3.png");
        textures[3].loadFromFile("sprites/characters/panda/run_4.png");
        jumpTexture.loadFromFile("sprites/characters/panda/jump_2.png");

        current=0;
        sprite.setTexture(textures[current], true);
        sprite.setOrigin({textures[current].getSize().x/2.f, textures[current].getSize().y/2.f});
        sprite.setScale({0.4f, 0.4f});
    }

    void animation() {
        if (animationClock.getElapsedTime().asSeconds()>=0.20f) {
            current=(current+1)%4;
            sprite.setTexture(textures[current], true);
            animationClock.restart();
        }
    }

    void jump() {
        if (!isJump) {
            isJump=true;
            dy=-14.f;
            sprite.setTexture(jumpTexture, true);
            sprite.setScale({direct*0.4f, 0.4f});
        }
    }

    void movement() {
        if (isMovingLeft) {
            move({-speed, 0.f});
            direct=-1.f;
            sprite.setScale({-0.4f, 0.4f});
            if (!isJump) animation();
        }
        if (isMovingRight) {
            move({speed, 0.f});
            direct=1.f;
            sprite.setScale({0.4f, 0.4f});
            if (!isJump) animation();
        }
        dy+=gravity;
        move({0.f, dy});

        if (invuln){
            if (invulnClock.getElapsedTime().asSeconds()<=1.0f){
                if (blinkClock.getElapsedTime().asSeconds()>=0.1f){
                    isBlink=!isBlink;
                    if (isBlink) sprite.setColor(Color::White);
                    else sprite.setColor(Color(255, 255, 255, 70));
                    blinkClock.restart();
                }
            }
            else{
                invuln=false;
                isBlink=false;
                sprite.setColor(Color::White);
            }
        }
    }

    void getDamage(){
        if (!invuln){
            invuln=true;
            invulnClock.restart();
            blinkClock.restart();
            setPosition({getPosition().x-(direct*50.f), getPosition().y});
        }
    }

    void landing() {
        isJump=false;
        dy=0.f;
        sprite.setTexture(textures[current], true);
        sprite.setScale({direct*0.4f, 0.4f});
    }

    FloatRect getBounds() { return getTransform().transformRect(sprite.getGlobalBounds()); }
    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(sprite, states);
    }
};

// .........................PLATFORMS.......................

class Platform: public Drawable, public Transformable {
protected:
    Sprite sprite;
    Texture large, medium, small;

public:
    Platform(): sprite(large) {
        large.loadFromFile("sprites/plat_big.png");
        medium.loadFromFile("sprites/plat_medium.png");
        small.loadFromFile("sprites/plat_small.png");
        sprite.setTexture(large, true);
        sprite.setScale({0.6f, 0.6f});
    }

    void medium_plat() { sprite.setTexture(medium, true); sprite.setScale({0.36f, 0.36f}); }
    void small_plat() { sprite.setTexture(small, true); sprite.setScale({0.47f, 0.47f}); }
    FloatRect getBounds() { return getTransform().transformRect(sprite.getGlobalBounds()); }
    virtual void movement(){}
    virtual Vector2f getSpeed(){ return {0.f, 0.f}; }
    virtual void onStepped() {}
    virtual bool isBroken() const { return false; }
    virtual void reset(){}

    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform(); target.draw(sprite, states);
    }
};

class MovingPlatform: public Platform{
private:
    float left, right, speed;
    int direction;
public:
    MovingPlatform(float startX, float startY, float l){
        setPosition({startX, startY});
        left=startX; right=startX+l; speed=2.f; direction=1;
    }
    Vector2f getSpeed() override{ return {speed*direction, 0.f}; }
    void movement() override{
        move({speed*direction, 0.f});
        if (getPosition().x>=right) direction=-1;
        if (getPosition().x<=left) direction=1;
    }
};

class FragilePlatform: public Platform {
private:
    Clock lifeTimer;
    bool broken = false, timerStarted = false;
    Vector2f basePos;
public:
    void setStartPos(Vector2f pos) { basePos = pos; setPosition(pos); }
    void reset() { broken = false; timerStarted = false; sprite.setColor(Color::White); setPosition(basePos); lifeTimer.restart(); }
    void onStepped() override {
        if (!timerStarted) { lifeTimer.restart(); timerStarted = true; basePos = getPosition(); }
    }
    void movement() override {
        if (timerStarted && !broken) {
            float time = lifeTimer.getElapsedTime().asSeconds();
            if (time > 2.0f) { broken = true; sprite.setColor(Color::Transparent); }
            else {
                setPosition({basePos.x + static_cast<float>(rand() % 5 - 2), basePos.y});
                int alpha = 255 - static_cast<int>((time / 2.0f) * 255.f);
                sprite.setColor(Color(255, 255, 255, (alpha < 0 ? 0 : alpha)));
            }
        }
    }
    bool isBroken() const override { return broken; }
};

//......................SPLIKES....................................

class Spike: public Drawable, public Transformable {
private:
    ConvexShape triangle;
public:
    Spike() {
        triangle.setPointCount(3);
        triangle.setPoint(0, {0.f, 0.f});
        triangle.setPoint(1, {15.f, -25.f});
        triangle.setPoint(2, {30.f, 0.f});
        triangle.setFillColor(Color(32, 58, 39));
    }
    FloatRect getBounds() const { return getTransform().transformRect(triangle.getGlobalBounds()); }
    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform(); target.draw(triangle, states);
    }
};

//.................HEALTHBAR............................

class Health: public Drawable, public Transformable{
private:
    Texture healthTex[4];
    Sprite sprite;
public:
    Health(): sprite(healthTex[0]){
        healthTex[0].loadFromFile("sprites/health/3.png");
        healthTex[1].loadFromFile("sprites/health/2.png");
        healthTex[2].loadFromFile("sprites/health/1.png");
        healthTex[3].loadFromFile("sprites/health/0.png");
        sprite.setTexture(healthTex[0], true);
        sprite.setPosition({20.f, 20.f});
        sprite.setScale({1.1f, 1.1f});
    }
    void updateHealth(int damageCount){
        if (damageCount>=0 && damageCount<4) sprite.setTexture(healthTex[damageCount], true);
    }
    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform(); target.draw(sprite, states);
    }
};

//..........................CHECKPOINT............................

class Checkpoint: public Drawable, public Transformable{
private:
    Texture textures[2];
    Sprite sprite;
    bool activated=false;
public:
#if VERSION==3
    Checkpoint(float x, float y): sprite(textures[0]){
#else
    Checkpoint(float x, float y){
#endif
        textures[0].loadFromFile("sprites/checkpoint_false.png");
        textures[1].loadFromFile("sprites/checkpoint_true.png");
        sprite.setTexture(textures[0], true);


        sprite.setTexture(textures[0], true);
        sprite.setScale({0.5f, 0.5f});
        setPosition({x, y});
    }


    bool activate(){
        if (!activated){
            activated=true;
            sprite.setTexture(textures[1], true);
            return true;
        }
        return false;
    }

    FloatRect getBounds(){
        return getTransform().transformRect(sprite.getGlobalBounds());}

    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform(); target.draw(sprite, states);
    }
};


//......................COIN...........................................

class Coin: public Drawable, public Transformable {
private:
    Sprite sprite;
    Texture textures[8];
    Clock animationClock;
    int current;

public:
    bool onScreen=true;

    Coin():sprite(textures[0]){
        textures[0].loadFromFile("sprites/Coins/coin_01.png");
        textures[1].loadFromFile("sprites/Coins/coin_02.png");
        textures[2].loadFromFile("sprites/Coins/coin_03.png");
        textures[3].loadFromFile("sprites/Coins/coin_04.png");
        textures[4].loadFromFile("sprites/Coins/coin_05.png");
        textures[5].loadFromFile("sprites/Coins/coin_06.png");
        textures[6].loadFromFile("sprites/Coins/coin_07.png");
        textures[7].loadFromFile("sprites/Coins/coin_08.png");
        current=0;
        sprite.setTexture(textures[0], true);

    }


    void animation(){
        if (animationClock.getElapsedTime().asSeconds()>=0.15f) {
            current=(current+1)%8;
            sprite.setTexture(textures[current], true);
            animationClock.restart();
        }
    }


    FloatRect getBounds() const {
        return getTransform().transformRect(sprite.getGlobalBounds());
    }


    void draw(RenderTarget& target, RenderStates states) const override {
        if (onScreen){
            states.transform *= getTransform();
            target.draw(sprite, states);
        }
    }
};



//..........................ENEMY....................................

class Enemy: public Drawable, public Transformable {
private:
    Texture textures[4];
    int current;
    Clock animationClock;
    float leftBound;
    float rightBound;
    float speed=1.5f;
    Sprite sprite;
    float direct=1.f;

public:
    Enemy(float start_x, float start_y, float distance): sprite(textures[0]){
        textures[0].loadFromFile("sprites/run1.png");
        textures[1].loadFromFile("sprites/run2.png");
        textures[2].loadFromFile("sprites/run3.png");
        textures[3].loadFromFile("sprites/run4.png");
        current=0;

        sprite.setTexture(textures[current], true);
        sprite.setOrigin({textures[current].getSize().x/2.f, textures[current].getSize().y/2.f});
        sprite.setScale({0.6f, 0.6f});

        setPosition({start_x, start_y});
        leftBound=start_x-distance/2.f;
        rightBound=start_x+distance/2.f;
    }


    void animation(){
        if (animationClock.getElapsedTime().asSeconds()>=0.20f){
            current=(current+1)%4;
            sprite.setTexture(textures[current], true);
            animationClock.restart();
        }
    }


    void movement(){
        move({speed*direct, 0.f});
        animation();
        if(getPosition().x>=rightBound && direct>0.f){
            direct=-1.f;
            sprite.setScale({-0.6f, 0.6f});
        }
        else if(getPosition().x<=leftBound && direct<0.f){
            direct=1.f;
            sprite.setScale({0.6f, 0.6f});
        }
    }


    FloatRect getBounds() const {
        return getTransform().transformRect(sprite.getGlobalBounds());
    }


    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(sprite, states);
    }
};




// ...................ÊÍÎÏÊÈ...........................
class Button : public Drawable, public Transformable {
protected:
    RectangleShape rect;
    Text text;
    function<void()> action;

public:
#if VERSION==3
    Button(float x, float y, float w, float h, const string& label, Font& font, function<void()> action1): text(font){
#else
    Button(float x, float y, float w, float h, const string& label, Font& font, function<void()> action1) {
        text.setFont(font);
#endif
        action=action1;

        rect.setSize({w, h});
        rect.setFillColor(Color(80, 140, 80));
        setPosition({x, y});

        text.setString(label);
        text.setCharacterSize(30);
        text.setFillColor(Color::White);

#if VERSION==3
        text.setOrigin({text.getLocalBounds().size.x / 2.f, text.getLocalBounds().size.y / 2.f});
#else
        FloatRect textRect=text.getLocalBounds();
        text.setOrigin({textRect.left + textRect.width / 2.f, textRect.top + textRect.height / 2.f});
#endif
        text.setPosition({w / 2.f, h / 2.f});
    }

    FloatRect getBounds(){
        return getTransform().transformRect(rect.getGlobalBounds());
    }

    void pressed(Vector2f mousePos){
        if(getBounds().contains(mousePos))
            action();
    }


    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(rect, states);
        target.draw(text, states);
    }
};
    
    class ImageButton : public Drawable, public Transformable {
    private:
        Texture normalTex;
        Texture hoverTex;
        Sprite sprite;
        function<void()> action;

    public:
        
#if VERSION==3
        ImageButton(const string& normalPath, float x, float y,float scale, function<void()> action1)
                : sprite(normalTex), action(action1)  {
#else
        ImageButton(const string& normalPath, float x, float y,float scale, function<void()> action1)
                : action(action1)  {
        sprite.setTexture(normalTex);
#endif

            normalTex.loadFromFile(normalPath);
            sprite.setTexture(normalTex, true);
            sprite.setOrigin({normalTex.getSize().x / 2.f,
                              normalTex.getSize().y / 2.f});
            sprite.setScale({scale, scale});
            setPosition({x, y});


        }


        FloatRect getBounds() {
            return getTransform().transformRect(sprite.getGlobalBounds());
        }

        void pressed(Vector2f mousePos) {
            if (getBounds().contains(mousePos)) action();
        }

        void draw(RenderTarget& target, RenderStates states) const override {
            states.transform *= getTransform();
            target.draw(sprite, states);
        }
    };
    
    int runMenu(RenderWindow& window, int unlockedLevel, int startPage = 0) {
        Texture menuBgTex;
        menuBgTex.loadFromFile("sprites/menu_background_4.jpg");
        Sprite menuBg(menuBgTex);
        
       // menuBg.setOrigin({menuBgTex.getSize().x / 2.f, menuBgTex.getSize().y / 2.f});
        menuBg.setPosition({80.f, 90.f}); // Ставим в самый центр экрана
        menuBg.setScale({0.95f,0.95f});
        
        Texture lockTex;
        lockTex.loadFromFile("sprites/lock.png");
        Sprite lockSprite(lockTex);
        lockSprite.setOrigin({lockTex.getSize().x / 2.f, lockTex.getSize().y / 2.f});
        lockSprite.setScale({0.25f, 0.25f});
       
        
        int page = startPage;
        int selectedLevel = -1;

        ImageButton startBtn("sprites/start_2.png", 630.f, 280.f, 0.25f, [&](){ page = 1; });
        ImageButton exitBtn("sprites/exit_2.png", 630.f, 450.f, 0.25f, [&](){ window.close(); });

        ImageButton levelsBtn("sprites/levels.png", 630.f, 190.f, 0.25f, [&](){ page = 2; });
        ImageButton shopBtn("sprites/shop.png", 630.f, 350.f, 0.25f, [&](){ page = 3; });
        ImageButton backBtn_play("sprites/back.png", 630.f, 540.f, 0.25f, [&](){ page = 0; });

        ImageButton level1Btn("sprites/level_1.png", 450.f, 320.f, 0.27f, [&](){ selectedLevel = 1; });
        ImageButton level2Btn("sprites/level_2.png", 820.f, 310.f, 0.29f, [&](){
            if (unlockedLevel >= 2) selectedLevel = 2;
        });
        ImageButton level3Btn("sprites/level_3.png", 740.f, 500.f, 0.25f, [&](){
            if (unlockedLevel >= 3) selectedLevel = 3;
        });
        ImageButton backBtn_levels("sprites/back.png", 630.f, 500.f, 0.25f, [&](){ page = 1; });

        ImageButton backBtn_shop("sprites/back.png", 630.f, 500.f, 0.25f, [&](){ page = 1; });

        

        while (window.isOpen() && selectedLevel == -1) {

    #if VERSION==3
            while (const std::optional event = window.pollEvent()) {
                if (event->is<Event::Closed>()) window.close();
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                      //  Vector2f mp = Vector2f(mouse->position);
                        Vector2f mp = window.mapPixelToCoords(mouse->position);
                        if (page == 0) { startBtn.pressed(mp); exitBtn.pressed(mp); }
                        else if (page == 1) { levelsBtn.pressed(mp); shopBtn.pressed(mp); backBtn_play.pressed(mp); }
                        else if (page == 2) { level1Btn.pressed(mp); level2Btn.pressed(mp); level3Btn.pressed(mp); backBtn_levels.pressed(mp); }
                        else if (page == 3) { backBtn_shop.pressed(mp); }
                    }
                }
            }
    #else
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) window.close();
                if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {
                        Vector2f mp = window.mapPixelToCoords(Mouse::getPosition(window));
                        if (page == 0) { startBtn.pressed(mp); exitBtn.pressed(mp); }
                        else if (page == 1) { levelsBtn.pressed(mp); shopBtn.pressed(mp); backBtn_play.pressed(mp); }
                        else if (page == 2) { level1Btn.pressed(mp); level2Btn.pressed(mp); level3Btn.pressed(mp); backBtn_levels.pressed(mp); }
                        else if (page == 3) { backBtn_shop.pressed(mp); }
                    }
                }
            }
    #endif

            window.clear(Color(205, 220, 210)); // Цвет под клетку тетради
            
            window.draw(menuBg);
           // window.draw(titleText);

            if (page == 0) {
                window.draw(startBtn);
                window.draw(exitBtn);
            } else if (page == 1) {
                window.draw(levelsBtn);
                window.draw(shopBtn);
                window.draw(backBtn_play);
            } else if (page == 2) {
                window.draw(level1Btn);
                window.draw(level2Btn);
                window.draw(level3Btn);
                
                    if (unlockedLevel < 2) {
                                    
                        lockSprite.setPosition({820.f, 310.f});
                        window.draw(lockSprite);
                }
                    if (unlockedLevel < 3) {
                                    
                        lockSprite.setPosition({750.f, 600.f});
                       // window.draw(lockSprite);
                    }

                
                window.draw(backBtn_levels);
                
                
            } else if (page == 3) {
               // window.draw(shopPanel);
                window.draw(backBtn_shop);
            }
            
            window.display();
        }

        return selectedLevel;
    }

//.........................LEVELS.......................................

class Level{
public:
    vector<Platform*> platforms;
    vector<Spike> spikes;
    vector<Checkpoint*> checkpoints;
    Vector2f startPos;
    RectangleShape levelEnd;
    vector<Coin*> coins;
    vector<Enemy*> enemies;

    Level(){}
    void create(int levelNumber){
        clearLevel();

        levelEnd.setSize({60.f, 100.f});
        levelEnd.setFillColor(Color(138, 43, 226));

        switch(levelNumber){
            case 1:{
                startPos={50.f, 660.f};
                levelEnd.setPosition({3800.f, 400.f});

                Platform* plat1=new Platform(); (*plat1).setPosition({0.f, 700.f}); (*plat1).medium_plat();
                Platform* plat2=new Platform(); (*plat2).setPosition({450.f, 560.f}); (*plat2).small_plat();
                Platform* plat3=new Platform(); (*plat3).setPosition({250.f, 420.f}); (*plat3).small_plat();
                Platform* plat4=new Platform(); (*plat4).setPosition({530.f, 265.f}); (*plat4).small_plat();
                MovingPlatform* plat5=new MovingPlatform(750.f, 265.f, 600.f); (*plat5).medium_plat();
                FragilePlatform* fragilePlat=new FragilePlatform();
                (*fragilePlat).setStartPos({1500.f, 500.f}); (*fragilePlat).medium_plat();
                Platform* plat6=new Platform(); (*plat6).setPosition({2000.f, 700.f}); (*plat6).medium_plat();
                MovingPlatform* plat7=new MovingPlatform(2330.f, 550.f, 250.f); (*plat7).small_plat();
                MovingPlatform* plat8=new MovingPlatform(2600.f, 400.f, 250.f); (*plat8).small_plat();
                Platform* plat9=new Platform(); (*plat9).setPosition({3000.f, 750.f});(*plat9).medium_plat();
                FragilePlatform* plat10=new FragilePlatform(); (*plat10).setStartPos({3100.f, 350.f}); (*plat10).small_plat();
                Platform* plat11=new Platform(); (*plat11).setPosition({3400.f, 500.f}); (*plat11).medium_plat();

                platforms={plat1, plat2, plat3, plat4, plat5, fragilePlat, plat6, plat7, plat8, plat9, plat10, plat11};

                Spike s1; s1.setPosition({2100.f, 700.f});
                Spike s2; s2.setPosition({2350.f, 700.f});
                Spike s3; s3.setPosition({3600.f, 500.f});
                Spike s4; s4.setPosition({3565.f, 500.f});
                spikes.push_back(s1); spikes.push_back(s2); spikes.push_back(s3); spikes.push_back(s4);

                checkpoints.push_back(new Checkpoint(610.f, 180.f));
                checkpoints.push_back(new Checkpoint(2210.f, 615.f));


                vector<Vector2f> coin_positions={ {510.f, 500.f},{920.f, 200.f}, {1200.f, 200.f}, {2600.f, 250.f}};
                for (int i=0; i<coin_positions.size(); i++) {
                    Coin* c=new Coin();
                    (*c).setPosition(coin_positions[i]);
                    coins.push_back(c);
                }


                break;
            }
            case 2:
                startPos={50.f, 660.f};
                Platform* plat1=new Platform(); (*plat1).setPosition({0.f, 700.f}); (*plat1).medium_plat();
                Platform* plat2=new Platform(); (*plat2).setPosition({450.f, 560.f}); (*plat2).small_plat();
                Platform* plat3=new Platform(); (*plat3).setPosition({250.f, 420.f}); (*plat3).small_plat();
                Platform* plat4=new Platform(); (*plat4).setPosition({530.f, 265.f}); (*plat4).small_plat();

                platforms={plat1, plat2, plat3, plat4};
                break;
        }
    }

    void clearLevel(){
        for (int i=0; i<platforms.size(); i++) delete platforms[i];
        platforms.clear();

        for (int i=0; i<checkpoints.size(); i++) delete checkpoints[i];
        checkpoints.clear();

        spikes.clear();

        for (int i=0; i<coins.size(); i++) delete coins[i];
        coins.clear();

        for (int i=0; i<enemies.size(); i++) delete enemies[i];
        enemies.clear();
    }
};

int main() {
    RenderWindow window(VideoMode({1500, 900}), "Platformer");
    window.setFramerateLimit(60);
    View view(FloatRect({0.f, 0.f}, {1500.f, 900.f}));
    
    
    int unlockedLevel = 1;
        int totalCoins = 0; // Монетки копятся глобально
        bool goToLevels = false;

        while (window.isOpen()) {
            // Передаем goToLevels ? 2 : 0. Если true, меню откроется на 2 странице (список уровней)
            int selectedLevel = runMenu(window, unlockedLevel, goToLevels ? 2 : 0);
            
            if (selectedLevel == -1) return 0;
            goToLevels = false;

    Font font;
#if VERSION==3
    font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
#else
    font.loadFromFile("pix2.ttf");
#endif

    RectangleShape backk({2850.f, 900.f});
    backk.setFillColor(Color(172, 231, 242));

    Texture backgroundTex;
    backgroundTex.loadFromFile("sprites/background.jpg");
    backgroundTex.setRepeated(true);
    Sprite background(backgroundTex);
    background.setTextureRect(sf::IntRect({0, 0}, {5000, 900}));
    background.setScale({1.5f, 1.5f});

    Level level;
    int levelNumb=selectedLevel;
    level.create(levelNumb);

    Character character;
    character.setPosition(level.startPos);
    Vector2f respawn=level.startPos;

    Health health;
    int damageCount=0;

    int countCoins=0;
    Texture moneyTex;
    moneyTex.loadFromFile("sprites/Coins/coin_01.png");
    Sprite moneySprite(moneyTex);
    moneySprite.setTexture(moneyTex, true);
    moneySprite.setPosition({20.f, 70.f});
    moneySprite.setScale({0.8f, 0.8f});


#if VERSION==3
    Text moneyText(font);
#else
    Text moneyText;
    moneyText.setFont(font);
#endif
    moneyText.setString("x 0");
    moneyText.setCharacterSize(35);
    moneyText.setFillColor(Color::White);
    moneyText.setPosition({70.f, 65.f});


    bool levelCompleted = false;

    RectangleShape winPanel({600.f, 400.f});
    winPanel.setFillColor(Color(200, 230, 200, 220));
    winPanel.setPosition({450.f, 250.f});


#if VERSION==3
    Text winText(font);
#else
    Text winText;
    winText.setFont(font);
#endif


    winText.setString("LEVEL COMPLETED!");

    winText.setCharacterSize(55);
    winText.setFillColor(Color(20, 40, 20));
    winText.setPosition({550.f, 270.f});


#if VERSION==3
    Text collectedText(font);
    Text totalText(font);
#else
    Text collectedText;
    Text totalText;
    collectedText.setFont(font);
    totalText.setFont(font);
#endif

    collectedText.setCharacterSize(30);
    collectedText.setFillColor(Color(40, 70, 40));
    collectedText.setPosition({480.f, 350.f});

    totalText.setCharacterSize(30);
    totalText.setFillColor(Color(40, 70, 40));
    totalText.setPosition({480.f, 385.f});

    bool returnToMenu = false;
    Button btnMenu(550.f, 440.f, 400.f, 50.f, "Main menu", font, [&](){
        totalCoins+=countCoins;
        returnToMenu = true;
});

    Button btnRetry(550.f, 510.f, 400.f, 50.f, "Retry level", font, [&](){ levelCompleted=false;
        level.create(levelNumb);
        character.setPosition(level.startPos);
        respawn=level.startPos;
        damageCount=0;
        health.updateHealth(0);
        moneyText.setString("x 0");
        totalCoins+=countCoins;
        countCoins=0;
    });


    Button btnNext(550.f, 580.f, 400.f, 50.f, "Next level", font, [&](){levelCompleted=false;
        
                if (levelNumb + 1 > unlockedLevel) {
                    unlockedLevel = levelNumb + 1;
                }

                levelCompleted=false;
                levelNumb++;
        
                level.create(levelNumb);
                character.setPosition(level.startPos);
                respawn=level.startPos;
                damageCount=0;
                health.updateHealth(0);
                moneyText.setString("x 0");
                totalCoins+=countCoins;
                countCoins=0;
        });



    while (window.isOpen() && !returnToMenu)
    {
#if VERSION==3
        while (const std::optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) window.close();

            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->code == Keyboard::Key::Space) character.jump();
                if (keyPressed->code == Keyboard::Key::A || keyPressed->code == Keyboard::Key::Left) character.isMovingLeft = true;
                if (keyPressed->code == Keyboard::Key::D || keyPressed->code == Keyboard::Key::Right) character.isMovingRight = true;
            }
            if (const auto* keyReleased = event->getIf<Event::KeyReleased>()) {
                if (keyReleased->code == Keyboard::Key::A || keyReleased->code == Keyboard::Key::Left) character.isMovingLeft = false;
                if (keyReleased->code == Keyboard::Key::D || keyReleased->code == Keyboard::Key::Right) character.isMovingRight = false;
            }

#else
        Event event;
        while(window.pollEvent(event)){
            if(event.type==Event::Closed)
                window.close();

            if (event.type==Event::KeyPressed){
                if (event.key.code==Keyboard::Space)
                    character.jump();
                if (event.key.code==Keyboard::A || event.key.code==Keyboard::Left)
                    character.isMovingLeft=true;
                if (event.key.code==Keyboard::D || event.key.code==Keyboard::Right)
                    character.isMovingRight=true;
            }
            if (event.type==Event::KeyReleased){
                if (event.key.code==Keyboard::A || event.key.code==Keyboard::Left)
                    character.isMovingLeft=false;
                if (event.key.code==Keyboard::D || event.key.code==Keyboard::Right)
                    character.isMovingRight=false;
            }

#endif


#if VERSION==3
            if (levelCompleted){
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()){
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2f mousePos = window.mapPixelToCoords(mouse->position);
                        btnMenu.pressed(mousePos);
                        btnRetry.pressed(mousePos);
                        btnNext.pressed(mousePos);
                    }
                }
            }

#else
            if (levelCompleted){
                if (event.type==Event::MouseButtonPressed){
                    if (event.mouseButton.button==Mouse::Left){
                        Vector2f mousePos=window.mapPixelToCoords(Mouse::getPosition(window));
                        btnMenu.pressed(mousePos);
                        btnRetry.pressed(mousePos);
                        btnNext.pressed(mousePos);
                    }
                }
            }
#endif

        }


        Vector2f prevPos=character.getPosition();
        character.movement();

        for (int i=0; i<level.platforms.size(); i++)
            (*level.platforms[i]).movement();

        if (character.getPosition().y>900.f) {
            damageCount++;
            health.updateHealth(damageCount);
            character.setPosition(respawn);
            character.direct=1.f;
            character.landing();

            for(int i=0; i<level.platforms.size(); i++)
                (*level.platforms[i]).reset();

            prevPos=character.getPosition();
            continue;
        }

        FloatRect characterBounds=character.getBounds();
         bool hitSpike = false;
        for (auto& spike : level.spikes) {
#if VERSION==3
            if (characterBounds.findIntersection(spike.getBounds())) { hitSpike = true; break; }
#else
            if (characterBounds.intersects(spike.getBounds())){
                    hitSpike = true;
                    break;}
#endif
        }


        if (hitSpike){
            if (!character.invuln){
                character.getDamage();
                damageCount++;
                health.updateHealth(damageCount);
            }
        }


        for (auto* checkpoint: level.checkpoints){
#if VERSION==3
            if (characterBounds.findIntersection(checkpoint->getBounds())){
#else
            if (characterBounds.intersects(checkpoint->getBounds())){
#endif
                if (checkpoint->activate()) {
                    respawn=checkpoint->getPosition();
                }
            }
        }


        for (int i=0; i<level.coins.size(); i++) {
            if (level.coins[i]->onScreen) {
                level.coins[i]->animation();
#if VERSION==3
                if (characterBounds.findIntersection(level.coins[i]->getBounds())){
#else
                if (characterBounds.intersects(level.coins[i]->getBounds())){
#endif
                    level.coins[i]->onScreen=false;
                    countCoins++;
                    moneyText.setString("x " + to_string(countCoins));
                }
            }
        }


            for (int i=0; i<level.enemies.size(); i++) {
#if VERSION==3
                if (characterBounds.findIntersection(level.enemies[i]->getBounds())){
#else
                    if (characterBounds.intersects(level.enemies[i]->getBounds())){
#endif
                        if (!character.invuln) {
                            character.getDamage();
                            damageCount++;
                            health.updateHealth(damageCount);
                        }
                    }
                }
                
                
#if VERSION==3
                if (characterBounds.findIntersection(level.levelEnd.getGlobalBounds()) && !levelCompleted) {
#else
                    if (characterBounds.intersects(level.levelEnd.getGlobalBounds()) && !levelCompleted){
#endif
                        levelCompleted=true;
                        
                        if (levelNumb + 1 > unlockedLevel) {
                                unlockedLevel = levelNumb + 1;
                        }
                        collectedText.setString("Collected coins: " + to_string(countCoins));
                        totalText.setString("Total coins: " + to_string(totalCoins+countCoins));
                    }
                    
                    
                    
                    bool onPlatform=false;
                    
                    for (int i=0; i<level.platforms.size(); i++) {
                        FloatRect platfBounds=(*level.platforms[i]).getBounds();
                        
#if VERSION==3
                        if (characterBounds.findIntersection(platfBounds)) {
                            if ((*level.platforms[i]).isBroken()) continue;
                            if (character.dy >= 0.f && (prevPos.y + characterBounds.size.y / 2.f) <= platfBounds.position.y + 5.f) {
                                float pos_y = platfBounds.position.y - (characterBounds.size.y / 2.f);
                                character.setPosition({character.getPosition().x, pos_y});
                                character.landing();
                                onPlatform = true;
                                (*level.platforms[i]).onStepped();
                                character.move((*level.platforms[i]).getSpeed());
                            }
                            else if (character.dy < 0.f && (prevPos.y - characterBounds.size.y / 2.f) >= (platfBounds.position.y + platfBounds.size.y - 5.f)) {
                                character.setPosition({character.getPosition().x, prevPos.y});
                                character.dy = 0.f;
                            }
                            else {
                                if (prevPos.x < platfBounds.position.x) character.setPosition({platfBounds.position.x - characterBounds.size.x / 2.f, character.getPosition().y});
                                else if (prevPos.x > platfBounds.position.x + platfBounds.size.x) character.setPosition({platfBounds.position.x + platfBounds.size.x + characterBounds.size.x / 2.f, character.getPosition().y});
                            }
                        }
#else
                        if (characterBounds.intersects(platfBounds)) {
                            if ((*level.platforms[i]).isBroken())
                                continue;
                            if (character.dy >= 0.f && (prevPos.y + characterBounds.height / 2.f) <= platfBounds.top + 5.f) {
                                float pos_y = platfBounds.top - (characterBounds.height / 2.f);
                                character.setPosition(character.getPosition().x, pos_y);
                                character.landing();
                                onPlatform = true;
                                (*level.platforms[i]).onStepped();
                                character.move((*level.platforms[i]).getSpeed());
                            }
                            else if (character.dy < 0.f && (prevPos.y - characterBounds.height / 2.f) >= (platfBounds.top + platfBounds.height - 5.f)) {
                                character.setPosition(character.getPosition().x, prevPos.y);
                                character.dy = 0.f;
                            }
                            else {
                                if (prevPos.x < platfBounds.left) character.setPosition(platfBounds.left - characterBounds.width / 2.f, character.getPosition().y);
                                else if (prevPos.x > platfBounds.left + platfBounds.width) character.setPosition(platfBounds.left + platfBounds.width + characterBounds.width / 2.f, character.getPosition().y);
                            }
                        }
#endif
                    }
                    
                    if (!onPlatform) character.isJump=true;
                    
                    float cameraX=character.getPosition().x +250.f;
                    if (cameraX<750.f) cameraX=750.f;
                    if (cameraX>3150.f) cameraX=3150.f;
                    view.setCenter({cameraX, 450.f});
                    
                    window.clear();
                    window.setView(view);
                    
                    window.draw(backk);
                    window.draw(background);
                    
                    for (int i=0; i<level.platforms.size(); i++) window.draw(*level.platforms[i]);
                    for (auto& spike : level.spikes) window.draw(spike);
                    
                    for (auto* i: level.checkpoints){
                        window.draw(*i);
                        
                    }
                    
                    for (int i=0; i<level.coins.size(); i++) {
                        window.draw(*level.coins[i]);
                    }
                    
                    for (int i=0; i<level.enemies.size(); i++) {
                        window.draw(*level.enemies[i]);
                    }
                    
                    
                    window.draw(character);
                    window.setView(window.getDefaultView());
                    window.draw(health);
                    window.draw(moneySprite);
                    window.draw(moneyText);
                    
                    if (levelCompleted) {
                        window.draw(winPanel);
                        window.draw(winText);
                        window.draw(btnMenu);
                        window.draw(btnRetry);
                        window.draw(btnNext);
                        window.draw(collectedText);
                        window.draw(totalText);
                    }
                    
                    window.draw(level.levelEnd);
                    
                    window.display();
                }
                
                // Если игрок нажал "Main menu", при следующем запуске откроется список уровней
                if (returnToMenu) {
                    goToLevels = true;
                }
                
                level.clearLevel();
            }

    return 0;
}
