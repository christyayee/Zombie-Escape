#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"

using namespace std;

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

//ACTOR
void Actor::addTicks()
{
    ticks++;
}
int Actor::getTicks()
{
    return ticks;
}

bool Actor::canBlock() const
{
    return blockable;
}
bool Actor::isEnvironment() const
{
    return false;
}
bool Actor::isFleeting() const
{
    return false;
}
bool Actor::canKill() const
{
    return false;
}
bool Actor::canTriggerLandmine() const
{
    return false;
}
bool Actor::canInfect() const
{
    return false;
}
bool Actor::isInfectable() const
{
    return false;
}
bool Actor::isPickable() const
{
    return false;
}

bool Actor::isAlive() const
{
    return alive;
}
void Actor::kill()
{
    alive = false;
}

void Actor::setInteractor(Actor* a)
{
    interactor = a;
}
Actor* Actor::getInteractor()
{
    return interactor;
}

StudentWorld* Actor::getWorld() const
{
    return sw;
}

Actor::~Actor(){}


//WALL
void Wall::doSomething(){ return; }
bool Wall::react() { return true;}

bool Wall::isEnvironment() const
{
    return true;
}

Wall::~Wall(){}

//EXIT
void Exit::doSomething()
{
    getWorld()->checkAllOverlaps(this);
}

//returns true, plays sound, and updates status if Penelope exits (no more citizens)
//sets Citizen interactor to Exit and kills them (Citizen kill overridden to account for more actions)
bool Exit::react()
{
    if (getInteractor()->isInfectable()) //isHuman
    {
        if (!getWorld()->listContains(getInteractor())) //Penelope
        {
            if (getWorld()->getnumCitizens() <= 0) //able to exit
            {
                //play level finished sound
                getWorld()->playSound(SOUND_LEVEL_FINISHED);
                
                getWorld()->updateStatus(GWSTATUS_FINISHED_LEVEL);
                return true;
            }
        }
        else    //Citizen
        {
            getInteractor()->setInteractor(this); //sets interactor so Citizen's kill() knows it exited, not died
            getInteractor()->kill();
        }
    }
    return false;
}

bool Exit::isEnvironment() const
{
    return true;
}
Exit::~Exit(){}


//DAMAGEABLE
Damageable::~Damageable() {}

//DAMAGING
bool Damaging::canBlock() const
{
    return false;
}

bool Damaging::canKill() const
{
    return true;
}

Damaging::~Damaging() {}

//FLAME
void Flame::doSomething()
{
    if (isAlive())
    {
        addTicks();
        if (getTicks() > 2)
            kill();
        else
            getWorld()->checkAllOverlaps(this);
    }
}

//returns true if interactor is killed by flame
//returns false if flame is blocked and kills flame itself
bool Flame::react()
{
    if (!getInteractor()->isEnvironment())
    {
        if (getInteractor()->canKill() && !getInteractor()->canTriggerLandmine() && getInteractor()->isFleeting())
        {
            if (getInteractor()->getTicks() >= 30) //if Landmine, check ticks
            {
                getInteractor()->kill();
                return true;
            }
        }
        else
        {
            getInteractor()->kill();
            return true;
        }
    }
    else
    {
        if (!getInteractor()->canKill())// not Pit
            kill(); //kill Flame itself, move() will delete it before end of tick
    }
    return false;
}

bool Flame::canTriggerLandmine() const
{
    return true;
}

Flame::~Flame() {}

//PIT
void Pit::doSomething()
{
    getWorld()->checkAllOverlaps(this);
}

//returns true if Pit kills interactor
bool Pit::react()
{
    //reacts the same to every Moveable
    if (getInteractor()->canBlock() && !getInteractor()->isEnvironment())
    {
        getInteractor()->kill();
        return true;
    }
    return false;
}

bool Pit::isEnvironment() const
{
    return true;
}

Pit::~Pit(){}

//LANDMINE
void Landmine::doSomething()
{
    addTicks();
    if (getTicks() >= 30)
    {
        if (isAlive())
            getWorld()->checkAllOverlaps(this);
        else //dead from flame trigger
            explode();
    }
}

//returns true if Landmine gets triggered
//kills landmine and explodes
bool Landmine::react()
{
    if (getInteractor()->canTriggerLandmine())
    {
        kill();
        explode();
        return true;
    }
    return false;
}

//creates up to 9 Flames in a grid surrounding the position of the Landmine
//plays sound, adds Pit
void Landmine::explode()
{
    //play sound
    getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
    
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            Flame* f = new Flame(IID_FLAME, getX() + i*SPRITE_WIDTH, getY() + j*SPRITE_HEIGHT, 0, 0, 1, getWorld());
            getWorld()->checkAllOverlaps(f); //calls react, which may set Flame f to dead
            getWorld()->addActor(f); // will add to m_actors even if it's dead because move() will delete at end of tick
        }
    }
    getWorld()->addActor(new Pit(IID_PIT,getX(),getY(), right, 0, 1, getWorld()));
}

bool Landmine::isFleeting() const
{
    return true;
}

Landmine::~Landmine(){}

//VOMIT
void Vomit::doSomething()
{
    addTicks();
    if (getTicks() >= 2)
        kill();
    else
        getWorld()->checkAllOverlaps(this);
}

//set Citizen/Penelope's interactor to Vomit
//call their overlap reaction to infect
bool Vomit::react()
{
    if (getInteractor()->isInfectable())
    {
        getInteractor()->setInteractor(this);
        //getInteractor()->react(); call after odds have been decided
        return true;
    }
    return false;
}

bool Vomit::isFleeting() const
{
    return true;
}

bool Vomit::canBlock() const
{
    return false;
}

bool Vomit::canKill() const
{
    return false;
}

Vomit::~Vomit(){}


//GOODIE
void Goodie::doSomething()
{
    if (isAlive())
        getWorld()->checkAllOverlaps(this);
}

//returns true if Goodie is picked up
//kills itself, increases score points, plays sound
bool Goodie::react()
{
    if ( !getWorld()->listContains(getInteractor()) )//Penelope
    {
        if (pickGoodie())
        {
            kill();
            
            //add 50 points
            getWorld()->increaseScore(50);
            
            //play sound
            getWorld()->playSound(SOUND_GOT_GOODIE);
            return true;
        }
    }
    return false;
}

bool Goodie::isPickable() const
{
    return true;
}

Goodie::~Goodie(){}

//GVACCINE
bool GVaccine::pickGoodie()
{
    return getWorld()->addVaccine();
}

GVaccine::~GVaccine(){}

//GGAS
bool GGas::pickGoodie()
{
    return getWorld()->addGas();
}

GGas::~GGas(){}

//GLANDMINE
bool GLandmine::pickGoodie()
{
    return getWorld()->addLandmine();
}

GLandmine::~GLandmine(){}


//MOVEABLE

//changes direction towards dir, moves # of steps in that direction if not blocked by canBlock Actors
bool Moveable::move(Direction dir, int steps)
{
    switch (dir)
    {
        case left:
            setDirection(left);
            if (!getWorld()->isBlocked(this, left))
            {
                moveTo(getX()-steps, getY());
                return true;
            }
            break;
        case right:
            setDirection(right);
            if (!getWorld()->isBlocked(this, right))
            {
                moveTo(getX()+steps, getY());
                return true;
            }
            break;
        case up:
            setDirection(up);
            if (!getWorld()->isBlocked(this,up))
            {
                moveTo(getX(), getY()+steps);
                return true;
            }
            break;
        case down:
            setDirection(down);
            if (!getWorld()->isBlocked(this, down))
            {
                moveTo(getX(), getY()-steps);
                return true;
            }
            break;
    }
    return false;
}//changes direction even if blocked, but that should happen

//returns true if takes one move to follow Interactor, returns false otherwise
bool Moveable::followInteractor(int steps)
{
    //same row/col as interactor
    if (getX() == getInteractor()->getX() || getY() == getInteractor()->getY())
    {
        if (getInteractor()->getX() == getX()) //same col
        {
            if (getInteractor()->getY() > getY()) //above
                setDirection(up);
            else                                  //down
                setDirection(down);
        }
        else if (getInteractor()->getY() == getY()) //same row
        {
            if (getInteractor()->getX() > getX()) //right
                setDirection(right);
            else                                  //left
                setDirection(left);
        }
        return move(getDirection(), steps);
    }
    else
        return turnTowards(steps);
    return false;
}//!!!!!!!!!!!!!!!!!!!!!!!!EVEN IF FOLLOWINTERACTOR RETURNS FALSE, MOVEABLE'S DIRECTION IS CHANGED!!!!!!!!!!!!!!!!!

//if not on same row/col as interactor, turn towards randomly btwn 2 dirs
//if blocked in chosen dir, choose other dir
//returns true if successfully moved one move, returns false if blocked in both directions
bool Moveable::turnTowards(int steps)
{
    Direction row = right;
    Direction col = up;
    
    if (getInteractor()->getX() < getX()) //left
        row = left;
    if (getInteractor()->getY() < getY()) //down
        col = down;
    //randInt = 0 --> turn vertically
    //randInt = 1 --> turn horizontally
    if (randInt(0,1) == 0)
    {
        if (!move(col, steps)) //if blocked, try other
            return move(row,steps);
    }
    else //rand == 1
    {
        if (!move(row, steps)) //if blocked, try other
            return move(col,steps);
    }
    return false;
}//!!!!!!!!!!!!!!!!!!!!!!!!EVEN IF TURNTOWARDS RETURNS FALSE, MOVEABLE'S DIRECTION IS CHANGED!!!!!!!!!!!!!!!!!

bool Moveable::canTriggerLandmine() const
{
    return true;
}

void Moveable::setClosest(Actor* close)
{
    closest = close;
}

Actor* Moveable::getClosest()
{
    return closest;
}

Moveable::~Moveable() {}

//ZOMBIE

//returns true if Zombie vomits
bool Zombie::react()
{
    if ( getWorld()->isBlocked(this, getDirection()) && getInteractor() != nullptr && getInteractor()->isInfectable() )
    {
        Vomit* vom = nullptr;
        switch (getDirection()) //create vomit
        {
            case left:
                vom = new Vomit(IID_VOMIT,getX()-SPRITE_WIDTH, getY(),left,0,1,getWorld());
                break;
            case right:
                vom = new Vomit(IID_VOMIT,getX()+SPRITE_WIDTH, getY(),right,0,1,getWorld());
                break;
            case up:
                vom = new Vomit(IID_VOMIT,getX(), getY()+SPRITE_HEIGHT,up,0,1,getWorld());
                break;
            case down:
                vom = new Vomit(IID_VOMIT,getX(), getY()-SPRITE_HEIGHT,down,0,1,getWorld());
                break;
        }
        getWorld()->addActor(vom); //add so that if killed, will be erased by move() before end of tick
        
        int num = 0;
        
        //calls vom's react on all interactors, if interactor is Human --> sets Human's interactor to vom --> sets Human's interactor to vom --> calls Human's react to infect them before checking odds!!!!
        
        //UPDATE: no longer calls Human's react
        if ( getWorld()->checkAllOverlaps(vom) )
        {
            num = randInt(1, 3);
            if (num == 1)  // 1/3 chance
            {
                //odds in favor-->call vomThings which calls Human's react for all Humans with vom as interactor
                getWorld()->vomThings(vom);
                getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
                return true;
            }
        }
        if (num != 1)   //either did not overlap so 0, or randInt was not 1
            vom->kill();
    }
    return false;
}

//kills Zombie, plays sound, updates score points
//defaults point increase to SmartZombie
void Zombie::kill()
{
    Actor::kill();
    //play sound
    getWorld()->playSound(SOUND_ZOMBIE_DIE);
    
    //increase points (default 2000, assume smart)
    getWorld()->increaseScore(2000);
}

//moves Zombie 1 pixel according to its movement plan, decrements movement plan
//if blocked, resets movementPlan to 0 to be updated in next tick
void Zombie::crawl()
{
    if (move(getDirection(),1))
        movementPlan--;
    else
        movementPlan = 0;
}

int Zombie::getPlan() const
{
    return movementPlan;
}

void Zombie::setPlan(int num)
{
    movementPlan = num;
}

bool Zombie::canInfect() const
{
    return true;
}

Zombie::~Zombie(){}

//DUMB ZOMBIE
void DumbZombie::doSomething()
{
    addTicks();
    if (isAlive())
    {
        if (getTicks() % 2 != 0)
        {
            if (!react())
            {
                if (getPlan() <= 0)
                {
                    setPlan(randInt(3,10));
                    setDirection(randInt(0,3)*90);
                }
                crawl();
            }
        }
    }
}

//kills DumbZombie
//updates correct score result for Dumb, 1/10 chance of new GVaccine
//1/10 chance of adding GVaccine to m_actors before end of tick
//that's correct because Pit's overlap will be called on new GVaccine and kill it
//erased from m_actors before the next tick
void DumbZombie::kill()
{
    Zombie::kill();
    
    //decrease 1000 points bc not SmartZombie (+2000-1000 --> 1000 net)
    getWorld()->increaseScore(-1000);
    
    int rand = randInt(1,10);
    cerr << rand << endl;
    if (rand == 10)
        getWorld()->addActor(new GVaccine(IID_VACCINE_GOODIE,getX(),getY(), 0, 1, 1, getWorld()));
}

DumbZombie::~DumbZombie(){}

//SMART ZOMBIE
void SmartZombie::doSomething()
{
    addTicks();
    if (isAlive())
    {
        if (getTicks() % 2 != 0)
        {
            setInteractor(nullptr);
            if (!react())
            {
                if (getPlan() <= 0)
                {
                    setPlan(randInt(3,10));
                    
                    //update interactor to Penelope, update closest
                    int distance = getWorld()->setClosest(this);
                    
                    if (getInteractor() != nullptr)
                    {
                        double px = abs(getX() - getInteractor()->getX());
                        double py = abs(getY() - getInteractor()->getY());
                        double pxy = (px*px) + (py*py);
                        if (distance <= pxy)             //if penelope close enough, but citizen is closer
                            setInteractor(getClosest());
                        else                             //if penelope is closest, update distance
                            distance = pxy;
                    }
                    else    //penelope not close enough
                        setInteractor(getClosest());
                    
                    if (distance <= 6400) //if close enough, followInteractor
                    {
                        if (!followInteractor(1)) //if followInteractor failed (blocked), reset plan for next tick
                            setPlan(0);
                        else
                            setPlan(getPlan() - 1); //if followInteractor was successful, decrement movementPlan
                    }
                    else
                    {
                        setDirection(randInt(0,3)*90);
                        if (!move(getDirection(),1)) //if blocked, reset plan for next tick
                            setPlan(0);
                        else
                            setPlan(getPlan() - 1); //if move was successful, decrement movementPlan
                    }
                }
                else
                    crawl();
            }
        }
    }
}

SmartZombie::~SmartZombie(){}

//HUMAN

//returns true if infected
bool Human::react()
{
    if (getInteractor()->isFleeting() && !getInteractor()->canKill()) //if Vomit
    {
        if (!isInfected()) //if not already infected
        {
            if (getWorld()->listContains(this))
            {
                //play infect sound for citizens
                getWorld()->playSound(SOUND_CITIZEN_INFECTED);
            }
            infected = true;
        }
    }
    return infected;
}

//do disease things if infected
//increments ticksInfected, kills if over 500
void Human::sufferFromDisease()
{
    ticksInfected++;
    if (ticksInfected >= 500)
        kill();
}

void Human::setHealth(bool stat)
{
    infected = !stat;
    if (!infected) //if healed and not infected
        ticksInfected = 0;
}

bool Human::isInfected() const
{
    return infected;
}

bool Human::isInfectable() const
{
    return true;
}

Human::~Human() {}

//CITIZEN
void Citizen::doSomething()
{
    addTicks();
    if (isAlive())
    {
        if (isInfected())
            sufferFromDisease();
        if (isAlive() && getTicks() % 2 != 0)
        {
            //if penelope distance closer than zombie distance, follow penelope
            //if zombie closer or cannot follow penelope, avoid zombie
            
            int distance = getWorld()->setClosest(this);
            //interactor now penelope, closest now closest zombie
            
            if (getInteractor() != nullptr) //penelope is close enough
            {
                double px = abs(getX() - getInteractor()->getX());
                double py = abs(getY() - getInteractor()->getY());
                double pxy = (px*px) + (py*py);
                if (pxy < distance && followInteractor(2)) //penelope is close enough and closer than zombie
                    return;
            }
            
            //penelope not close enough or followInteractor returned false bc blocked
            setInteractor(getClosest());
            if (distance <= 6400) //avoid zombie
            {
                
            }
        }
    }
}

//kills Citizen, different depending on whether it exits, dies from infection, or dies from pit/flame
//plays sounds and updates score points
void Citizen::kill()
{
    Actor::kill();
    if (getInteractor() != nullptr && getInteractor()->isEnvironment()
        && !getInteractor()->canBlock() && !getInteractor()->canKill()) //if interacting with Exit
    {
        //play citizen saved sound
        getWorld()->playSound(SOUND_CITIZEN_SAVED);
        
        //increase points
        getWorld()->increaseScore(500);
        
    }
    else
    {
        //decrease points
        getWorld()->increaseScore(-1000);
        
        //!!!!!!!!!!!!!!!!!ZOMBIE IS BORN AND ADDED TO M_ACTORS BEFORE END OF TICK!!!!!!!!!!!!!!!!!!!!
        if (isInfected() && getTicks() >= 500) //died from infection
        {
            //play zombie born sound
            getWorld()->playSound(SOUND_ZOMBIE_BORN);
            int rand = randInt(1,10);
            if (rand <= 7)
                getWorld()->addActor(new DumbZombie(IID_ZOMBIE,getX(),getY(), right, 0, 1, getWorld()));
            else
                getWorld()->addActor(new SmartZombie(IID_ZOMBIE,getX(),getY(), right, 0, 1, getWorld()));
        }
        else //died from pit/flame
        {
            //play citizen dead sound
            getWorld()->playSound(SOUND_CITIZEN_DIE);
        }

    }
}

Citizen::~Citizen() {}


//PENELOPE
void Penelope::doSomething()
{
    addTicks();
    if (isAlive())
    {
        if (isInfected())
            sufferFromDisease();
        if (isAlive())
        {
            switch (key)
            {
                case KEY_PRESS_LEFT:
                    move(left, 4);
                    break;
                case KEY_PRESS_RIGHT:
                    move(right, 4);
                    break;
                case KEY_PRESS_UP:
                    move(up, 4);
                    break;
                case KEY_PRESS_DOWN:
                    move(down, 4);
                    break;
                case KEY_PRESS_SPACE:
                    fireFlameThrower();
                    break;
                case KEY_PRESS_TAB:
                    dropLandmine();
                    break;
                case KEY_PRESS_ENTER:
                    heal();
                    break;
                default:
                    break;
            }
        }
    }
}

//kills Penelope, while playing sound and updating status
void Penelope::kill()
{
    Actor::kill();
    
    //play player dead sound
    getWorld()->playSound(SOUND_PLAYER_DIE);

    getWorld()->updateStatus(GWSTATUS_PLAYER_DIED);
}

//set Penelope's key press as determined by player (to be called in StudentWorld)
void Penelope::setKey(int num)
{
    key = num;
}

//returns true if successfully fires a single flame in Penelope's direction, returns false otherwise
bool Penelope::fireFlame(int i)
{
    Flame* f = nullptr;
    switch (getDirection())
    {
        case left:
            f = new Flame(IID_FLAME, getX() - i*SPRITE_WIDTH, getY(), left, 0, 1, getWorld());
            break;
        case right:
            f = new Flame(IID_FLAME,getX() + i*SPRITE_WIDTH, getY(), right, 0, 1, getWorld());
            break;
        case up:
            f = new Flame(IID_FLAME,getX(), getY() + i*SPRITE_HEIGHT, up, 0, 1, getWorld());
            break;
        case down:
            f = new Flame(IID_FLAME,getX(), getY() - i*SPRITE_HEIGHT, down, 0, 1, getWorld());
            break;
    }
    
    getWorld()->checkAllOverlaps(f); //calls Flame's react which may set f to dead
    getWorld()->addActor(f); //add to m_actors even if dead because move() will delete dead Flame before end of tick
    
    if (f != nullptr && f->isAlive())
        return true;
    else
        return false;
}

//if, Penelope has Gas, fires up to 3 flames in Penelope's direction
void Penelope::fireFlameThrower()
{
    if (getWorld()->useGas())
    {
        int i = 1;
        while (i <=3 && fireFlame(i))
            i++;
        if (i >= 2) //while loop true at least once --> fired at least once
            getWorld()->playSound(SOUND_PLAYER_FIRE);
    }
}

//if Penelope has a Landmine, drops inactive landmine on Penelope's position
void Penelope::dropLandmine()
{
    if (getWorld()->useLandmine())
        getWorld()->addActor(new Landmine(IID_LANDMINE, getX(), getY(), right, 1, 1, getWorld()));
}

//if Penelope has Vaccine, heal herself
void Penelope::heal()
{
    if (getWorld()->useVaccine())
        setHealth(true);
}

Penelope::~Penelope() {}
