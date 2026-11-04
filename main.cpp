#define VERSION 2

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

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

    bool invuln=false;
    Clock invulnClock;
    Clock blinkClock;
    bool isBlink=false;

    Character(): sprite(textures[0]){
        /*textures[0].loadFromFile("sprites/characters/dog/run_1.png");
        textures[1].loadFromFile("sprites/characters/dog/run_4.png");
        textures[2].loadFromFile("sprites/characters/dog/run_3.png");
        textures[3].loadFromFile("sprites/characters/dog/run_22.png");
        jumpTexture.loadFromFile("sprites/characters/dog/jump.png");*/


        textures[0].loadFromFile("sprites/characters/panda/run_1.png");
        textures[1].loadFromFile("sprites/characters/panda/run_2.png");
        textures[2].loadFromFile("sprites/characters/panda/run_3.png");
        textures[3].loadFromFile("sprites/characters/panda/run_4.png");
        jumpTexture.loadFromFile("sprites/characters/panda/jump_2.png");

        current=0;
        sprite.setTexture(textures[current], true);
        sprite.setOrigin(textures[current].getSize().x/2.f, textures[current].getSize().y/2.f);
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
        if (Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left)) {
            move({-speed, 0.f});
            direct=-1.f;
            sprite.setScale({-0.4f, 0.4f});
            if (!isJump) animation();
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right)) {
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
                    if (isBlink){
                        sprite.setColor(Color::White);
                    }
                    else{
                        sprite.setColor(Color(255, 255, 255, 70));
                    }

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

    FloatRect getBounds() {
        return getTransform().transformRect(sprite.getGlobalBounds());
    }

    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(sprite, states);
    }
};



// .........................PLATFORMS.......................

class Platform: public Drawable, public Transformable {
protected:
    Sprite sprite;
    Texture large;
    Texture medium;
    Texture small;

public:
    Platform(): sprite(large) {
        large.loadFromFile("sprites/plat_big.png");
        medium.loadFromFile("sprites/plat_medium.png");
        small.loadFromFile("sprites/plat_small.png");
        sprite.setTexture(large, true);
        sprite.setScale({0.6f, 0.6f});
    }

    void medium_plat() {
        sprite.setTexture(medium, true);
        sprite.setScale({0.36f, 0.36f});
    }

    void small_plat() {
        sprite.setTexture(small, true);
        sprite.setScale({0.47f, 0.47f});
    }

    FloatRect getBounds(){
        return getTransform().transformRect(sprite.getGlobalBounds());
    }



    virtual void movement(){}
    virtual Vector2f getSpeed(){
        return {0.f, 0.f};}

    virtual void onStepped() {}
    virtual bool isBroken() const { return false; }
    virtual void reset(){}



    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(sprite, states);
    }
};


class MovingPlatform: public Platform{
private:
    float left;
    float right;
    float speed;
    int direction;

public:
    MovingPlatform(float startX, float startY, float l){

        setPosition({startX, startY});
        left=startX;
        right=startX+l;
        speed=2.f;
        direction=1;
    }

    Vector2f getSpeed() override{
        return {speed*direction, 0.f};
    }

    void movement() override{
        move({speed*direction, 0.f});

        if (getPosition().x>=right)
            direction=-1;
        if (getPosition().x<=left)
            direction=1;
    }
};


class FragilePlatform: public Platform {
private:
    Clock lifeTimer;
    bool broken = false, timerStarted = false;
    Vector2f basePos;

public:
    void setStartPos(Vector2f pos) {
        basePos = pos;
        setPosition(pos); }

    void reset() {
        broken = false;
        timerStarted = false;
        sprite.setColor(Color::White);
        setPosition(basePos);
        lifeTimer.restart(); }


    void onStepped() override {
        if (!timerStarted) {
                lifeTimer.restart();
                timerStarted = true;
                basePos = getPosition(); }
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
        triangle.setFillColor(Color(200, 0, 0));
    }
    FloatRect getBounds() const {
        return getTransform().transformRect(triangle.getGlobalBounds()); }

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
        if (damageCount>=0 && damageCount<4){
            sprite.setTexture(healthTex[damageCount], true);
        }
    }

    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(sprite, states);
    }
};


//..........................CHECKPOINT............................

class Checkpoint: public Drawable, public Transformable{
private:
    Texture active;
    Texture unactive;
    Sprite sprite;
    bool activated=false;

public:
#if VERSION==3
    Checkpoint(float x, float y): sprite(unactive){
#else
    Checkpoint(float x, float y){
#endif

        unactive.loadFromFile("sprites/checkpoint_false.png");
        active.loadFromFile("sprites/checkpoint_true.png");
        sprite.setTexture(unactive, true);
        sprite.setScale({0.5f, 0.5f});
        setPosition({x, y});
    }

    bool activate(){
        if (!activated){
            activated=true;
            sprite.setTexture(active, true);
            return true;
        }
        return false;
    }

    FloatRect getBounds() {
        return getTransform().transformRect(sprite.getGlobalBounds());
    }

    void draw(RenderTarget& target, RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(sprite, states);
    }
};


//.........................LEVELS.......................................

class Level{
public:
    vector<Platform*> platforms;
    vector<Spike> spikes;
    vector<Checkpoint> checkpoints;
    //ïîòîì åùå øòóêè áóäóò, êîãäà äîáàâèì èõ
    //ôîí, êñòàòè, òîæå òóò íàäî
    Vector2f startPos;

    Level(){}
    void create(int levelNumber){
        clearLevel();

        switch(levelNumber){
            case 1:{
                startPos={50.f, 660.f};

                Platform* plat1=new Platform(); (*plat1).setPosition({0.f, 700.f}); (*plat1).medium_plat();
                Platform* plat2=new Platform(); (*plat2).setPosition({450.f, 560.f}); (*plat2).small_plat();
                Platform* plat3=new Platform(); (*plat3).setPosition({250.f, 420.f}); (*plat3).small_plat();
                Platform* plat4=new Platform(); (*plat4).setPosition({530.f, 265.f}); (*plat4).small_plat();

                MovingPlatform* plat5=new MovingPlatform(750.f, 265.f, 600.f); (*plat5).medium_plat();

                FragilePlatform* fragilePlat=new FragilePlatform();
                (*fragilePlat).setStartPos({1500.f, 500.f});
                (*fragilePlat).medium_plat();

                Platform* plat6=new Platform(); (*plat6).setPosition({2000.f, 700.f}); (*plat6).medium_plat();
                MovingPlatform* plat7=new MovingPlatform(2330.f, 550.f, 250.f); (*plat7).small_plat();
                MovingPlatform* plat8=new MovingPlatform(2600.f, 400.f, 250.f); (*plat8).small_plat();
                Platform* plat9=new Platform(); (*plat9).setPosition({3000.f, 750.f});(*plat9).medium_plat();

                FragilePlatform* plat10=new FragilePlatform(); (*plat10).setStartPos({3100.f, 350.f}); (*plat10).small_plat();
                Platform* plat11=new Platform(); (*plat11).setPosition({3400.f, 500.f}); (*plat11).medium_plat();

                platforms={plat1, plat2, plat3, plat4, plat5, fragilePlat, plat6, plat7, plat8, plat9, plat10, plat11};

                Spike s1; s1.setPosition({2100.f, 700.f});
                Spike s2; s2.setPosition({2350.f, 700.f});
                Spike s3; s3.setPosition({3700.f, 500.f});
                spikes.push_back(s1);
                spikes.push_back(s2);
                spikes.push_back(s3);



                Checkpoint checkP_1(610.f, 180.f);
                Checkpoint checkP_2(2210.f, 615.f);
                checkpoints.push_back(checkP_1);
                checkpoints.push_back( checkP_2);
                break;
            }


            case 2:
                //ïðîñòî äëÿ òåñòà, ïîòîì ïðèäóìàåì ñàì óðîâåíü
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
        for (int i=0; i<platforms.size(); i++) {
            delete platforms[i];
        }
        platforms.clear();
        spikes.clear();
        checkpoints.clear();
    }
};




int main() {
    RenderWindow window(VideoMode({1500, 900}), "Platformer");
    window.setFramerateLimit(60);
    View view(FloatRect({0.f, 0.f}, {1500.f, 900.f}));

    RectangleShape backk({2850.f, 900.f});
    backk.setFillColor(Color(172, 231, 242));

    Texture backgroundTex;
    backgroundTex.loadFromFile("sprites/background.jpg");
    Sprite background(backgroundTex);
    background.setTexture(backgroundTex);
    background.setScale({1.5f, 1.5f});



    Level level;
    int levelNumb=1;
    level.create(levelNumb);

    Character character;
    character.setPosition(level.startPos);
    Vector2f respawn=level.startPos;


    Health health;
    int damageCount=0;

    while (window.isOpen())
    {
#if VERSION==3
        while (const std::optional event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->code == Keyboard::Key::Space) {
                    character.jump();
                }
            }
        }
#else
        Event event;
        while (window.pollEvent(event))
        {
            if(event.type==Event::Closed)
                window.close();

            if(event.type==Event::KeyPressed) {
                if(event.key.code==Keyboard::Key::Space){
                    character.jump();
                }
            }
        }
#endif


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

        bool hitSpike=false;
        for (auto& spike: level.spikes) {
#if VERSION==3
            if (characterBounds.findIntersection(spike.getBounds())) {
                hitSpike=true;
                break;
            }
#else
            if (characterBounds.intersects(spike.getBounds())) {
                hitSpike=true;
                break;
            }
#endif
        }

        if (hitSpike){
            if (!character.invuln){
                character.getDamage();
                damageCount++;
                health.updateHealth(damageCount);
            }
        }


        for (auto& checkpoint: level.checkpoints){
#if VERSION==3
            if (characterBounds.findIntersection(checkpoint.getBounds())){
#else
            if (characterBounds.intersects(checkpoint.getBounds())) {
#endif
                if (checkpoint.activate()){
                    respawn=checkpoint.getPosition();
                }
            }
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
                    if (prevPos.x < platfBounds.position.x) {
                        character.setPosition({platfBounds.position.x - characterBounds.size.x / 2.f, character.getPosition().y});
                    }
                    else if (prevPos.x > platfBounds.position.x + platfBounds.size.x) {
                        character.setPosition({platfBounds.position.x + platfBounds.size.x + characterBounds.size.x / 2.f, character.getPosition().y});
                    }
                }
            }
#else
            if (characterBounds.intersects(platfBounds)) {
                if ((*level.platforms[i]).isBroken()) continue;
                if (character.dy>=0.f && (prevPos.y+characterBounds.height/2.f)<= platfBounds.top+5.f){
                    float pos_y=platfBounds.top-(characterBounds.height/2.f);
                    character.setPosition(character.getPosition().x, pos_y);
                    character.landing();
                    onPlatform=true;
                    (*level.platforms[i]).onStepped();
                    character.move((*level.platforms[i]).getSpeed());
                }

                else if (character.dy < 0.f && (prevPos.y-characterBounds.height/2.f)>=(platfBounds.top+platfBounds.height-5.f)) {
                    character.setPosition(character.getPosition().x, prevPos.y);
                    character.dy=0.f;
                }

                else {
                    if (prevPos.x<platfBounds.left){
                        character.setPosition(platfBounds.left-characterBounds.width/2.f, character.getPosition().y);
                    }

                    else if (prevPos.x>platfBounds.left+platfBounds.width) {
                        character.setPosition(platfBounds.left+platfBounds.width+characterBounds.width/2.f, character.getPosition().y);
                    }
                }
            }
#endif
        }

        if (!onPlatform)
            character.isJump=true;

        float cameraX=character.getPosition().x +250.f;
        if (cameraX<750.f)
            cameraX=750.f;
        if (cameraX>3150.f)
            cameraX=3150.f;
        view.setCenter({cameraX, 450.f});


        window.clear();
        window.setView(view);


        window.draw(backk);
        window.draw(background);

        for (int i=0; i<level.platforms.size(); i++) {
            window.draw(*level.platforms[i]);
        }

        for (auto& spike : level.spikes) {
            window.draw(spike);
        }

        for (auto& i: level.checkpoints) {
            window.draw(i);
        }

        window.draw(character);


        window.setView(window.getDefaultView());
        window.draw(health);

        window.display();
    }

    level.clearLevel();
    return 0;
}
