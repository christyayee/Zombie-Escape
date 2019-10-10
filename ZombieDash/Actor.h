#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class Actor: public GraphObject
{
public:
    Actor(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):GraphObject(imageID, startX, startY, dir, depth, size){
        ticks = 0;
        interactor = nullptr;
        alive = true;
        sw = world;
        blockable = (depth == 0);
    }
    virtual bool react() = 0;
    virtual void doSomething() = 0;
    StudentWorld* getWorld() const;

    void setInteractor(Actor* a);
    Actor* getInteractor();
    int getTicks();
    void addTicks();
    
    virtual bool canBlock() const;
    virtual bool isEnvironment() const;
    virtual bool isFleeting() const;
    virtual bool canKill() const;
    virtual bool canTriggerLandmine() const;
    virtual bool canInfect() const;
    virtual bool isInfectable() const;
    virtual bool isPickable() const;
    virtual bool isAlive() const;
    virtual void kill();
    
    virtual ~Actor();
private:
    int ticks;
    bool blockable;
    Actor* interactor;
    bool alive;
    StudentWorld* sw;
};

//DONE
class Wall: public Actor
{
public:
    Wall(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Actor(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual bool react();
    virtual bool isEnvironment() const;
    virtual ~Wall();
private:
};

//DONE
class Exit: public Actor
{
public:
    Exit(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Actor(imageID, startX, startY, dir, depth, size, world){
    }
    virtual void doSomething();
    virtual bool react();
    virtual bool isEnvironment() const;
    virtual ~Exit();
private:
};

class Damageable:public Actor
{
public:
    Damageable(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Actor(imageID, startX, startY, dir, depth, size, world){}
    virtual ~Damageable();
private:
};

class Damaging:public Actor
{
public:
    Damaging(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Actor(imageID, startX, startY, dir, depth, size, world){}
    virtual bool canBlock() const;
    virtual bool canKill() const;
    virtual ~Damaging();
    
private:
};

//DONE
class Flame:public Damaging
{
public:
    Flame(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Damaging(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual bool react();
    virtual bool canTriggerLandmine() const;
    virtual ~Flame();
    
private:
};

//DONE
class Pit:public Damaging
{
public:
    Pit(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Damaging(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual bool react();
    virtual bool isEnvironment() const;
    virtual ~Pit();
    
private:
};

//DONE
class Landmine:public Damaging
{
public:
    Landmine(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Damaging(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual bool react();
    virtual bool isFleeting() const;
    void explode();
    virtual ~Landmine();
private:
};

//DONE
class Vomit:public Damaging
{
public:
    Vomit(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Damaging(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual bool react();
    virtual bool isFleeting() const;
    virtual bool canBlock() const;
    virtual bool canKill() const;
    virtual ~Vomit();
private:
};

class Goodie:public Damageable
{
public:
    Goodie(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Damageable(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual bool react();
    
    virtual bool isPickable() const;
    virtual bool pickGoodie() = 0;
    
    virtual ~Goodie();
    
private:
};

//DONE
class GVaccine:public Goodie
{
public:
    GVaccine(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Goodie(imageID, startX, startY, dir, depth, size, world){}
    
    virtual bool pickGoodie();
    virtual ~GVaccine();
    
private:
};

//DONE
class GGas:public Goodie
{
public:
    GGas(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Goodie(imageID, startX, startY, dir, depth, size, world){}
    
    virtual bool pickGoodie();
    virtual ~GGas();
    
private:
};

//DONE
class GLandmine:public Goodie
{
public:
    GLandmine(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world):Goodie(imageID, startX, startY, dir, depth, size, world){}
    
    virtual bool pickGoodie();
    virtual ~GLandmine();
    
private:
};

class Moveable:public Damageable
{
public:
    Moveable(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Damageable(imageID, startX, startY, dir, depth, size, world){
        closest = nullptr;
    }
    virtual bool canTriggerLandmine() const;
    bool move(Direction dir, int steps);
    bool followInteractor(int steps);
    bool turnTowards(int steps);
    void setClosest(Actor* close);
    Actor* getClosest();
    
    virtual ~Moveable();
private:
    Actor* closest;
};

class Zombie:public Moveable
{
public:
    Zombie(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Moveable(imageID, startX, startY, dir, depth, size, world){
        movementPlan = 0;
    }
    virtual bool react();   //reaction is vomit
    virtual void kill();
    virtual bool canInfect() const;
    void crawl();
    int getPlan() const;
    void setPlan(int num);
    virtual ~Zombie();
private:
    int movementPlan;
};

//DONE
class DumbZombie:public Zombie
{
public:
    DumbZombie(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Zombie(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual void kill();
    virtual ~DumbZombie();
private:
};

//DONE
class SmartZombie:public Zombie
{
public:
    SmartZombie(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Zombie(imageID, startX, startY, dir, depth, size, world){}
    virtual void doSomething();
    virtual ~SmartZombie();
private:
};

class Human:public Moveable
{
public:
    Human(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Moveable(imageID, startX, startY, dir, depth, size, world){
        infected = false;
        ticksInfected = 0;
    }
    virtual bool react();   //react to infection
    void setHealth(bool stat);
    bool isInfected() const;
    
    void sufferFromDisease();

    
    virtual bool isInfectable() const;
    
    virtual ~Human();
private:
    bool infected;
    int ticksInfected;
};

class Citizen: public Human
{
public:
    Citizen(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Human(imageID, startX, startY, dir, depth, size, world){
    }
    virtual void doSomething();
    virtual void kill();
    
    virtual ~Citizen();
private:
};

//DONE
class Penelope: public Human
{
public:
    Penelope(int imageID, double startX, double startY, Direction dir, int depth, double size, StudentWorld* world): Human(imageID, startX, startY, dir, depth, size, world){
        key = 0;
    }
    virtual void doSomething();
    virtual void kill();
    void setKey(int num);
        
    virtual ~Penelope();
private:
    int key;
    bool fireFlame(int i);
    void fireFlameThrower();
    void dropLandmine();
    void heal();
};

#endif // ACTOR_H_
