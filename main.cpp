#define VERSION 2

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#if VERSION==3
    #include <optional>
#endif

using namespace sf;
using namespace std;


//  ÏÅÐÑÎÍÀÆ

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
        if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
            move({-speed, 0.f});
            direct=-1.f;
            sprite.setScale({-0.4f, 0.4f});
            if (!isJump) animation();
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
            move({speed, 0.f});
            direct=1.f;
            sprite.setScale({0.4f, 0.4f});
            if (!isJump) animation();
        }
        dy+=gravity;
        move({0.f, dy});
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



//   ÏËÀÒÔÎÐÌÛ

class Platform: public Drawable, public Transformable {
private:
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



//   ÇÄÎÐÎÂÜÅ

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
        sprite.setTexture(healthTex[0]);
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






int main() {
    RenderWindow window(VideoMode({1500, 900}), "Platformer");
    window.setFramerateLimit(60);
    View view(FloatRect({0.f, 0.f}, {1500.f, 900.f}));

    RectangleShape backk({2850.f, 900.f});
    backk.setFillColor(Color(172, 231, 242));

    Texture backgroundTex;
    backgroundTex.loadFromFile("sprites/background.jpg");
    Sprite background;
    background.setTexture(backgroundTex);
    background.setScale({1.5f, 1.5f});



    const Vector2f startPos(50.f, 660.f);
    Character character;
    character.setPosition(startPos);

    Platform plat1;
    plat1.setPosition({0.f, 700.f});
    plat1.medium_plat();


    Platform plat2;
    plat2.setPosition({450.f, 560.f});
    plat2.small_plat();

    Platform plat3;
    plat3.setPosition({250.f, 420.f});
    plat3.small_plat();

    Platform plat4;
    plat4.setPosition({530.f, 265.f});
    plat4.small_plat();

    MovingPlatform plat5(800.f, 265.f, 600.f);
    plat5.medium_plat();

    Platform plat6;
    plat6.setPosition({2100.f, 700.f});

    vector<Platform*> platforms={&plat1, &plat2, &plat3, &plat4, &plat5, &plat6};

    Health health;
    int damageCount=0;

    while (window.isOpen())
    {
#if VERSION == 3
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

        for (int i=0; i<platforms.size(); i++) {
            (*platforms[i]).movement();
        }


        if (character.getPosition().y>900.f) {
            damageCount++;
            health.updateHealth(damageCount);

            character.setPosition(startPos);
            character.landing();

            character.direct=1.f;
            prevPos=character.getPosition();
            continue;
        }

        FloatRect characterBounds=character.getBounds();
        bool onPlatform=false;

        for (int i=0; i<platforms.size(); i++) {
            FloatRect platfBounds=(*platforms[i]).getBounds();
#if VERSION == 3
            if (characterBounds.findIntersection(platfBounds)) {
                if (character.dy >= 0.f && (prevPos.y + characterBounds.size.y / 2.f) <= platfBounds.position.y + 5.f) {
                    float pos_y = platfBounds.position.y - (characterBounds.size.y / 2.f);
                    character.setPosition({character.getPosition().x, pos_y});
                    character.landing();
                    onPlatform = true;
                    character.move((*platforms[i]).getSpeed());
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
                if (character.dy>=0.f && (prevPos.y+characterBounds.height/2.f)<= platfBounds.top+5.f){
                    float pos_y=platfBounds.top-(characterBounds.height/2.f);
                    character.setPosition(character.getPosition().x, pos_y);
                    character.landing();
                    onPlatform=true;
                    character.move((*platforms[i]).getSpeed());
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
        if (cameraX>2100.f)
            cameraX=2100.f;
        view.setCenter({cameraX, 450.f});


        window.clear();
        window.setView(view);


        window.draw(backk);
        window.draw(background);

        for (int i=0; i<platforms.size(); i++) {
            window.draw(*platforms[i]);
        }

        window.draw(character);




        window.setView(window.getDefaultView());
        window.draw(health);

        window.display();
    }

    return 0;
}
